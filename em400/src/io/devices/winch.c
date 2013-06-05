//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "errors.h"
#include "io/devices/winch.h"


// -----------------------------------------------------------------------
struct winchester_t * winch_create(int cylinders, int heads, int sectors, int sector_size)
{
	struct winchester_t *w = malloc(sizeof(struct winchester_t));
	if (w) {
		w->cylinders = cylinders;
		w->heads = heads;
		w->sectors = sectors;
		w->sector_size = sector_size;
		w->total_sectors = cylinders*heads*sectors;
		w->image_name = NULL;
		w->image = NULL;
	}

	return w;
}

// -----------------------------------------------------------------------
int winch_open(struct winchester_t *w, char *image_name)
{
	if (!w) {
		return E_WINCH_NO;
	}

	if (w->image || w->image_name) {
		return E_WINCH_OPEN;
	}

	struct stat st;
	if (stat(image_name, &st) != 0) {
		return E_WINCH_STAT;
	} else {
		int want_size = w->sector_size * (w->cylinders * w->heads * w->sectors);
		if (st.st_size != want_size) {
			return E_WINCH_SIZE;
		}
	}

	w->image = fopen(image_name, "r+");
	if (!w->image) {
		return E_FILE_OPEN;
	}

	w->image_name = strdup(image_name);
	if (!w->image_name) {
		return E_ALLOC;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void winch_close(struct winchester_t *w)
{
	if (w) {
		if (w->image) {
			fclose(w->image);
			w->image = NULL;
		}
		free(w->image_name);
		w->image_name = NULL;
	}
}

// -----------------------------------------------------------------------
void winch_shutdown(struct winchester_t *w)
{
	winch_close(w);
	free(w);
}

// -----------------------------------------------------------------------
int winch_p2l(struct winchester_t *w, int cyl, int head, int sect)
{
	return sect + (head * w->sectors) + (cyl * w->heads * w->sectors);
}

// -----------------------------------------------------------------------
int winch_read_sector_p(struct winchester_t *w, uint8_t *buf, int cyl, int head, int sect)
{
	return winch_read_sector_l(w, buf, winch_p2l(w, cyl, head, sect));
}

// -----------------------------------------------------------------------
int winch_read_sector_l(struct winchester_t *w, uint8_t *buf, int sect)
{
	int res;

	if (sect >= w->total_sectors) {
		return E_WINCH_NO_SECTOR;
	}

	res = fseek(w->image, sect*w->sector_size, SEEK_SET);
	if (res < 0) {
		return E_WINCH_NO_SECTOR;
	}
	res = fread(buf, 1, w->sector_size, w->image);
	if (res != w->sector_size) {
		return E_WINCH_RW_SIZE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int winch_write_sector_p(struct winchester_t *w, uint8_t *buf, int count, int cyl, int head, int sect)
{
	return winch_write_sector_l(w, buf, count, winch_p2l(w, cyl, head, sect));
}

// -----------------------------------------------------------------------
int winch_write_sector_l(struct winchester_t *w, uint8_t *buf, int count, int sect)
{
	int res;

	if (sect >= w->total_sectors) {
		return E_WINCH_NO_SECTOR;
	}

	if (count > w->sector_size) {
		return E_WINCH_DATA_TOO_BIG;
	}

	res = fseek(w->image, sect*w->sector_size, SEEK_SET);
	if (res < 0) {
		return E_WINCH_NO_SECTOR;
	}
	res = fwrite(buf, 1, count, w->image);
	if (res != count) {
		return E_WINCH_RW_SIZE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int winch_park(int cyl)
{
	// mhm, sure.
	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
