//  Copyright (c) 2025 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <stdbool.h>
#include <pthread.h>

#include "libem400.h"
#include "log.h"

#include "io/dev/sp45de.h"

#define SP45DE_NO_IMAGE -1

extern uv_loop_t *ioloop;

// -----------------------------------------------------------------------
static void sp45de_fd_close(uv_file fd)
{
	uv_fs_t req;
	uv_fs_close(ioloop, &req, fd, NULL);
	uv_fs_req_cleanup(&req);
}

// -----------------------------------------------------------------------
static bool sp45de_zombie_close(sp45de_t *sp45de)
{
	if (sp45de->zombie_image == SP45DE_NO_IMAGE) {
		return false;
	}
	sp45de_fd_close(sp45de->zombie_image);
	sp45de->zombie_image = SP45DE_NO_IMAGE;

	return true;
}

// -----------------------------------------------------------------------
static void sp45de_image_close(sp45de_t *sp45de, unsigned slot)
{
	if (sp45de->image[slot] != SP45DE_NO_IMAGE) {
		// if active transfer uses the slot, store it as a zombie image
		// recycled upon transfer completion
		if (sp45de->image[slot] == sp45de->blk_image) {
			sp45de->zombie_image = sp45de->image[slot];
		} else {
			sp45de_fd_close(sp45de->image[slot]);
		}
		sp45de->image[slot] = SP45DE_NO_IMAGE;
	}
	free(sp45de->image_name[slot]);
	sp45de->image_name[slot] = NULL;
}

// -----------------------------------------------------------------------
static void sp45de_free(sp45de_t *sp45de)
{
	for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
		sp45de_image_close(sp45de, slot);
	}
	sp45de_zombie_close(sp45de);
	pthread_mutex_destroy(&sp45de->media_mutex);
	free(sp45de);
}

// -----------------------------------------------------------------------
void sp45de_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	sp45de_t *sp45de = (sp45de_t *) dev;

	LOG(L_FLOP, "SP45DE shutting down");

	pthread_mutex_lock(&sp45de->media_mutex);
	sp45de->shutting_down = true;
	bool blk_in_flight = (sp45de->blk_image != SP45DE_NO_IMAGE);
	pthread_mutex_unlock(&sp45de->media_mutex);

	// postpone freeing resources - transfer finisher will take care of it
	if (blk_in_flight) {
		return;
	}

	sp45de_free(sp45de);
}

// -----------------------------------------------------------------------
void sp45de_reset(sp45de_t *sp45de)
{
	LOG(L_FLOP, "SP45DE reset");

	pthread_mutex_lock(&sp45de->media_mutex);
	sp45de->buf_pos = 0;
	pthread_mutex_unlock(&sp45de->media_mutex);
}

// -----------------------------------------------------------------------
static void sp45de_on_blk_done(uv_fs_t *req)
{
	sp45de_t *sp45de = (sp45de_t *) uv_req_get_data((uv_req_t *) req);
	ssize_t transferred = req->result;
	uv_fs_req_cleanup(req);

	pthread_mutex_lock(&sp45de->media_mutex);
	sp45de->blk_image = SP45DE_NO_IMAGE;
	bool shutting_down = sp45de->shutting_down;
	bool media_changed = sp45de_zombie_close(sp45de);
	pthread_mutex_unlock(&sp45de->media_mutex);

	if (shutting_down) {
		sp45de_free(sp45de);
		return;
	}

	int ret;
	if (media_changed) {
		ret = E_ERR;
		LOG(L_FLOP, "Block transfer torn by a media change");
	} else if (transferred != SP45DE_BLK_SIZE) {
		ret = E_ERR;
		LOG(L_FLOP, "Block transfer failed: %s", (transferred < 0) ? uv_strerror((int) transferred) : "short transfer");
	} else {
		ret = E_OK;
		LOG(L_FLOP, "Block transferred (%2x %2x %2x %2x ...)", sp45de->buf[0], sp45de->buf[1], sp45de->buf[2], sp45de->buf[3]);
	}

	sp45de->blk_cb(sp45de->blk_ctx, ret);
}

