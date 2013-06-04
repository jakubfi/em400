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
	struct stat st;

	if (!w) {
		// not created
		return -1;
	}

	if (w->image || w->image_name) {
		// already connected
		return -1;
	}

	if (stat(image_name, &st) != 0) {
		// cannot stat image file
		return -1;
	} else {
		int want_size = w->sector_size * (w->cylinders * w->heads * w->sectors);
		if (st.st_size != want_size) {
			// wrong image file size
			return -1;
		}
	}

	w->image = fopen(image_name, "r+");
	if (!w->image) {
		// cannot open file
		return -1;
	}

	w->image_name = strdup(image_name);
	if (!w->image_name) {
		// out of memory
		return -1;
	}

	return 0;
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
		// sector outside disk
		return -1;
	}

	res = fseek(w->image, sect*w->sector_size, SEEK_SET);
	if (res < 0) {
		// cannot find sector
		return -1;
	}
	res = fread(buf, 1, w->sector_size, w->image);
	if (res != w->sector_size) {
		// cannot read sector
		return -1;
	}
	return 0;
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
		// sector outside disk
		return -1;
	}

	if (count > w->sector_size) {
		// data won't fit in one sector
		return -4;
	}

	res = fseek(w->image, sect*w->sector_size, SEEK_SET);
	if (res < 0) {
		// cannot find sector
		return -2;
	}
	res = fwrite(buf, 1, count, w->image);
	if (res != count) {
		// cannot write sector
		return -3;
	}
	return 0;
}

// -----------------------------------------------------------------------
int winch_park(int cyl)
{
	// mhm, sure.
	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
