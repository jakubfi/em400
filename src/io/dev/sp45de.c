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
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "log.h"

#include "io/dev/sp45de.h"

// -----------------------------------------------------------------------
static void sp45de_ioloop_teardown(sp45de_t * sp45de)
{

}

// -----------------------------------------------------------------------
void sp45de_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	sp45de_t *sp45de = (sp45de_t *) dev;

	LOG(L_FLOP, "SP45DE shutting down");

	// TODO: proper async free with libuv
	sp45de_ioloop_teardown((sp45de_t *) dev);
	pthread_mutex_lock(&sp45de->media_mutex);
	for (int slot=0 ; slot<SP45DE_SLOT_COUNT ; slot++) {
		if (sp45de->image[slot]) {
			fclose(sp45de->image[slot]);
		}
		free(sp45de->image_name[slot]);
	}
	pthread_mutex_unlock(&sp45de->media_mutex);
	pthread_mutex_destroy(&sp45de->media_mutex);
	free(sp45de);
}

// -----------------------------------------------------------------------
void sp45de_reset(em400_dev_t *dev)
{
	if (!dev) return;

	sp45de_t *sp45de = (sp45de_t *) dev;
	sp45de_motor_stop(sp45de);
	LOG(L_FLOP, "SP45DE reset");
}


// -----------------------------------------------------------------------
static int sp45de_image_seek(FILE *img_file, unsigned track, unsigned sector)
{
	if (!img_file) {
		LOG(L_FLOP, "Seek to: track %i, sector %i failed, no image loaded", track, sector);
		return E_ERR;
	}
	LOG(L_FLOP, "Seek to: track %i, sector %i", track, sector);
	int offset = (track * SP45DE_SECTOR_PER_TRACK + (sector-1)) * SP45DE_BLK_SIZE;
	int res = fseek(img_file, offset, SEEK_SET);
	if (res != 0) {
		return E_ERR;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int sp45de_blk_read(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector)
{
	int ret = E_ERR;
	sp45de->buf_pos = 0;

	pthread_mutex_lock(&sp45de->media_mutex);
	if (!sp45de->image[slot]) {
		LOG(L_FLOP, "Read with no image in slot %i", slot);
		goto fin;
	}

	if (sp45de_image_seek(sp45de->image[slot], track, sector) != E_OK) {
		LOG(L_FLOP, "Image file %s slot %i seek failed", sp45de->image_name[slot], slot);
		goto fin;
	}

	int res = fread(sp45de->buf, 1, SP45DE_BLK_SIZE, sp45de->image[slot]);
	if (res != SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Failed to read %i bytes from disk in slot %i: %s", SP45DE_BLK_SIZE, slot, sp45de->image_name[slot]);
		goto fin;
	}

	LOG(L_FLOP, "Read sector (%2x %2x %2x %2x ...)", sp45de->buf[0], sp45de->buf[1], sp45de->buf[2], sp45de->buf[3]);
	ret = E_OK;

fin:
	pthread_mutex_unlock(&sp45de->media_mutex);

	return ret;
}

// -----------------------------------------------------------------------
int sp45de_blk_write(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector)
{
	int ret = E_ERR;
	sp45de->buf_pos = 0;

	pthread_mutex_lock(&sp45de->media_mutex);
	if (!sp45de->image[slot]) {
		LOG(L_FLOP, "Write with no image in slot %i", slot);
		goto fin;
	}

	if (sp45de_image_seek(sp45de->image[slot], track, sector) != E_OK) {
		LOG(L_FLOP, "Image file %s slot %i seek failed", sp45de->image_name[slot], slot);
		goto fin;
	}

	int res = fwrite(sp45de->buf, 1, SP45DE_BLK_SIZE, sp45de->image[slot]);
	if (res != SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Failed floppy write");
		goto fin;
	}

	LOG(L_FLOP, "Write sector (%2x %2x %2x %2x ...)", sp45de->buf[0], sp45de->buf[1], sp45de->buf[2], sp45de->buf[3]);
	ret = E_OK;

fin:
	pthread_mutex_unlock(&sp45de->media_mutex);

	return ret;
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
int sp45de_read(sp45de_t *sp45de, uint8_t *c)
{
	*c = sp45de->buf[sp45de->buf_pos];
	LOG(L_FLOP, "buf read: %02x @ %i", (uint8_t) *c, sp45de->buf_pos);

	return sp45de_buf_advance(sp45de);
}

// -----------------------------------------------------------------------
int sp45de_write(sp45de_t *sp45de, uint8_t c)
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

	if ((!dev) || (slot >= SP45DE_SLOT_COUNT)) {
		LOG(L_FLOP, "Wrong SP45DE slot or nonexistent device");
		return E_ERR;
	}

	int ret = E_ERR;

	pthread_mutex_lock(&sp45de->media_mutex);
	if (sp45de->doors_locked) {
		LOG(L_FLOP, "SP45DE doors locked, cannot replace image in slot %i with %s", slot, image_name);
		goto fin;
	}

	// eject current image if present
	if (sp45de->image[slot]) {
		fclose(sp45de->image[slot]);
		sp45de->image[slot] = NULL;
		free(sp45de->image_name[slot]);
		sp45de->image_name[slot] = NULL;
	}

	// no new image, nothing to insert
	if (!image_name) {
		ret = E_OK;
		goto fin;
	}

	sp45de->image_name[slot] = strdup(image_name);
	if (!sp45de->image_name[slot]) {
		LOG(L_FLOP, "Memory allocation error when replacing image in slot %i with %s", slot, image_name);
		goto fin;
	}

	sp45de->image[slot] = fopen(sp45de->image_name[slot], "r+b");
	if (!sp45de->image[slot]) {
		LOG(L_FLOP, "Cannot open image file: %s for slot %i", image_name, slot);
		free(sp45de->image_name[slot]);
		sp45de->image_name[slot] = NULL;
		goto fin;
	}

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
		return NULL;
	}

	if (pthread_mutex_init(&sp45de->media_mutex, NULL)) {
		free(sp45de);
		return NULL;
	}

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

