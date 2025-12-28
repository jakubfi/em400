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

#include "log.h"

#include "io/dev2/sp45de.h"

// -----------------------------------------------------------------------
static void sp45de_ioloop_teardown(sp45de_t * sp45de)
{

}

// -----------------------------------------------------------------------
void sp45de_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	sp45de_t *sp45de = (sp45de_t *) dev;

	LOG(L_TERM, "SP45DE shutting down");

	// TODO: proper async free with libuv
	sp45de_ioloop_teardown((sp45de_t *) dev);
	for (int slot=0 ; slot<SP45DE_SLOT_COUNT ; slot++) {
		if (sp45de->image[slot]) {
			fclose(sp45de->image[slot]);
		}
		free(sp45de->image_name[slot]);
	}
	free(sp45de);
}

// -----------------------------------------------------------------------
void sp45de_reset(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_TERM, "SP45DE reset");
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
	sp45de->buf_pos = 0;
	if (sp45de_image_seek(sp45de->image[slot], track, sector) != E_OK) {
		LOG(L_FLOP, "Image file %s slot %i seek failed", sp45de->image_name[slot], slot);
		return E_ERR;
	}

	int res = fread(sp45de->buf, 1, SP45DE_BLK_SIZE, sp45de->image[slot]);
	if (res != SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Failed to read %i bytes from disk in slot %i: %s", SP45DE_BLK_SIZE, slot, sp45de->image_name[slot]);
		return E_ERR;
	}
	LOG(L_FLOP, "Read sector (%2x %2x %2x %2x ...)", sp45de->buf[0], sp45de->buf[1], sp45de->buf[2], sp45de->buf[3]);

	return E_OK;
}

// -----------------------------------------------------------------------
int sp45de_blk_write(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector)
{
	sp45de->buf_pos = 0;
	if (sp45de_image_seek(sp45de->image[slot], track, sector) != E_OK) {
		LOG(L_FLOP, "Image file %s slot %i seek failed", sp45de->image_name[slot], slot);
		return E_ERR;
	}

	LOG(L_FLOP, "Write sector (%2x %2x %2x %2x ...)", sp45de->buf[0], sp45de->buf[1], sp45de->buf[2], sp45de->buf[3]);

	int res = fwrite(sp45de->buf, 1, SP45DE_BLK_SIZE, sp45de->image[slot]);
	if (res != SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Failed floppy write");
		return E_ERR;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int sp45de_read(sp45de_t *sp45de)
{
	if (sp45de->buf_pos >= SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Trying to read past the buffer: @ %i", sp45de->buf_pos);
		return -1;
	}

	int ret = (int) sp45de->buf[sp45de->buf_pos];
	LOG(L_FLOP, "buf read: %02x", (uint8_t) ret);
	sp45de->buf_pos = (sp45de->buf_pos+1) & 0x7f;
	return ret;
}

// -----------------------------------------------------------------------
int sp45de_write(sp45de_t *sp45de, char c)
{
	LOG(L_FLOP, "buf write: %02x", (uint8_t) c);
	if (sp45de->buf_pos >= SP45DE_BLK_SIZE) {
		LOG(L_FLOP, "Trying to write past the buffer: @ %i", sp45de->buf_pos);
		return E_ERR;
	}

	sp45de->buf[sp45de->buf_pos] = c;
	sp45de->buf_pos = (sp45de->buf_pos+1) & 0x7f;

	return E_OK;
}

// -----------------------------------------------------------------------
int sp45de_park(sp45de_t *sp45de, unsigned slot)
{
	return E_OK;
}

// -----------------------------------------------------------------------
unsigned sp45de_slot_cnt()
{
	return SP45DE_SLOT_COUNT;
}

// -----------------------------------------------------------------------
bool sp45de_is_ejectable(em400_dev_t *dev, unsigned slot)
{
	sp45de_t *sp45de = (sp45de_t *) dev;
	return !sp45de->locked[slot];
}

// -----------------------------------------------------------------------
int sp45de_eject(em400_dev_t *dev, unsigned slot)
{
	sp45de_t *sp45de = (sp45de_t *) dev;

	if (!sp45de_is_ejectable(dev, slot)) {
		LOG(L_FLOP, "SP45DE 8 inch floppy in slot %i cannot be ejected", slot);
		return E_ERR;
	}

	fclose(sp45de->image[slot]);
	sp45de->image[slot] = NULL;
	free(sp45de->image_name[slot]);
	sp45de->image_name[slot] = NULL;

	return E_OK;
}

// -----------------------------------------------------------------------
const char * sp45de_image(em400_dev_t *dev, unsigned slot)
{
	sp45de_t *sp45de = (sp45de_t *) dev;

	return sp45de->image_name[slot];
}

// -----------------------------------------------------------------------
int sp45de_load(em400_dev_t *dev, unsigned slot, const char *image_name)
{
	sp45de_t *sp45de = (sp45de_t *) dev;

	if ((!dev) || (slot >= SP45DE_SLOT_COUNT) || (!image_name)) {
		LOG(L_FLOP, "Wrong SP45DE slot, empty image name or nonexistent device");
		return E_ERR;
	}

	if (sp45de_image(dev, slot)) {
		if (sp45de_eject(dev, slot) != E_OK) {
			return E_ERR;
		}
	}

	sp45de->image_name[slot] = strdup(image_name);
	if (!sp45de->image_name[slot]) {
		LOG(L_FLOP, "Memory allocation error for image name: %s (drive %i).", sp45de->image[slot], slot);
		return E_ERR;
	}

	sp45de->image[slot] = fopen(sp45de->image_name[slot], "r+b");
	if (!sp45de->image[slot]) {
		LOG(L_FLOP, "Failed to open 8-inch floppy image: %s (drive %i).", sp45de->image[slot], slot);
		free(sp45de->image_name[slot]);
		sp45de->image_name[slot] = NULL;
		return E_ERR;
	}

	LOG(L_FLOP, "SP45DE opened disk image %s in drive %i", sp45de->image_name[slot], slot);

	return E_OK;
}

// -----------------------------------------------------------------------
em400_dev_t * sp45de_create(const char *image_name[4])
{
	LOG(L_FLOP, "Creating SP45DE");

	sp45de_t *sp45de = calloc(1, sizeof(sp45de_t));
	if (!sp45de) {
		goto fail;
	}

	sp45de->base.type = EM400_DEV_SP45DE;
	sp45de->base.reset = NULL;
	sp45de->base.write = NULL;
	sp45de->base.shutdown = sp45de_shutdown;

	sp45de->base.slot_cnt = sp45de_slot_cnt;
	sp45de->base.is_ejectable = sp45de_is_ejectable;
	sp45de->base.load = sp45de_load;
	sp45de->base.eject = sp45de_eject;
	sp45de->base.image = sp45de_image;

	// TODO: drop image preloading, handle top-level with load()
	for (int slot=0 ; slot<SP45DE_SLOT_COUNT ; slot++) {
		if (!image_name[slot]) continue;
		sp45de_load((em400_dev_t *) sp45de, slot, image_name[slot]);
	}

	return (em400_dev_t *) sp45de;

fail:
	return NULL;
}

