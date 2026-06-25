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
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif



// -----------------------------------------------------------------------
// --- LIMITS ------------------------------------------------------------
// -----------------------------------------------------------------------

#define EM400_IO_MAX_CHAN 16
#define EM400_CHAN_MAX_DEV 32
#define EM400_SP45DE_SLOT_COUNT 4

// -----------------------------------------------------------------------
// --- DEVICE/CHANNEL TYPES ----------------------------------------------
// -----------------------------------------------------------------------

enum em400_device_types {
	EM400_DEV_NONE = 0,
	EM400_DEV_TERMINAL,
	EM400_DEV_SP45DE,
	EM400_DEV_WINCHESTER,
	EM400_DEV_FLOP5,
	EM400_DEV_RTCLOCK,
};

enum em400_channel_types {
	EM400_CHANNEL_NONE = 0,
	EM400_CHANNEL_CHAR,
	EM400_CHANNEL_MULTIX,
	EM400_CHANNEL_IOTESTER,
	EM400_CHANNEL_TYPE_COUNT
};

// -----------------------------------------------------------------------
// --- HOST CONFIGURATION ------------------------------------------------
// -----------------------------------------------------------------------

struct em400_sound_cfg {
	bool enabled;
	int volume;
	int sample_rate;
	int buffer_len;
	int latency;
	const char *backend;
	const char *device;
};

struct em400_emulation_cfg {
	bool speed_real;
	int emulation_quantum_us;
};

struct em400_host_cfg {
	struct em400_emulation_cfg emu;
	struct em400_sound_cfg sound;
};

// -----------------------------------------------------------------------
// --- MACHINE CONFIGURATION ---------------------------------------------
// -----------------------------------------------------------------------

struct em400_cpu_cfg {
	bool awp;
	bool mod;
	bool user_io_illegal;
	bool nomem_stop;
	int clock_period_ms;
};

struct em400_mem_cfg {
	int elwro_modules;
	int mega_modules;
	int os_segments;
	const char *mega_prom_image;
	const char *preload_image;
};

struct em400_device_cfg {
	enum em400_device_types type;
	union {
		struct { int port; int speed; } terminal;
		struct { const char *image; } winchester;
		struct { const char *prom; } rtclock;
		struct { const char *images[EM400_SP45DE_SLOT_COUNT]; } sp45de;
	};
};

struct em400_channel_cfg {
	enum em400_channel_types type;
	struct em400_device_cfg device[EM400_CHAN_MAX_DEV];
};

struct em400_machine_cfg {
	struct em400_cpu_cfg cpu;
	struct em400_mem_cfg mem;
	struct em400_channel_cfg channel[EM400_IO_MAX_CHAN];
};

// -----------------------------------------------------------------------
// --- ENUMS -------------------------------------------------------------
// -----------------------------------------------------------------------

enum em400_reg_ids {
	EM400_REG_R0 = 0,
	EM400_REG_R1,
	EM400_REG_R2,
	EM400_REG_R3,
	EM400_REG_R4,
	EM400_REG_R5,
	EM400_REG_R6,
	EM400_REG_R7,
	EM400_REG_IC,
	EM400_REG_AC,
	EM400_REG_AR,
	EM400_REG_IR,
	EM400_REG_SR,
	EM400_REG_RZ,
	EM400_REG_KB,
	EM400_REG_KB2
};

enum em400_return_codes_e {
	E_ERR = -1,
	E_OK = 0,
};

typedef enum em400_log_buf_type_e {
	EM400_LOG_FULL_BUFFERED,
	EM400_LOG_LINE_BUFFERED
} em400_log_buf_type_t;

extern const char *em400_reg_names[];

#define EM400_REG_COUNT EM400_REG_KB2

enum em400_log_components {
	L_ALL = 0,
	L_EM4H, L_CRK5,
	L_MEM, L_CPU, L_OP, L_INT, L_IO,
	L_MX, L_CCHR, L_CMEM,
	L_UZDAT, L_UZFX, L_MECLO,
	L_TERM, L_9425, L_WNCH, L_FLOP, L_PNCH, L_PNRD, L_TAPE,
	L_COUNT,
};

