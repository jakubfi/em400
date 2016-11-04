//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef ECTL_H
#define ECTL_H

#include <inttypes.h>
#include <stdio.h>

enum ectl_states {
	ECTL_OFF = 0,
	ECTL_ON
};

enum ectl_cpu_state_bits {
	ECTL_STATE_STOP = 0,
	ECTL_STATE_HALT,
	ECTL_STATE_CLM,
	ECTL_STATE_CLO,
	ECTL_STATE_QUIT,
	ECTL_STATE_COUNT
};

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
	ECTL_REG_SR,
	ECTL_REG_IR,
	ECTL_REG_KB,
	ECTL_REG_MOD,
	ECTL_REG_MODc,
	ECTL_REG_ALARM,
	ECTL_REG_COUNT
};

enum ectl_capability_bits {
	ECTL_CAPA_MX16 = 0,
	ECTL_CAPA_CRON,
	ECTL_CAPA_AWP,
	ECTL_CAPA_UIO,
	ECTL_CAPA_MEGABOOT,
	ECTL_CAPA_NOMEMSTOP,
	ECTL_CAPA_COUNT
};

enum ectl_log_components {
	ECTL_LOG_REG = 0,
	ECTL_LOG_MEM,
	ECTL_LOG_CPU,
	ECTL_LOG_OP,
	ECTL_LOG_INT,
	ECTL_LOG_IO,
	ECTL_LOG_MX,
	ECTL_LOG_PX,
	ECTL_LOG_CCHR,
	ECTL_LOG_CMEM,
	ECTL_LOG_TERM,
	ECTL_LOG_9425,
	ECTL_LOG_WNCH,
	ECTL_LOG_FLOP,
	ECTL_LOG_PNCH,
	ECTL_LOG_PNRD,
	ECTL_LOG_CRK5,
	ECTL_LOG_EM4H,
	ECTL_LOG_ECTL,
	ECTL_LOG_ALL,
	ECTL_LOG_COUNT = ECTL_LOG_ALL
};

#define ECTL_LOG_LEVEL_MIN 0
#define ECTL_LOG_LEVEL_MAX 9

// maintenance
int ectl_init();
void ectl_shutdown();

// registers
const char * ectl_reg_name(unsigned id);
int ectl_reg_get_id(char *name);
void ectl_regs_get(uint16_t *dest);
int ectl_reg_get(unsigned id);
int ectl_reg_set(unsigned id, uint16_t val);

// memory
int ectl_mem_get(int seg, uint16_t addr, uint16_t *dest, int count);
int ectl_mem_set(int seg, uint16_t addr, uint16_t *src, int count);
int ectl_mem_map(int seg);
int ectl_load(FILE *f, const char *name, int seg, uint16_t saddr);
void ectl_bootstrap(int chan, int unit);

// CPU state
int ectl_cpu_state_get();
const char * ectl_cpu_state_bit_name(int bitpos);
void ectl_cpu_stop();
void ectl_cpu_start();
void ectl_cpu_cycle();
void ectl_cpu_clear();
void ectl_cpu_quit();
void ectl_stoponhlt040_set(int state);

// interrupts
void ectl_clock_set(int state);
int ectl_clock_get();
void ectl_oprq();
int ectl_int_set(unsigned interrupt);
uint32_t ectl_int_get();

// informational, other
const char * ectl_version();
const char * ectl_capa_bit_name(unsigned bitpos);
int ectl_capa();
unsigned long ectl_ips_get();
int ectl_eval(char *expression, char **err_msg, int *err_beg, int *err_end);

// logging
int ectl_log_state_get();
int ectl_log_state_set(int state);
int ectl_log_level_get(unsigned component);
int ectl_log_level_set(unsigned component, unsigned level);
const char * ectl_log_component_name(unsigned component);
int ectl_log_component_id(char *name);

// breakpoints
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end);
int ectl_brk_del(unsigned id);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
