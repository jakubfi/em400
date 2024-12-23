//  Copyright (c) 2024 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __LIBEM400_H__
#define __LIBEM400_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct em400_cfg_cpu {
	bool awp;
	bool mod;
	bool user_io_illegal;
	bool nomem_stop;
	bool speed_real;
	int throttle_granularity;
	int clock_period;
};

struct em400_cfg_buzzer {
	bool enabled;
	const char *driver;
	const char *output;
	int volume;
	int sample_rate;
	int buffer_len;
	int latency;
};

struct em400_cfg_mem {
	int elwro_modules;
	int mega_modules;
	int os_segments;
	const char *mega_prom_image;
	const char *os_mem_preload; // ?
};

enum device_types {
	DEV_TERMINAL_TCP,
	DEV_TERMINAL_SERIAL,
	DEV_WINCHESTER,
	DEV_FLOP8,
};

struct em400_config_device {
	int chnum;
	int devnum;
	int type;
	const char *image;
	int port;
	int tansport_type;
};

// -----------------------------------------------------------------------
// --- MAINTENANCE -------------------------------------------------------
// -----------------------------------------------------------------------

int em400_mem_configure(struct em400_cfg_mem *c_mem);
int em400_cpu_configure(struct em400_cfg_cpu *c_cpu, struct em400_cfg_buzzer *c_buzzer);

// -----------------------------------------------------------------------
// --- CONTROL PANEL -----------------------------------------------------
// -----------------------------------------------------------------------

// LEDs
uint16_t em400_cp_w_leds();
bool em400_cp_run_led();
bool em400_cp_wait_led();
bool em400_cp_alarm_led();
// MODE LED
// STOP*N LED
bool em400_cp_clock_led();
bool em400_cp_q_led();
bool em400_cp_p_led();
bool em400_cp_mc_led();
bool em400_cp_irq_led();
void em400_cp_kb(uint16_t val);
// STEP
// MODE
void em400_cp_stopn(bool state);
void em400_cp_cycle();
void em400_cp_load();
void em400_cp_store();
void em400_cp_fetch();
void em400_cp_start(bool state);
void em400_cp_bin();
void em400_cp_clear();
void em400_cp_clock(int state);
void em400_cp_oprq();
// rotary
void em400_cp_reg_select(int reg_id);

// -----------------------------------------------------------------------
// --- EM400 EXTENSIONS --------------------------------------------------
// -----------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
