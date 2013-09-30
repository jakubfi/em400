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

#ifndef DEBUGGER_DECODE_H
#define DEBUGGER_DECODE_H

#include <inttypes.h>

typedef char * (*decoder_fun_t)(int nb, uint16_t addr, int arg);

struct decoder_t {
	char *name;
	char *desc;
	decoder_fun_t f_decode;
};

extern struct decoder_t decoders[];

struct decoder_t * find_decoder(char *name);

char * decode_iv(int nb, uint16_t addr, int arg);
char * decode_ctx(int nb, uint16_t addr, int arg);
char * decode_exl(int nb, uint16_t addr, int arg);
char * decode_mxpsuk(int nb, uint16_t addr, int arg);
char * decode_mxpsdl(int nb, uint16_t addr, int arg);
char * decode_mxpst_winch(int nb, uint16_t addr, int arg);
char * decode_mxpst_term(int nb, uint16_t addr, int arg);
char * decode_cmempst(int nb, uint16_t addr, int arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
