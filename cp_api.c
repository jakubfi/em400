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

#include <stdlib.h>
#include <inttypes.h>

#ifndef EM400CTL_H
#define EM400CTL_H

enum em4cp_callbacks {
	EM4CP_CB_NONE = 0,
	EM4CP_CB_ALARM,
	EM4CP_CB_BREAKPOINT,
	EM4CP_CB_IRQ,
	EM4CP_CB_STATE_CHANGE,
	EM4CP_CB_CLEAR,
	EM4CP_CB_MAX
};

enum em4cp_registers {
	EM4CP_REG_R0 = 0,
	EM4CP_REG_R1,
	EM4CP_REG_R2,
	EM4CP_REG_R3,
	EM4CP_REG_R4,
	EM4CP_REG_R5,
	EM4CP_REG_R6,
	EM4CP_REG_R7,
	EM4CP_REG_IC,
	EM4CP_REG_SR,
	EM4CP_REG_IR,
	EM4CP_REG_KB,
	EM4CP_REG_MOD,
	EM4CP_REG_MODc,
	EM4CP_REG_ALARM,
	EM4CP_REG_MAX
};

enum em4cp_log_components {
	EM4CP_LOG_REG = 0,
	EM4CP_LOG_MEM,
	EM4CP_LOG_CPU,
	EM4CP_LOG_OP,
	EM4CP_LOG_INT,
	EM4CP_LOG_IO,
	EM4CP_LOG_MX,
	EM4CP_LOG_PX,
	EM4CP_LOG_CCHR,
	EM4CP_LOG_CMEM,
	EM4CP_LOG_TERM,
	EM4CP_LOG_9425,
	EM4CP_LOG_WNCH,
	EM4CP_LOG_FLOP,
	EM4CP_LOG_PNCH,
	EM4CP_LOG_PNRD,
	EM4CP_LOG_CRK5,
	EM4CP_LOG_EM4H,
	EM4CP_LOG_ALL
};

enum em4cp_capabilities {
	EM4CP_CAPA_NONE = 0,
	EM4CP_CAPA_MX16	= 1 << 0,
	EM4CP_CAPA_AWP	= 1 << 1,
	EM4CP_CAPA_UIO	= 1 << 2,
};

enum em4cp_mem_types {
	EM4CP_MEM_ANY = 0,
	EM4CP_MEM_ELWRO,
	EM4CP_MEM_MEGA,
};

enum dev_interface_types {
	DEVIF_TYPE_NONE = 0,
	DEVIF_TYPE_INT,
	DEVIF_TYPE_STR,
	DEVIF_TYPE_ENUM,
};

enum dev_interface_flags {
	DEVIF_F_NONE	= 0,
	DEVIF_F_R	= 1 << 0,
	DEVIF_F_W	= 1 << 1,
	DEVIF_F_OPTS	= 1 << 2,
};

struct dev_interface_option {
	int val_int;
	char *val_str;
};

struct dev_interface {
	char *id;
	char *name;
	int type;
	uint32_t flags;
	int val_int;
	char *val_str;
	struct dev_interface_option *opts;
};

struct dev_interface dev_media[] = {
	{
		.id = "media",
		.name = "Disk image",
		.type = DEVIF_TYPE_STR,
		.flags = DEVIF_F_R | DEVIF_F_W,
	}, {
		.id = "eject",
		.name = "Eject",
		.type = DEVIF_TYPE_NONE,
		.flags = DEVIF_F_W,
	}
};

struct dev_interface_option dev_serial_speeds[] = {
	{ 1200, "1200" },
	{ 2400, "2400" },
	{ 4800, "4800" },
	{ 9600, "9600" },
};

struct dev_interface_option dev_serial_params[] = {
	{ 0, "8N1" },
	{ 1, "8N2" },
	{ 2, "8O1" },
	{ 3, "8O2" },
	{ 5, "8E1" },
	{ 6, "8E2" },
};

struct dev_interface dev_serial[] = {
	{
		.id = "speed",
		.name = "Port speed",
		.type = DEVIF_TYPE_INT,
		.flags = DEVIF_F_R | DEVIF_F_W | DEVIF_F_OPTS,
		.opts = dev_serial_speeds,
	},
	{
		.id = "param",
		.name = "Port parameters",
		.type = DEVIF_TYPE_INT,
		.flags = DEVIF_F_R | DEVIF_F_W | DEVIF_F_OPTS,
		.opts = dev_serial_params,
	}

};

typedef int (*dev_callback_f)(int v);

// emulation specific - general

int em4cp_callback_register(int callback, dev_callback_f callback_f);
uint32_t em4cp_capabilities_get();
int em4cp_log_state_set(int state);
int em4cp_log_state_get();
int em4cp_log_level_set(int component, int level);
int em4cp_log_level_get(int component);
int em4cp_cpu_count();
int em4cp_mem_frames_get(int type, int module);
uint16_t em4cp_mem_layout_get(int nb);
int em4cp_mem_mega_prom_state_get();
uint32_t em4cp_interrupts_get();
void em4cp_interrupt_set(int interrupt);

// emulation specific - device control

int dev_param_int_set(int chan, int unit, int value);
int dev_param_int_get(int chan, int unit);
int dev_param_str_set(int chan, int unit, char *value);
char * dev_param_str_get(int chan, int unit);

// low-level interface

void em4cp_regs_get(uint16_t *regs);
uint16_t em4cp_reg_get(int reg);
int em4cp_reg_set(int reg, uint16_t val);
int em4cp_mem_get(int nb, uint16_t addr, int count, uint16_t *dest);
int em4cp_mem_set(int nb, uint16_t addr, int count, uint16_t *src);
int em4cp_state_get();
int em4cp_state_set(int state);
int em4cp_cycle();
int em4cp_clock_set(int state);
int em4cp_clock_get();
void em4cp_clear();
int em4cp_bootstrap(int chan, int unit);
void em4cp_oprq();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
