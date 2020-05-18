//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef UTILS_H
#define UTILS_H

#include <inttypes.h>
#include <termios.h>

#define BIN_ENDBYTE 0b01010000

char * int2binf(char *buf, const char *format, uint64_t value, int size);
char * int2chars(uint16_t w, char *buf);
void endianswap(uint16_t *ptr, int size);
double stopwatch_ns();
speed_t serial_int2speed(int s);
int serial_open(const char *device, speed_t speed);
int parity(unsigned int v);
void word2bin(uint16_t w, uint8_t *b);
uint16_t bin2word(uint8_t *b);
int bin_is_end(uint8_t b);
int bin_is_valid(uint8_t b);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
