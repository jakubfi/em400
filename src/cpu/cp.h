//  Copyright (c) 2017 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CP_H
#define CP_H

#include <inttypes.h>
#include <stdbool.h>

#include "cfg.h"

int cp_init(em400_cfg *cfg);
void cp_shutdown();

int cp_bus_w_get();
int cp_reg_get(unsigned id);
int cp_reg_set(unsigned id, uint16_t v);
bool cp_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
bool cp_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
void cp_stop();
void cp_start();
void cp_cycle();
void cp_off();
void cp_clock_set(int state);
int cp_clock_get();
void cp_clear();
int cp_bin();
void cp_oprq();
int cp_state();
int cp_stopn(uint16_t addr);
int cp_stopn_off();
void cp_reg_select(int reg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
