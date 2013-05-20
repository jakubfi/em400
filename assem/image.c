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
#include <stdio.h>
#include <arpa/inet.h>

#include "eval.h"
#include "image.h"
#include "errors.h"

#define SECTOR_SIZE 256
#define IMAGE_SIZE 2 * 1024 * SECTOR_SIZE

uint16_t image[IMAGE_SIZE]; // this should be done dynamically
int ic = 0;
int max_filepos = -1;
int filepos = 0;

// -----------------------------------------------------------------------
int img_write(char *output_file)
{
	if (max_filepos < 0) {
		return E_PROG_EMPTY;
	}

	FILE *bin_out = fopen(output_file, "w");
	if (!bin_out) {
		return E_IO_OPEN;
	}

	int res = fwrite(image, 2, max_filepos+1, bin_out);
	fclose(bin_out);

	if (max_filepos+1 != res) {
		return E_IO_WRITE;
	}

	return res;
}

// -----------------------------------------------------------------------
int img_next_sector(int new_ic)
{
	filepos = (img_get_sector()+1) * SECTOR_SIZE;
	ic = new_ic;
	DEBUG("img_next_sector(): filepos = %i, ic = %i\n", filepos, ic);

	if ((ic < 0) || (ic > 0xffff)) {
		return E_IC;
	} else if ((filepos < 0) || (filepos >= IMAGE_SIZE)) {
		return E_FILEPOS;
	} else {
		return E_OK;
	}
}

// -----------------------------------------------------------------------
int img_set_ic(int new_ic)
{
	filepos += new_ic - ic;
	ic = new_ic;

	if ((ic < 0) || (ic > 0xffff)) {
		return E_IC;
	} else if ((filepos < 0) || (filepos >= IMAGE_SIZE)) {
		return E_FILEPOS;
	} else {
		return E_OK;
	}
}

// -----------------------------------------------------------------------
int img_get_ic()
{
	return ic;
}

// -----------------------------------------------------------------------
int img_inc_ic()
{
	return img_set_ic(ic+1);
}

// -----------------------------------------------------------------------
int img_get_filepos()
{
	return filepos;
}

// -----------------------------------------------------------------------
int img_get_sector()
{
	return filepos / SECTOR_SIZE;
}

// -----------------------------------------------------------------------
int img_put_at(int addr, int orig_ic, uint16_t word)
{
	DEBUG("img_put_at(): [0x%04x] (ic:%i, sector:%i) = %i\n", addr, orig_ic, img_get_sector(), word);
	if ((addr < 0) || (addr >= IMAGE_SIZE)) {
		return E_FILEPOS;
	}
	image[addr] = ntohs(word);
	return E_OK;
}


// -----------------------------------------------------------------------
int img_put(uint16_t word)
{
	int res = img_put_at(filepos, ic, word);
	if (res != E_OK) {
		return res;
	}
	if (filepos > max_filepos) {
		max_filepos = filepos;
	}
	ic++;
	filepos++;
	return E_OK;
}

// vim: tabstop=4

