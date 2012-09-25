//  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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

int mjc400_init();
void mjc400_reset();
void mjc400_clear_mem();
void mjc400_fetch_instr();
int16_t mjc400_fetch_data();
void mjc400_step();
int mjc400_execute();
int __mjc400_load_image(const char* fname, uint16_t *ptr, uint16_t len);
int mjc400_load_os_image(const char* fname, uint16_t addr);
int mjc400_load_user_image(const char* fname, unsigned short bank, uint16_t addr);
void mjc400_dump_os_mem();
int16_t mjc400_get_eff_arg();
