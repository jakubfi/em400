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

#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdatomic.h>
#ifdef _WIN32
#include <winsock2.h> // must precede windows.h, else libuv's uv.h warns
#include <windows.h>
#include <mmsystem.h>
#endif

#include "libem400.h"
#include "mem/mem.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "io/io.h"
#include "io/chan.h"
#include "cp/cp.h"
#include "cp/brk.h"
#include "cp/eval.h"
#include "cp/watch.h"
#include "utils/utils.h"
#include "log.h"
#include "io/dev/terminal.h"
#include "io/dev/sp45de.h"
#include "io/dev/winchester.h"
#include "io/dev/rtclock.h"

static const char ver[] = EM400_VERSION;

const char *em400_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "AC", "AR", "IR", "SR", "RZ", "KB", "KB",
	"??"
};

const char *em400_cpu_state_names[] = {
	"RUN",
	"STOP",
	"WAIT",
	"CLO",
	"OFF",
	"CYCLE",
	"BIN",
	"LOAD",
	"STORE",
	"FETCH",
	"ANY",
	"???"
};


// -----------------------------------------------------------------------
// --- LIBRARY -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
const char * em400_version()
{
	return ver;
}

// -----------------------------------------------------------------------
static int device_build(unsigned chnum, unsigned devnum, const struct em400_device_cfg *dcfg)
{
	em400_dev_t *dev = NULL;

	switch (dcfg->type) {
		case EM400_DEV_NONE:
			return E_OK;
		case EM400_DEV_TERMINAL:
			dev = terminal_create(dcfg->terminal.port, dcfg->terminal.speed);
			break;
		case EM400_DEV_WINCHESTER:
			dev = winchester_create(dcfg->winchester.image);
			break;
		case EM400_DEV_SP45DE:
			dev = sp45de_create();
			if (dev) {
				for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
					if (dcfg->sp45de.images[slot]) {
						dev->load(dev, slot, dcfg->sp45de.images[slot]);
					}
				}
			}
			break;
		case EM400_DEV_RTCLOCK:
			dev = rtclock_create(dcfg->rtclock.prom);
			break;
		case EM400_DEV_FLOP5:
			return E_OK;
		default:
			return LOGERR("Device %i.%i: unknown device type: %i", chnum, devnum, dcfg->type);
	}

	if (!dev) {
		LOG(L_LIB, "Device %i.%i: creation failed", chnum, devnum);
		return E_ERR;
	}
	if (io_dev_connect(chnum, devnum, dev) != E_OK) {
		LOG(L_LIB, "Device %i.%i: failed to connect to channel", chnum, devnum);
		return E_ERR;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
static int channel_build(unsigned chnum, const struct em400_channel_cfg *ccfg)
{
	if (ccfg->type == EM400_CHANNEL_NONE) {
		return E_OK;
	}

	LOG(L_LIB, "Initializing I/O channel %i (type %i)", chnum, ccfg->type);
	if (io_channel_init(chnum, ccfg->type) != E_OK) {
		LOG(L_LIB, "Channel %i initialization error", chnum);
		return E_ERR;
	}

	for (unsigned devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
		if (device_build(chnum, devnum, &ccfg->device[devnum]) != E_OK) {
			LOG(L_LIB, "Channel %i: device %i initialization error", chnum, devnum);
			return E_ERR;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int em400_init(const struct em400_machine_cfg *machine, const struct em400_host_cfg *host)
{
#ifdef _WIN32
	timeBeginPeriod(1);
#endif
	if (mem_init(&machine->mem) != E_OK) {
		LOG(L_LIB, "Failed to initialize memory.");
		return E_ERR;
	}
	if (cpu_init(host, machine) != E_OK) {
		LOG(L_LIB, "Failed to initialize CPU.");
		return E_ERR;
	}
	if (io_init() != E_OK) {
		LOG(L_LIB, "Failed to initialize I/O.");
		return E_ERR;
	}

	for (unsigned chnum=0 ; chnum<EM400_IO_MAX_CHAN ; chnum++) {
		if (channel_build(chnum, &machine->channel[chnum]) != E_OK) {
			LOG(L_LIB, "Failed to initialize I/O channels.");
			return E_ERR;
		}
	}

	if (io_run() != E_OK) {
		LOG(L_LIB, "Failed to start the I/O loop.");
		return E_ERR;
	}

	if (machine->mem.preload_image && *machine->mem.preload_image && !em400_load_os_image_path(machine->mem.preload_image)) {
		return LOGERR("Failed to preload OS image: %s", machine->mem.preload_image);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_shutdown()
{
	LOG(L_LIB, "Shutting down EM400 instance");
#ifdef _WIN32
	timeEndPeriod(1);
#endif
	io_shutdown();
	cpu_shutdown();
	brk_del_all();
	watch_del_all();
	mem_shutdown();
}

// -----------------------------------------------------------------------
int em400_channel_max_devices(enum em400_channel_types type)
{
	return chan_max_devices(type);
}

// -----------------------------------------------------------------------
int em400_dev_type(unsigned chnum, unsigned devnum)
{
	return io_dev_type(chnum, devnum);
}

// -----------------------------------------------------------------------
int em400_dev_slot_count(unsigned chnum, unsigned devnum)
{
	return io_dev_slot_count(chnum, devnum);
}

// -----------------------------------------------------------------------
bool em400_dev_can_eject(unsigned chnum, unsigned devnum, unsigned slot)
{
	return io_dev_can_eject(chnum, devnum, slot);
}

// -----------------------------------------------------------------------
int em400_dev_eject(unsigned chnum, unsigned devnum, unsigned slot)
{
	return io_dev_eject( chnum, devnum, slot);
}

// -----------------------------------------------------------------------
int em400_dev_load_image(unsigned chnum, unsigned devnum, unsigned slot, const char *image)
{
	return io_dev_load_image(chnum, devnum, slot, image);
}

// -----------------------------------------------------------------------
const char * em400_dev_get_image(unsigned chnum, unsigned devnum, unsigned slot)
{
	return io_dev_get_image(chnum, devnum, slot);
}

// -----------------------------------------------------------------------
// --- LOGGING -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void em400_logf(const char *func, const char *fmt, ...)
{
	if (!LOG_WANTS(L_APP)) return;

	va_list vl;
	va_start(vl, fmt);
	log_log_va(L_APP, func, fmt, vl);
	va_end(vl);
}

// -----------------------------------------------------------------------
int em400_log_init(const char *file, em400_log_buf_type_t buf_type, const char *components, bool enabled)
{
	if (log_init(file, buf_type) != E_OK) {
		return LOGERR("Failed to initialize logging");
	}
	if (enabled) {
		if (log_enable() != E_OK) {
			return LOGERR("Failed to enable logging");
		}
	}
	if (log_setup_components(components) != E_OK) {
		return LOGERR("Failed to set which components to log");
	}
	return E_OK;
}

// -----------------------------------------------------------------------
const char * em400_msg_take(em400_sev_t *sev)
{
	return log_msg_take(sev);
}

// -----------------------------------------------------------------------
void em400_log_shutdown()
{
	log_shutdown();
}

// -----------------------------------------------------------------------
bool em400_log_state()
{
	return log_is_enabled() ? true : false;
}

// -----------------------------------------------------------------------
int em400_log_set(bool state)
{
	if (state) {
		return log_enable();
	} else {
		log_disable();
		return E_OK;
	}
}

// -----------------------------------------------------------------------
bool em400_log_component_state(unsigned component)
{
	return log_component_get(component) ? true : false;
}

// -----------------------------------------------------------------------
int em400_log_component_set(unsigned component, bool state)
{
	if (state) {
		log_component_enable(component);
	} else {
		log_component_disable(component);
	}
	return state;
}

// -----------------------------------------------------------------------
const char * em400_log_component_name(unsigned component)
{
	return log_get_component_name(component);
}

// -----------------------------------------------------------------------
int em400_log_component_id(const char *name)
{
	return log_get_component_id(name);
}


// -----------------------------------------------------------------------
// --- CONTROL PANEL -----------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
uint16_t em400_cp_w_leds()
{
	return cp_bus_w();
}

// -----------------------------------------------------------------------
bool em400_cp_run_led()
{
	return cp_run_get();
}

// -----------------------------------------------------------------------
bool em400_cp_wait_led()
{
	return cp_wait_get();
}

// -----------------------------------------------------------------------
bool em400_cp_alarm_led()
{
	return cp_alarm_get();
}

// -----------------------------------------------------------------------
bool em400_cp_clock_led()
{
	return cp_clock_get();
}

// -----------------------------------------------------------------------
bool em400_cp_q_led()
{
	return cp_q_get();
}

// -----------------------------------------------------------------------
bool em400_cp_p_led()
{
	return cp_p_get();
}

// -----------------------------------------------------------------------
bool em400_cp_mc_led()
{
	return cp_mc_get();
}

// -----------------------------------------------------------------------
bool em400_cp_irq_led()
{
	return cp_irq_get();
}

// -----------------------------------------------------------------------
void em400_cp_kb(uint16_t val)
{
	cp_kb_set(val);
}

// -----------------------------------------------------------------------
void em400_cp_stopn(bool state)
{
	cp_stopn(state);
}

// -----------------------------------------------------------------------
void em400_cp_cycle()
{
	cp_cycle();
}

// -----------------------------------------------------------------------
void em400_cp_load()
{
	cp_load();
}

// -----------------------------------------------------------------------
void em400_cp_store()
{
	cp_store();
}

// -----------------------------------------------------------------------
void em400_cp_fetch()
{
	cp_fetch();
}

// -----------------------------------------------------------------------
void em400_cp_start(bool state)
{
	cp_start(state);
}

// -----------------------------------------------------------------------
void em400_cp_bin()
{
	cp_bin();
}

// -----------------------------------------------------------------------
void em400_cp_clear()
{
	cp_clear();
}

// -----------------------------------------------------------------------
void em400_cp_clock(int state)
{
	cp_clock_set(state);
}

// -----------------------------------------------------------------------
void em400_cp_oprq()
{
	cp_oprq();
}

// -----------------------------------------------------------------------
void em400_cp_reg_select(int reg_id)
{
	cp_reg_select(reg_id);
}

// -----------------------------------------------------------------------
// --- EM400 EXTENSIONS --------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
unsigned int em400_reg_id(char *name)
{
	const char **rname = em400_reg_names;
	int idx = 0;
	while (idx < EM400_REG_COUNT) {
		if (!strcasecmp(name, *rname)) {
			return idx;
		}
		idx++;
		rname++;
	}

	return -1;
}

// -----------------------------------------------------------------------
const char * em400_reg_name(unsigned reg_id)
{
	if (reg_id < EM400_REG_COUNT) {
		return em400_reg_names[reg_id];
	} else {
		return em400_reg_names[EM400_REG_COUNT];
	}
}

// -----------------------------------------------------------------------
int em400_reg(unsigned reg_id)
{
	return cpu_reg_fetch(reg_id);
}

// -----------------------------------------------------------------------
void em400_regs(uint16_t *dest)
{
	for (int i=0 ; i<EM400_REG_COUNT ; i++) {
		dest[i] = cpu_reg_fetch(i);
	}
}

// -----------------------------------------------------------------------
void em400_reg_set(unsigned reg_id, uint16_t val)
{
	cpu_reg_load(reg_id, val);
}

// -----------------------------------------------------------------------
unsigned em400_nb()
{
	return nb;
}

// -----------------------------------------------------------------------
unsigned em400_mc()
{
	return mc;
}

// -----------------------------------------------------------------------
unsigned em400_qnb()
{
	return q * nb;
}

// -----------------------------------------------------------------------
uint32_t em400_rz32()
{
	return rz;
}

// -----------------------------------------------------------------------
int em400_int_set(unsigned interrupt)
{
	if (interrupt >= 32) {
		return E_ERR;
	}
	int_set(interrupt);
	return E_OK;
}

// -----------------------------------------------------------------------
int em400_int_clear(unsigned interrupt)
{
	if (interrupt >= 32) {
		return E_ERR;
	}
	int_clear(interrupt);
	return E_OK;
}

// -----------------------------------------------------------------------
int em400_int_mask_bit(unsigned interrupt)
{
	return int_get_mask_bit(interrupt);
}

// -----------------------------------------------------------------------
bool em400_mem_read(int seg, uint16_t addr, uint16_t *dest, unsigned count)
{
	return mem_read_n(seg, addr, dest, count);
}

// -----------------------------------------------------------------------
bool em400_mem_write(int seg, uint16_t addr, uint16_t *src, unsigned count)
{
	return mem_write_n(seg, addr, src, count);
}

// -----------------------------------------------------------------------
int em400_mem_map(int seg)
{
	return mem_get_map(seg);
}

// -----------------------------------------------------------------------
int em400_load_os_image(FILE *f)
{
	uint16_t buf[0x2000];

	int words_read = fread(buf, sizeof(uint16_t), 0x2000, f);
	if (words_read <= 0) {
		return 0;
	}
	endianswap(buf, words_read);
	if (!mem_write_n(0, 0, buf, words_read)) {
		return 0;
	}

	return words_read;
}

// -----------------------------------------------------------------------
int em400_load_os_image_path(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		LOG(L_LIB, "Failed to open OS image file: %s", path);
		return 0;
	}
	int words = em400_load_os_image(f);
	fclose(f);
	if (!words) {
		LOG(L_LIB, "Failed to load OS image file: %s", path);
	}
	return words;
}

// -----------------------------------------------------------------------
unsigned long em400_ips_get()
{
	double ips;
	static unsigned long oips;

	unsigned long ips_now = atomic_load_explicit(&ips_counter, memory_order_relaxed);
	double elapsed_ns = stopwatch_ns();
	if (elapsed_ns > 0) {
		ips = (1000000000.0 * (ips_now - oips)) / elapsed_ns;
	} else {
		ips = 0;
	}
	oips = ips_now;

	return ips;
}

// -----------------------------------------------------------------------
const char * em400_cpu_state_name(unsigned state)
{
	if (state > EM400_STATE_UNKNOWN) {
		return em400_cpu_state_names[EM400_STATE_UNKNOWN];
	} else {
		return em400_cpu_state_names[state];
	}
}

// -----------------------------------------------------------------------
unsigned em400_cpu_state()
{
	return cpu_state_get();
}

// -----------------------------------------------------------------------
// --- BREAKPOINTS -------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int em400_brk_add(char *expr, char **err_msg, int *err_beg, int *err_end)
{
	return brk_add(expr, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
int em400_brk_edit(unsigned id, char *expr, char **err_msg, int *err_beg, int *err_end)
{
	return brk_edit(id, expr, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
int em400_brk_delete(unsigned id)
{
	return brk_delete(id);
}

// -----------------------------------------------------------------------
int em400_brk_enable(unsigned id, bool enabled)
{
	return brk_enable(id, enabled);
}

// -----------------------------------------------------------------------
int em400_brk_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end)
{
	return brk_eval(id, result, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
void em400_brk_foreach(em400_brk_cb cb, void *ctx)
{
	brk_foreach(cb, ctx);
}

// -----------------------------------------------------------------------
int em400_brk_hit()
{
	return brk_hit_get();
}

// -----------------------------------------------------------------------
// --- EXPRESSION EVALUATION ---------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int em400_eval(char *expr, int *result, char **err_msg, int *err_beg, int *err_end)
{
	int res = eval_str_eval(expr, err_msg, err_beg, err_end);
	if (res < 0) {
		return E_ERR;
	}
	*result = res;
	return E_OK;
}

// -----------------------------------------------------------------------
// --- WATCHES -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int em400_watch_add(char *expr, char **err_msg, int *err_beg, int *err_end)
{
	return watch_add(expr, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
int em400_watch_delete(unsigned id)
{
	return watch_delete(id);
}

// -----------------------------------------------------------------------
int em400_watch_edit(unsigned id, char *expr, char **err_msg, int *err_beg, int *err_end)
{
	return watch_edit(id, expr, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
int em400_watch_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end)
{
	return watch_eval(id, result, err_msg, err_beg, err_end);
}

// -----------------------------------------------------------------------
void em400_watch_foreach(em400_watch_cb cb, void *ctx)
{
	watch_foreach(cb, ctx);
}

// vim: tabstop=4 shiftwidth=4 autoindent
