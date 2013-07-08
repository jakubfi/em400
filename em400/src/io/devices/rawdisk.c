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
#include "io/devices/rawdisk.h"


// -----------------------------------------------------------------------
struct rawdisk_t * rawdisk_create(int cylinders, int heads, int sectors, int sector_size, char *image_name)
{
	struct stat st;
	if (stat(image_name, &st) != 0) {
		gerr = E_DISK_STAT;
		return NULL;
	} else {
		int want_size = sector_size * (cylinders * heads * sectors);
		if (st.st_size != want_size) {
			gerr = E_DISK_SIZE;
			return NULL;
		}
	}

	FILE *image = fopen(image_name, "r+");
	if (!image) {
		gerr = E_FILE_OPEN;
		return NULL;
	}

	struct rawdisk_t *disk = malloc(sizeof(struct rawdisk_t));
	if (!disk) {
		gerr = E_ALLOC;
		return NULL;
	}

	disk->cylinders = cylinders;
	disk->heads = heads;
	disk->sectors = sectors;
	disk->sector_size = sector_size;
	disk->total_sectors = cylinders * heads * sectors;
	disk->image = image;
	disk->image_name = strdup(image_name);

	if (!disk->image_name) {
		gerr = E_ALLOC;
		rawdisk_shutdown(disk);
		return NULL;
	}

	return disk;
}

// -----------------------------------------------------------------------
void rawdisk_shutdown(struct rawdisk_t *d)
{
	if (d) {
		if (d->image) {
			fclose(d->image);
			d->image = NULL;
		}
		free(d->image_name);
		d->image_name = NULL;
	}
	free(d);
}

// -----------------------------------------------------------------------
int rawdisk_p2l(struct rawdisk_t *d, int cyl, int head, int sect)
{
	return sect + (head * d->sectors) + (cyl * d->heads * d->sectors);
}

// -----------------------------------------------------------------------
int rawdisk_read_sector_p(struct rawdisk_t *d, uint8_t *buf, int cyl, int head, int sect)
{
	return rawdisk_read_sector_l(d, buf, rawdisk_p2l(d, cyl, head, sect));
}

// -----------------------------------------------------------------------
int rawdisk_read_sector_l(struct rawdisk_t *d, uint8_t *buf, int sect)
{
	int res;

	if (sect >= d->total_sectors) {
		return E_DISK_NO_SECTOR;
	}

	res = fseek(d->image, sect*d->sector_size, SEEK_SET);
	if (res < 0) {
		return E_DISK_NO_SECTOR;
	}
	res = fread(buf, 1, d->sector_size, d->image);
	if (res != d->sector_size) {
		return E_DISK_RW_SIZE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int rawdisk_write_sector_p(struct rawdisk_t *w, uint8_t *buf, int count, int cyl, int head, int sect)
{
	return rawdisk_write_sector_l(w, buf, count, rawdisk_p2l(w, cyl, head, sect));
}

// -----------------------------------------------------------------------
int rawdisk_write_sector_l(struct rawdisk_t *d, uint8_t *buf, int count, int sect)
{
	int res;

	if (sect >= d->total_sectors) {
		return E_DISK_NO_SECTOR;
	}

	if (count > d->sector_size) {
		return E_DISK_DATA_TOO_BIG;
	}

	res = fseek(d->image, sect*d->sector_size, SEEK_SET);
	if (res < 0) {
		return E_DISK_NO_SECTOR;
	}
	res = fwrite(buf, 1, count, d->image);
	if (res != count) {
		return E_DISK_RW_SIZE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int rawdisk_park(int cyl)
{
	// mhm, sure.
	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
