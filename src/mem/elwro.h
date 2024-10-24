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

#ifndef MEM_ELWRO_H
#define MEM_ELWRO_H

int mem_elwro_init(int modc, int osc);
void mem_elwro_shutdown();
void mem_elwro_reset();
int mem_elwro_cmd(int segment, int page, int module, int frame, int flags);
bool mem_elwro_get_seg_ptrs(int segment, int page, uint16_t **rd_frame_ptr, uint16_t **wr_frame_ptr);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