// -----------------------------------------------------------------------
static int sp45de_blk_start(sp45de_t *sp45de, bool write, unsigned slot, unsigned track, unsigned sector, sp45de_blk_cb_f cb, void *ctx)
{
	int ret = E_ERR;

	pthread_mutex_lock(&sp45de->media_mutex);

	if (sp45de->blk_image != SP45DE_NO_IMAGE) {
		LOG(L_FLOP, "Block transfer requested while old one is still active");
		goto fin;
	}
	if (slot >= EM400_SP45DE_SLOT_COUNT) {
		LOG(L_FLOP, "Block transfer for a wrong slot %i", slot);
		goto fin;
	}
	if (sp45de->image[slot] == SP45DE_NO_IMAGE) {
		LOG(L_FLOP, "Block transfer with no image in slot %i", slot);
		goto fin;
	}

	if ((track >= SP45DE_TRACK_CNT) || (sector < 1) || (sector > SP45DE_SECTOR_PER_TRACK)) {
		LOG(L_FLOP, "Block transfer with address out of range: track %i, sector %i", track, sector);
		goto fin;
	}

	sp45de->buf_pos = 0;

	int64_t offset = ((int64_t) track * SP45DE_SECTOR_PER_TRACK + (sector-1)) * SP45DE_BLK_SIZE;
	uv_buf_t buf = uv_buf_init((char *) sp45de->buf, SP45DE_BLK_SIZE);

	sp45de->blk_cb = cb;
	sp45de->blk_ctx = ctx;
	uv_req_set_data((uv_req_t *) &sp45de->fs_req, sp45de);

	int res;
	if (write) {
		res = uv_fs_write(ioloop, &sp45de->fs_req, sp45de->image[slot], &buf, 1, offset, sp45de_on_blk_done);
	} else {
		res = uv_fs_read(ioloop, &sp45de->fs_req, sp45de->image[slot], &buf, 1, offset, sp45de_on_blk_done);
	}
	if (res) {
		LOG(L_FLOP, "Failed to queue %s of track %i, sector %i in slot %i: %s", write ? "write" : "read", track, sector, slot, uv_strerror(res));
		goto fin;
	}

	LOG(L_FLOP, "Queued %s of track %i, sector %i in slot %i", write ? "write" : "read", track, sector, slot);
	sp45de->blk_image = sp45de->image[slot];
	ret = E_OK;

fin:
	pthread_mutex_unlock(&sp45de->media_mutex);

	return ret;
}

// -----------------------------------------------------------------------
int sp45de_blk_read(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector, sp45de_blk_cb_f cb, void *ctx)
{
	return sp45de_blk_start(sp45de, false, slot, track, sector, cb, ctx);
}

// -----------------------------------------------------------------------
int sp45de_blk_write(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector, sp45de_blk_cb_f cb, void *ctx)
{
	return sp45de_blk_start(sp45de, true, slot, track, sector, cb, ctx);
}

// -----------------------------------------------------------------------
static int sp45de_buf_advance(sp45de_t *sp45de)
{
	sp45de->buf_pos++;
	if (sp45de->buf_pos >= SP45DE_BLK_SIZE) {
		sp45de->buf_pos = 0;
		return SP45DE_BUF_END;
	}
	return SP45DE_BUF_OK;
}

// -----------------------------------------------------------------------
int sp45de_buf_read(sp45de_t *sp45de, uint8_t *c)
{
	*c = sp45de->buf[sp45de->buf_pos];
	LOG(L_FLOP, "buf read: %02x @ %i", (uint8_t) *c, sp45de->buf_pos);

	return sp45de_buf_advance(sp45de);
}

// -----------------------------------------------------------------------
int sp45de_buf_write(sp45de_t *sp45de, uint8_t c)
{
	LOG(L_FLOP, "buf write: %02x @ %i", (uint8_t) c, sp45de->buf_pos);
	sp45de->buf[sp45de->buf_pos] = c;

	return sp45de_buf_advance(sp45de);
}

// -----------------------------------------------------------------------
int sp45de_motor_start(sp45de_t *sp45de)
{
	// starting drives and getting heads off the parking position
	// locks doors for all floppy slots
	pthread_mutex_lock(&sp45de->media_mutex);
	sp45de->doors_locked = true;
	pthread_mutex_unlock(&sp45de->media_mutex);

	return E_OK;
}

// -----------------------------------------------------------------------
int sp45de_motor_stop(sp45de_t *sp45de)
{
	pthread_mutex_lock(&sp45de->media_mutex);
	sp45de->doors_locked = false;
	pthread_mutex_unlock(&sp45de->media_mutex);

	return E_OK;
}

