//  Copyright (c) 2016-2019 Jakub Filipowicz <jakubf@gmail.com>
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

#pragma once

#ifndef ECTL_H
#define ECTL_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ectl_states {
	ECTL_OFF = 0,
	ECTL_ON
};

enum ectl_cpu_states {
	ECTL_STATE_ANY = -1,
	ECTL_STATE_RUN = 0,
	ECTL_STATE_STOP,
	ECTL_STATE_WAIT,
	ECTL_STATE_CLO,
	ECTL_STATE_OFF,
	ECTL_STATE_CYCLE,
	ECTL_STATE_BIN,
	ECTL_STATE_LOAD,
	ECTL_STATE_STORE,
	ECTL_STATE_FETCH,
// NOTE: update state names in ectl_cpu_state_get() when adding new states
	ECTL_STATE_UNKNOWN,
};

#define ECTL_REG_COUNT 16

enum ectl_registers {
	ECTL_REG_R0 = 0,
	ECTL_REG_R1,
	ECTL_REG_R2,
	ECTL_REG_R3,
	ECTL_REG_R4,
	ECTL_REG_R5,
	ECTL_REG_R6,
	ECTL_REG_R7,
	ECTL_REG_IC,
	ECTL_REG_AC,
	ECTL_REG_AR,
	ECTL_REG_IR,
	ECTL_REG_SR,
	ECTL_REG_RZ,
	ECTL_REG_KB,
	ECTL_REG_KB2
};

enum ectl_log_components {
	L_ALL = 0,
	L_EM4H, L_ECTL, L_FDBR, L_CRK5,
	L_MEM, L_CPU, L_OP, L_INT, L_IO,
	L_MX, L_CCHR, L_CMEM,
	L_TERM, L_9425, L_WNCH, L_FLOP, L_PNCH, L_PNRD, L_TAPE,
	L_COUNT,
};

int ectl_init();
void ectl_shutdown();

// Standard control panel interface
int ectl_bus_w_get();
void ectl_kb_set(uint16_t val);
// TODO: step
// TODO: mode + LED
int ectl_stopn(bool state);
// TODO: stopn LED
void ectl_cpu_cycle();
void ectl_load();
void ectl_store();
void ectl_fetch();
void ectl_cpu_start(bool state);
int ectl_bin();
void ectl_cpu_clear();
void ectl_clock_set(int state);
bool ectl_clock_get();
bool ectl_q_get();
bool ectl_p_get();
bool ectl_mc_get();
bool ectl_irq_get();
void ectl_oprq();
void ectl_reg_select(int id);
bool ectl_run_get();
bool ectl_wait_get();
int ectl_alarm_get();
void ectl_cpu_off(); // TODO: on + led?

// Emulator extensions

// registers
const char * ectl_reg_name(unsigned id);
int ectl_reg_get_id(char *name);
void ectl_regs_get(uint16_t *dest);
int ectl_reg_get(unsigned id);
int ectl_reg_set(unsigned id, uint16_t val);
int ectl_nb_get();
int ectl_qnb_get();

// memory
bool ectl_mem_read_n(int seg, uint16_t addr, uint16_t *dest, unsigned count);
bool ectl_mem_write_n(int seg, uint16_t addr, uint16_t *src, unsigned count);
int ectl_mem_map(int seg);
int ectl_mem_cfg(int nb, int ab, int mp, int seg);
bool ectl_load_os_image(FILE *f, const char *name, int seg, uint16_t saddr);

// CPU state
const char * ectl_cpu_state_name(unsigned state);
unsigned ectl_cpu_state_get();

// interrupts
int ectl_int_set(unsigned interrupt);
int ectl_int_clear(unsigned interrupt);
uint32_t ectl_int_get32();

// informational, other
const char * ectl_version();
unsigned long ectl_ips_get();
int ectl_eval(char *expression, char **err_msg, int *err_beg, int *err_end);

// logging
int ectl_log_state_get();
int ectl_log_state_set(int state);
int ectl_log_component_get(unsigned component);
int ectl_log_component_set(unsigned component, int state);
const char * ectl_log_component_name(unsigned component);
int ectl_log_component_id(char *name);

// breakpoints
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end);
int ectl_brk_del(unsigned id);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
