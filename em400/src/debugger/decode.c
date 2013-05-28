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

#include "memory.h"

#include "debugger/awin.h"
#include "debugger/ui.h"
#include "debugger/decode.h"

struct decoder_t decoders[] = {
	{ "iv", "SYS: interrupt vectors", decode_iv},
	{ "mxpsuk", "MULTIX: set configuration", decode_mxpsuk },
	{ "mxpsdl", "MULTIX: assign line", decode_mxpsdl },
	{ "mxpst", "MULTIX: transmit", decode_mxpst },
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
int decode_iv(int wid, uint16_t addr, int arg)
{
	awprint(wid, C_LABEL, "0  NMI        = 0x%04x    16 channel 4  = 0x%04x\n", nMEMB(0, addr+0), nMEMB(0, addr+16));
	awprint(wid, C_LABEL, "1  mem parity = 0x%04x    17 channel 5  = 0x%04x\n", nMEMB(0, addr+1), nMEMB(0, addr+17));
	awprint(wid, C_LABEL, "2  no mem     = 0x%04x    18 channel 6  = 0x%04x\n", nMEMB(0, addr+2), nMEMB(0, addr+18));
	awprint(wid, C_LABEL, "3  2nd CPU    = 0x%04x    19 channel 7  = 0x%04x\n", nMEMB(0, addr+3), nMEMB(0, addr+19));
	awprint(wid, C_LABEL, "4  I/F power  = 0x%04x    20 channel 8  = 0x%04x\n", nMEMB(0, addr+4), nMEMB(0, addr+20));
	awprint(wid, C_LABEL, "5  timer      = 0x%04x    21 channel 9  = 0x%04x\n", nMEMB(0, addr+5), nMEMB(0, addr+21));
	awprint(wid, C_LABEL, "6  illegal op = 0x%04x    22 channel 10 = 0x%04x\n", nMEMB(0, addr+6), nMEMB(0, addr+22));
	awprint(wid, C_LABEL, "7  div overfl = 0x%04x    23 channel 11 = 0x%04x\n", nMEMB(0, addr+7), nMEMB(0, addr+23));
	awprint(wid, C_LABEL, "8  FP underfl = 0x%04x    24 channel 12 = 0x%04x\n", nMEMB(0, addr+8), nMEMB(0, addr+24));
	awprint(wid, C_LABEL, "9  FP overfl  = 0x%04x    25 channel 13 = 0x%04x\n", nMEMB(0, addr+9), nMEMB(0, addr+25));
	awprint(wid, C_LABEL, "10 FP error   = 0x%04x    26 channel 14 = 0x%04x\n", nMEMB(0, addr+10), nMEMB(0, addr+26));
	awprint(wid, C_LABEL, "11 extra int  = 0x%04x    27 channel 15 = 0x%04x\n", nMEMB(0, addr+11), nMEMB(0, addr+27));
	awprint(wid, C_LABEL, "12 channel 0  = 0x%04x    28 OPRQ       = 0x%04x\n", nMEMB(0, addr+12), nMEMB(0, addr+28));
	awprint(wid, C_LABEL, "13 channel 1  = 0x%04x    29 2nd CPU    = 0x%04x\n", nMEMB(0, addr+13), nMEMB(0, addr+29));
	awprint(wid, C_LABEL, "14 channel 2  = 0x%04x    30 software H = 0x%04x\n", nMEMB(0, addr+14), nMEMB(0, addr+30));
	awprint(wid, C_LABEL, "15 channel 3  = 0x%04x    31 software L = 0x%04x\n", nMEMB(0, addr+15), nMEMB(0, addr+31));
	return 0;
}

// -----------------------------------------------------------------------
int decode_mxpsuk(int wid, uint16_t addr, int arg)
{
	return 0;
}

// -----------------------------------------------------------------------
int decode_mxpsdl(int wid, uint16_t addr, int arg)
{
	return 0;
}

// -----------------------------------------------------------------------
int decode_mxpst(int wid, uint16_t addr, int arg)
{
	return 0;
}


// vim: tabstop=4
