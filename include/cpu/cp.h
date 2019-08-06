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

#include "cfg.h"

// this must match register ids in ectl.h
enum cp_registers {
	CP_REG_R0 = 0,
	CP_REG_R1,
	CP_REG_R2,
	CP_REG_R3,
	CP_REG_R4,
	CP_REG_R5,
	CP_REG_R6,
	CP_REG_R7,
	CP_REG_IC,
	CP_REG_AC,
	CP_REG_AR,
	CP_REG_IR,
	CP_REG_SR,
	CP_REG_RZ,
	CP_REG_KB,
	CP_REG_KB2,
	CP_REG_MOD,
	CP_REG_MODc,
	CP_REG_ALARM,
	CP_REG_RM,
	CP_REG_Q,
	CP_REG_BS,
	CP_REG_NB,
	CP_REG_P,
	CP_REG_COUNT
};

int cp_init(struct cfg_em400 *cfg);
void cp_shutdown();

int cp_reg_get(unsigned id);
int cp_reg_set(unsigned id, uint16_t v);
int cp_mem_mget(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
int cp_mem_mput(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
void cp_stop();
void cp_start();
void cp_cycle();
void cp_off();
void cp_clock_set(int state);
int cp_clock_get();
void cp_clear();
void cp_bin();
void cp_oprq();
int cp_int_set(unsigned i);
uint32_t cp_int_get();
int cp_state();
int cp_stopn(uint16_t addr);
int cp_stopn_off();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
