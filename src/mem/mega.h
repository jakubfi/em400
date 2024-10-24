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

#ifndef MEM_MEGA_H
#define MEM_MEGA_H

#include <inttypes.h>

enum mem_mega_flags {
	MEM_MEGA_LOC		= 0b0000001,
	MEM_MEGA_DEALLOC	= 0b0000010,
	MEM_MEGA_PROM_SHOW	= 0b0010000,
	MEM_MEGA_PROM_HIDE	= 0b0100000,
	MEM_MEGA_ALLOC_DONE	= 0b1000000,
};

int mem_mega_init(int module_count, const char *prom_image);
void mem_mega_shutdown();
void mem_mega_reset(bool long_reset);
int mem_mega_cmd(int segment, int page, int module, int frame, int flags);
bool mem_mega_get_seg_ptrs(int segment, int page, uint16_t **rd_frame_ptr, uint16_t **wr_frame_ptr);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
