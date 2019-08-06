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

#include <strings.h>
#include <stdlib.h>
#include <emcrk/process.h>

#include "ectl.h"
#include "debugger/decode.h"

struct decoder_t decoders[] = {
	{ "iv", "SYS: interrupt vectors (0x40)", decode_iv },
	{ NULL, NULL, NULL}
};

// -----------------------------------------------------------------------
struct decoder_t * find_decoder(char *name)
{
	struct decoder_t *d = decoders;
	while (d && d->name) {
		if (!strcasecmp(d->name, name)) {
			return d;
		}
		d++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
char * decode_iv(int nb, uint16_t addr, int arg)
{
	char *buf = (char *) malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	uint16_t data[33];
	ectl_mem_get(nb, addr, data, 33);

	pos += sprintf(b+pos, "0  NMI        = 0x%04x    16 channel 4  = 0x%04x\n", data[0],  data[addr+16]);
	pos += sprintf(b+pos, "1  mem parity = 0x%04x    17 channel 5  = 0x%04x\n", data[1],  data[addr+17]);
	pos += sprintf(b+pos, "2  no mem     = 0x%04x    18 channel 6  = 0x%04x\n", data[2],  data[addr+18]);
	pos += sprintf(b+pos, "3  2nd CPU    = 0x%04x    19 channel 7  = 0x%04x\n", data[3],  data[addr+19]);
	pos += sprintf(b+pos, "4  I/F power  = 0x%04x    20 channel 8  = 0x%04x\n", data[4],  data[addr+20]);
	pos += sprintf(b+pos, "5  timer      = 0x%04x    21 channel 9  = 0x%04x\n", data[5],  data[addr+21]);
	pos += sprintf(b+pos, "6  illegal op = 0x%04x    22 channel 10 = 0x%04x\n", data[6],  data[addr+22]);
	pos += sprintf(b+pos, "7  div overfl = 0x%04x    23 channel 11 = 0x%04x\n", data[7],  data[addr+23]);
	pos += sprintf(b+pos, "8  FP underfl = 0x%04x    24 channel 12 = 0x%04x\n", data[8],  data[addr+24]);
	pos += sprintf(b+pos, "9  FP overfl  = 0x%04x    25 channel 13 = 0x%04x\n", data[9],  data[addr+25]);
	pos += sprintf(b+pos, "10 FP error   = 0x%04x    26 channel 14 = 0x%04x\n", data[10], data[addr+26]);
	pos += sprintf(b+pos, "11 extra int  = 0x%04x    27 channel 15 = 0x%04x\n", data[11], data[addr+27]);
	pos += sprintf(b+pos, "12 channel 0  = 0x%04x    28 OPRQ       = 0x%04x\n", data[12], data[addr+28]);
	pos += sprintf(b+pos, "13 channel 1  = 0x%04x    29 2nd CPU    = 0x%04x\n", data[13], data[addr+29]);
	pos += sprintf(b+pos, "14 channel 2  = 0x%04x    30 software H = 0x%04x\n", data[14], data[addr+30]);
	pos += sprintf(b+pos, "15 channel 3  = 0x%04x    31 software L = 0x%04x\n", data[15], data[addr+31]);
	pos += sprintf(b+pos, "EXL = 0x%04x\n",data[32]);

	return buf;
}

// vim: tabstop=4 shiftwidth=4 autoindent
