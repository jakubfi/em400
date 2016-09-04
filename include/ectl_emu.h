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

#include <inttypes.h>

#ifndef ECTL_H
#define ECTL_H

enum ectl_state {
	ECTL_OFF = 0,
	ECTL_ON = 1,
};

enum ectl_callbacks {
	ECTL_CB_STOP,
	ECTL_CB_HALT,
	ECTL_CB_MAX
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
	ECTL_LOG_ALL
};

enum ectl_capabilities {
	ECTL_CAPA_NONE		= 0,
	ECTL_CAPA_MX16		= 1 << 0,
	ECTL_CAPA_AWP		= 1 << 1,
	ECTL_CAPA_UIO		= 1 << 2,
	ECTL_CAPA_NOMEM_STOP= 1 << 3,
};

enum ectl_mem_types {
	ECTL_MEM_ANY = 0,
	ECTL_MEM_ELWRO,
	ECTL_MEM_MEGA,
};

typedef int (*ectl_callback_f)(int v);

int ectl_callback_register(int callback, ectl_callback_f callback_f);
uint16_t ectl_capabilities_get();
void ectl_log_state_set(int state);
int ectl_log_state_get();
int ectl_log_level_set(int component, int level);
int ectl_log_level_get(int component);
int ectl_cpu_count();
int ectl_mem_frames_get(int type, int module);
uint16_t ectl_mem_layout_get(int nb);
int ectl_mem_mega_prom_state_get();
uint32_t ectl_interrupts_get();
void ectl_interrupt_set(int interrupt);
void ectl_interrupt_clear(int interrupt);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
