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

uint16_t image[2 * 1024 * 256]; // this should be done dynamically
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
	return img_get_sector();
}

// -----------------------------------------------------------------------
int img_set_ic(int new_ic)
{
	filepos += new_ic - ic;
	ic = new_ic;
	return ic;
}

// -----------------------------------------------------------------------
int img_get_ic()
{
	return ic;
}

// -----------------------------------------------------------------------
int img_inc_ic()
{
	ic++;
	filepos++;
	return ic;
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
int img_put_at(int addr, uint16_t word)
{
	DEBUG("img_put_at(): [0x%04x] (ic:%i, sector:%i) = %i\n", addr, addr, img_get_sector(), word);
	image[addr] = ntohs(word);
	return addr;
}


// -----------------------------------------------------------------------
int img_put(uint16_t word)
{
	DEBUG("img_put(): [0x%04x] (ic:%i, sector:%i) = %i\n", filepos, ic, img_get_sector(), word);
	image[filepos] = ntohs(word);
	if (filepos > max_filepos) {
		max_filepos = filepos;
	}
	ic++;
	filepos++;
	return ic;
}

// vim: tabstop=4

