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

#define RET_OK	0
#define RET_ERR	1

// internal functions

void _wdc_port_setup(void);
inline void _wdc_dir_in(void);
inline void _wdc_dir_out(void);
inline void _wdc_step_pulse(void);
void _wdc_seek_reset(void);

// user-level access

int wdc_init(void);
int wdc_drv_sel(unsigned char drv);
void wdc_head_sel(unsigned char head);
void wdc_seek(unsigned int cyl);
void wdc_step_in(void);
void wdc_step_out(void);

// vim: tabstop=4