// -----------------------------------------------------------------------
bool sp45de_can_eject(em400_dev_t *dev, unsigned slot)
{
	sp45de_t *sp45de = (sp45de_t *) dev;

	pthread_mutex_lock(&sp45de->media_mutex);
	int can_eject = !sp45de->doors_locked;
	pthread_mutex_unlock(&sp45de->media_mutex);

	return can_eject;
}

// -----------------------------------------------------------------------
static int sp45de_image_replace(em400_dev_t *dev, unsigned slot, const char *image_name)
{
	sp45de_t *sp45de = (sp45de_t *) dev;

	if ((!dev) || (slot >= EM400_SP45DE_SLOT_COUNT)) {
		LOG(L_FLOP, "Wrong SP45DE slot or nonexistent device");
		return E_ERR;
	}

	int ret = E_ERR;

	pthread_mutex_lock(&sp45de->media_mutex);
	if (sp45de->doors_locked) {
		LOG(L_FLOP, "SP45DE doors locked, cannot replace image in slot %i with %s", slot, image_name);
		goto fin;
	}
	sp45de_image_close(sp45de, slot);

	// no new image (NULL or empty), nothing to insert
	if (!image_name || !*image_name) {
		ret = E_OK;
		goto fin;
	}

	sp45de->image_name[slot] = strdup(image_name);
	if (!sp45de->image_name[slot]) {
		LOG(L_FLOP, "Memory allocation error when replacing image in slot %i with %s", slot, image_name);
		goto fin;
	}

	uv_fs_t req;
	int fd = uv_fs_open(ioloop, &req, sp45de->image_name[slot], UV_FS_O_RDWR, 0, NULL);
	uv_fs_req_cleanup(&req);
	if (fd < 0) {
		LOG(L_FLOP, "Cannot open image file: %s for slot %i: %s", image_name, slot, uv_strerror(fd));
		free(sp45de->image_name[slot]);
		sp45de->image_name[slot] = NULL;
		goto fin;
	}
	sp45de->image[slot] = fd;

	LOG(L_FLOP, "SP45DE floppy image in slot %i: %s", slot, image_name);
	ret = E_OK;

fin:
	pthread_mutex_unlock(&sp45de->media_mutex);
	return ret;
}

// -----------------------------------------------------------------------
int sp45de_eject(em400_dev_t *dev, unsigned slot)
{
	return sp45de_image_replace(dev, slot, NULL);
}

// -----------------------------------------------------------------------
const char * sp45de_image(em400_dev_t *dev, unsigned slot)
{
	sp45de_t *sp45de = (sp45de_t *) dev;
	pthread_mutex_lock(&sp45de->media_mutex);
	const char *image = sp45de->image_name[slot];
	pthread_mutex_unlock(&sp45de->media_mutex);

	return image;
}

// -----------------------------------------------------------------------
int sp45de_load(em400_dev_t *dev, unsigned slot, const char *image_name)
{
	return sp45de_image_replace(dev, slot, image_name);
}

// -----------------------------------------------------------------------
em400_dev_t * sp45de_create()
{
	LOG(L_FLOP, "Creating SP45DE");

	sp45de_t *sp45de = calloc(1, sizeof(sp45de_t));
	if (!sp45de) {
		LOGERR("Failed to allocate memory for SP45DE device");
		return NULL;
	}

	if (pthread_mutex_init(&sp45de->media_mutex, NULL)) {
		LOGERR("Failed to initialize SP45DE media mutex");
		free(sp45de);
		return NULL;
	}

	for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
		sp45de->image[slot] = SP45DE_NO_IMAGE;
	}
	sp45de->zombie_image = SP45DE_NO_IMAGE;
	sp45de->blk_image = SP45DE_NO_IMAGE;

	sp45de->base.type = EM400_DEV_SP45DE;
	sp45de->base.slot_count = 4;
	sp45de->base.reset = NULL;
	sp45de->base.write = NULL;
	sp45de->base.shutdown = sp45de_shutdown;

	sp45de->base.can_eject = sp45de_can_eject;
	sp45de->base.load = sp45de_load;
	sp45de->base.eject = sp45de_eject;
	sp45de->base.image = sp45de_image;

	return (em400_dev_t *) sp45de;
}

// vim: tabstop=4 shiftwidth=4 autoindent