enum em400_cpu_states {
	EM400_STATE_RUN = 0,
	EM400_STATE_STOP,
	EM400_STATE_WAIT,
	EM400_STATE_CLO,
	EM400_STATE_OFF,
	EM400_STATE_CYCLE,
	EM400_STATE_BIN,
	EM400_STATE_LOAD,
	EM400_STATE_STORE,
	EM400_STATE_FETCH,
	EM400_STATE_ANY,
	EM400_STATE_UNKNOWN,
};

// -----------------------------------------------------------------------
// --- LIBRARY -----------------------------------------------------------
// -----------------------------------------------------------------------

const char * em400_version();
int em400_init(const struct em400_machine_cfg *machine, const struct em400_host_cfg *host);
void em400_shutdown();

// -----------------------------------------------------------------------
// --- I/O ---------------------------------------------------------------
// -----------------------------------------------------------------------

int em400_channel_max_devices(enum em400_channel_types type);
int em400_dev_type(unsigned chnum, unsigned devnum);
int em400_dev_slot_count(unsigned chnum, unsigned devnum);
bool em400_dev_can_eject(unsigned chnum, unsigned devnum, unsigned slot);
int em400_dev_eject(unsigned chnum, unsigned devnum, unsigned slot);
int em400_dev_load_image(unsigned chnum, unsigned devnum, unsigned slot, const char *image);
const char * em400_dev_get_image(unsigned chnum, unsigned devnum, unsigned slot);

// -----------------------------------------------------------------------
// --- LOGGING -----------------------------------------------------------
// -----------------------------------------------------------------------

int em400_log_init(const char *file, em400_log_buf_type_t buf_type, const char *components, bool enabled);
void em400_log_shutdown();
bool em400_log_state();
int em400_log_set(bool state);
bool em400_log_component_state(unsigned component);
int em400_log_component_set(unsigned component, bool state);
const char * em400_log_component_name(unsigned component);
int em400_log_component_id(const char *name);


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

// keys

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
// ignition
// off?

// -----------------------------------------------------------------------
// --- EM400 CONTROL PANEL EXTENSIONS ------------------------------------
// -----------------------------------------------------------------------

unsigned em400_reg_id(char *name);
const char * em400_reg_name(unsigned reg_id);
int em400_reg(unsigned reg_id);
void em400_regs(uint16_t *dest);
void em400_reg_set(unsigned reg_id, uint16_t val);
unsigned em400_nb();
unsigned em400_qnb();
unsigned em400_mc();

uint32_t em400_rz32();
int em400_int_set(unsigned interrupt);
int em400_int_clear(unsigned interrupt);
int em400_int_mask_bit(unsigned interrupt);

bool em400_mem_read(int seg, uint16_t addr, uint16_t *dest, unsigned count);
bool em400_mem_write(int seg, uint16_t addr, uint16_t *src, unsigned count);
int em400_mem_map(int seg);
int em400_load_os_image(FILE *f);
int em400_load_os_image_path(const char *path);

unsigned long em400_ips_get();
const char * em400_cpu_state_name(unsigned state);
unsigned em400_cpu_state();

// -----------------------------------------------------------------------
// --- BREAKPOINTS -------------------------------------------------------
// -----------------------------------------------------------------------

typedef void (*em400_brk_cb)(unsigned id, const char *expr, bool enabled, void *ctx);

int em400_brk_add(char *expr, char **err_msg, int *err_beg, int *err_end);
int em400_brk_delete(unsigned id);
int em400_brk_enable(unsigned id, bool enabled);
int em400_brk_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end);
void em400_brk_foreach(em400_brk_cb cb, void *ctx);
int em400_brk_hit();

// -----------------------------------------------------------------------
// --- EXPRESSION EVALUATION ---------------------------------------------
// -----------------------------------------------------------------------

int em400_eval(char *expr, int *result, char **err_msg, int *err_beg, int *err_end);

// -----------------------------------------------------------------------
// --- WATCHES -----------------------------------------------------------
// -----------------------------------------------------------------------

typedef void (*em400_watch_cb)(unsigned id, const char *expr, void *ctx);

int em400_watch_add(char *expr, char **err_msg, int *err_beg, int *err_end);
int em400_watch_delete(unsigned id);
int em400_watch_edit(unsigned id, char *expr, char **err_msg, int *err_beg, int *err_end);
int em400_watch_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end);
void em400_watch_foreach(em400_watch_cb cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
