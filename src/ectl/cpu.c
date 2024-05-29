//  Copyright (c) 2016-2022 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>

#include "log.h"
#include "utils/utils.h"
#include "atomic.h"
#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "io/defs.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl/brk.h"
#include "ectl_parser.h"

const char *state_names[] = {
	"RUN",
	"STOP",
	"WAIT",
	"CLM",
	"CLO",
	"OFF",
	"CYCLE",
	"BIN",
	"???"
};

// this must match register order in ectl.h
static const char *ectl_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "AC", "AR", "IR", "SR", "RZ", "KB", "KB",
	"??"
};

typedef struct ectl_yy_buffer_state *YY_BUFFER_STATE;
int ectl_yyparse(struct ectl_est **tree);
YY_BUFFER_STATE ectl_yy_scan_string(char *input);
void ectl_yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
int ectl_init()
{
	LOG(L_ECTL, "ECTL init");
	return 0;
}

// -----------------------------------------------------------------------
void ectl_shutdown()
{
	LOG(L_ECTL, "ECTL shutdown");
	ectl_brk_del_all();
}

// -----------------------------------------------------------------------
int ectl_bus_w_get()
{
    LOG(L_ECTL, "ECTL reg get");
    int bus_w = cp_bus_w_get();
    LOG(L_ECTL, "ECTL bus W get: 0x%04x", bus_w);
    return bus_w;
}

// -----------------------------------------------------------------------
void ectl_regs_get(uint16_t *dest)
{
	LOG(L_ECTL, "ECTL regs get");
	for (int i=0 ; i<ECTL_REG_COUNT ; i++) {
		dest[i] = cp_reg_get(i);
	}
}

// -----------------------------------------------------------------------
int ectl_reg_get(unsigned id)
{
	LOG(L_ECTL, "ECTL reg get");
	int reg = cp_reg_get(id);
	LOG(L_ECTL, "ECTL reg get: %s = 0x%04x", ectl_reg_name(id), reg);
	return reg;
}

// -----------------------------------------------------------------------
int ectl_reg_get_id(char *name)
{
	const char **rname = ectl_reg_names;
	int idx = 0;
	while (idx < ECTL_REG_COUNT) {
		if (!strcasecmp(name, *rname)) {
			return idx;
		}
		idx++;
		rname++;
	}

	return -1;
}

// -----------------------------------------------------------------------
const char * ectl_reg_name(unsigned id)
{
	if (id < ECTL_REG_COUNT) {
		return ectl_reg_names[id];
	} else {
		return ectl_reg_names[ECTL_REG_COUNT];
	}
}

// -----------------------------------------------------------------------
int ectl_reg_set(unsigned id, uint16_t val)
{
	LOG(L_ECTL, "ECTL reg set: %s = 0x%04x", ectl_reg_name(id), val);
	return cp_reg_set(id, val);
}

// -----------------------------------------------------------------------
bool ectl_mem_read_n(int seg, uint16_t addr, uint16_t *dest, unsigned count)
{
	LOG(L_ECTL, "ECTL mem read: %i:0x%04x, %i words", seg, addr, count);
	return cp_mem_read_n(seg, addr, dest, count);
}

// -----------------------------------------------------------------------
bool ectl_mem_write_n(int seg, uint16_t addr, uint16_t *src, unsigned count)
{
	LOG(L_ECTL, "ECTL mem write: %i:0x%04x, %i words", seg, addr, count);
	return cp_mem_write_n(seg, addr, src, count);
}

// -----------------------------------------------------------------------
int ectl_mem_map(int seg)
{
	LOG(L_ECTL, "ECTL mem map");
	int map = mem_get_map(seg);
	LOG(L_ECTL, "ECTL mem map: %i = 0x%04x", seg, map);
	return map;
}

// -----------------------------------------------------------------------
int ectl_mem_cfg(int nb, int ab, int mp, int seg)
{
	uint16_t r = (nb & 0b1111) | (ab << 12);
	uint16_t n = ((mp & 0b1111) << 1) | ((seg & 0b1111) << 5);
	int res = mem_cmd(n, r);
	if (res == IO_OK) {
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
const char * ectl_cpu_state_name(unsigned state)
{
	if (state > ECTL_STATE_UNKNOWN) {
		return state_names[ECTL_STATE_UNKNOWN];
	} else {
		return state_names[state];
	}
}

// -----------------------------------------------------------------------
unsigned ectl_cpu_state_get()
{
	LOG(L_ECTL, "ECTL state get");
	unsigned state = cp_state();
	if (state > ECTL_STATE_UNKNOWN) state = ECTL_STATE_UNKNOWN;
	LOG(L_ECTL, "ECTL state get: %s", state_names[state]);
	return state;
}

// -----------------------------------------------------------------------
void ectl_cpu_start(bool state)
{
	LOG(L_ECTL, "ECTL cpu START: %i", state);
	cp_start(state);
}

// -----------------------------------------------------------------------
void ectl_cpu_cycle()
{
	LOG(L_ECTL, "ECTL cpu CYCLE");
	cp_cycle();
}

// -----------------------------------------------------------------------
void ectl_cpu_off()
{
	LOG(L_ECTL, "ECTL cpu OFF");
	cp_off();
}

// -----------------------------------------------------------------------
void ectl_clock_set(int state)
{
	LOG(L_ECTL, "ECTL clock set: %i", state);
	cp_clock_set(state);
}

// -----------------------------------------------------------------------
bool ectl_clock_get()
{
	int state = cp_clock_get();
	LOG(L_ECTL, "ECTL clock get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
void ectl_cpu_clear()
{
	LOG(L_ECTL, "ECTL cpu CLEAR");
	cp_clear();
}

// -----------------------------------------------------------------------
int ectl_bin()
{
	LOG(L_ECTL, "ECTL binary load");
	return cp_bin();
}

// -----------------------------------------------------------------------
void ectl_oprq()
{
	LOG(L_ECTL, "ECTL OPRQ");
	cp_oprq();
}

// -----------------------------------------------------------------------
int ectl_int_set(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_set(interrupt);
	LOG(L_ECTL, "ECTL int set %i", interrupt);
	return 0;
}

// -----------------------------------------------------------------------
int ectl_int_clear(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_clear(interrupt);
	LOG(L_ECTL, "ECTL int clear %i", interrupt);
	return 0;
}

// -----------------------------------------------------------------------
uint16_t ectl_int_get_chan()
{
	uint16_t chan_int = cp_int_get_chan();
	LOG(L_ECTL, "ECTL get channel interrupts: 0x%04x", chan_int);
	return chan_int;
}

// -----------------------------------------------------------------------
uint32_t ectl_int_get32()
{
	LOG(L_ECTL, "ECTL interrupts get");
	uint16_t rz = ectl_reg_get(ECTL_REG_RZ);
	uint16_t rz_io = ectl_int_get_chan();
	uint32_t rz32 = ((rz & 0b1111111111110000) << 16) | (rz_io << 4) | (rz & 0b1111);
	LOG(L_ECTL, "ECTL interrupts get: 0x%08x", rz32);

	return rz32;
}

// -----------------------------------------------------------------------
const char * ectl_version()
{
	static const char *ver = EM400_VERSION;
	LOG(L_ECTL, "ECTL version: %s", ver);
	return ver;
}

// -----------------------------------------------------------------------
const char * ectl_capa_bit_name(unsigned bitpos)
{
	static const char *capa_names[] = { "MX16", "CRON", "AWP", "UIO", "MEGABOOT", "NOMEMSTOP", "???" };

	if (bitpos < ECTL_CAPA_COUNT) {
		return capa_names[bitpos];
	} else {
		return capa_names[ECTL_CAPA_COUNT];
	}
}

// -----------------------------------------------------------------------
int ectl_capa()
{
	int capa = 0;

	if (cpu_mod_present) capa |= 1 << ECTL_CAPA_MX16;
	if (cpu_mod_active) capa |= 1 << ECTL_CAPA_CRON;
	if (awp_enabled) capa |= 1 << ECTL_CAPA_AWP;
	if (!cpu_user_io_illegal) capa |= 1 << ECTL_CAPA_UIO;
	if (mem_mega_boot()) capa |= 1 << ECTL_CAPA_MEGABOOT;
	//TODO: if (nomem_stop) capa |= 1 << ECTL_CAPA_NOMEMSTOP;

	LOG(L_ECTL, "ECTL capabilities: 0x%04x", capa);
	return capa;
}

// -----------------------------------------------------------------------
bool ectl_load_os_image(FILE *f, const char *name, int seg, uint16_t saddr)
{
	LOG(L_ECTL, "ECTL load: %i:0x%04x %s", seg, saddr, name);

	bool res = false;
	uint16_t *bufw = (uint16_t *) malloc(sizeof(uint16_t) * 0x10000);
	uint16_t *bufr = (uint16_t *) malloc(sizeof(uint16_t) * 0x10000);

	int words_read = fread(bufw, sizeof(uint16_t), 0x10000, f);
	if (words_read <= 0) {
		LOG(L_ECTL, "ECTL load failed to read from file");
		goto cleanup;
	}
	endianswap(bufw, words_read);
	if (!ectl_mem_write_n(seg, saddr, bufw, words_read)) {
		LOG(L_ECTL, "ECTL load failed to write memory");
		goto cleanup;
	}
	LOG(L_ECTL, "ECTL load verify start");
	if (!ectl_mem_read_n(seg, saddr, bufr, words_read)) {
		LOG(L_ECTL, "ECTL load failed to readmemory");
		goto cleanup;
	}
	int cmpres = memcmp(bufw, bufr, words_read * sizeof(uint16_t));
	LOG(L_ECTL, "ECTL load verify (%i words): %s", words_read, cmpres ? "FAILED" : "OK");
	if (cmpres != 0) {
		goto cleanup;
	}

	res = true;
cleanup:
	free(bufw);
	free(bufr);
	return res;
}

// -----------------------------------------------------------------------
int ectl_log_state_get()
{
	int state = log_is_enabled();
	LOG(L_ECTL, "ECTL log state get: %i", state);
	if (state) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
int ectl_log_state_set(int state)
{
	int res = -1;
	LOG(L_ECTL, "ECTL log state set: %i", state);
	if (state == ECTL_OFF) {
		log_disable();
		res = 0;
	} else if (state == ECTL_ON) {
		res = log_enable();
	}
	return res;
}

// -----------------------------------------------------------------------
int ectl_log_component_get(unsigned component)
{
	int state = log_component_get(component) ? 1 : 0;
	LOG(L_ECTL, "ECTL log component %s: %i", ectl_log_component_name(component), state);
	return state;
}

// -----------------------------------------------------------------------
int ectl_log_component_set(unsigned component, int state)
{
	LOG(L_ECTL, "ECTL log component %s: %i", ectl_log_component_name(component), state);
	if (state == ECTL_OFF) {
		log_component_disable(component);
	} else {
		log_component_enable(component);
	}
	return state;
}

// -----------------------------------------------------------------------
const char * ectl_log_component_name(unsigned component)
{
	return log_get_component_name(component);
}

// -----------------------------------------------------------------------
int ectl_log_component_id(char *name)
{
	return log_get_component_id(name);
}

// -----------------------------------------------------------------------
extern unsigned long ips_counter;
unsigned long ectl_ips_get()
{
	double ips;
	static unsigned long oips;

	double elapsed_ns = stopwatch_ns();
	if (elapsed_ns > 0) {
		ips = (1000000000.0 * (ips_counter - oips)) / elapsed_ns;
	} else {
		ips = 0;
	}
	oips = ips_counter;

	LOG(L_ECTL, "ECTL ips: %li", ips);
	return ips;
}

// -----------------------------------------------------------------------
static struct ectl_est * __ectl_parse(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct ectl_est *tree;

	YY_BUFFER_STATE yb = ectl_yy_scan_string(expression);
	ectl_yyparse(&tree);
	ectl_yy_delete_buffer(yb);

	if (!tree) {
		*err_msg = strdup("Fatal error, parser did not return anything");
		return NULL;
	}

	if (tree->type == ECTL_AST_N_ERR) {
		*err_msg = strdup(tree->err);
		*err_beg = tree->c_beg;
		*err_end = tree->c_end;
		ectl_est_delete(tree);
		return NULL;
	}

	return tree;
}

// -----------------------------------------------------------------------
int ectl_eval(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL eval: %s", expression);
	int res = -1;
	struct ectl_est *tree = __ectl_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		goto fin;
	}

	res = ectl_est_eval(tree);
	if (res < 0) {
		struct ectl_est *err_node = ectl_est_get_eval_err();
		if (err_node) {
			*err_msg = strdup(err_node->err);
			*err_beg = err_node->c_beg;
			*err_end = err_node->c_end;
		} else {
			*err_msg = strdup("Evaluation failed");
		}
		goto fin;
	}

fin:
	ectl_est_delete(tree);
	LOG(L_ECTL, "ECTL eval: %s = %i %s", expression, res, err_msg);
	return res;
}

// -----------------------------------------------------------------------
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL brk add: %s", expression);
	struct ectl_est *tree = __ectl_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	int id = ectl_brk_insert(tree, expression);
	if (id < 0) {
		*err_msg = strdup("Cannot add new breakpoint");
		ectl_est_delete(tree);
		return -1;
	}

	return id;
}

// -----------------------------------------------------------------------
int ectl_brk_del(unsigned id)
{
	LOG(L_ECTL, "ECTL brk del: %i", id);
	return ectl_brk_delete(id);
}

// -----------------------------------------------------------------------
int ectl_stopn(bool state)
{
	LOG(L_ECTL, "ECTL stopn: %i", state);
	return cp_stopn(state);
}

// -----------------------------------------------------------------------
void ectl_reg_select(int reg)
{
	LOG(L_ECTL, "ECTL reg select");
	cp_reg_select(reg);
}

// -----------------------------------------------------------------------
int ectl_alarm_get()
{
	bool state = cp_alarm_get();
	LOG(L_ECTL, "ECTL ALARM get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_p_get()
{
	bool state = cp_p_get();
	LOG(L_ECTL, "ECTL P get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_mc_get()
{
	bool state = cp_mc_get();
	LOG(L_ECTL, "ECTL MC get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_irq_get()
{
	bool state = cp_irq_get();
	LOG(L_ECTL, "ECTL IRQ get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_run_get()
{
	bool state = cp_run_get();
	LOG(L_ECTL, "ECTL RUN get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_wait_get()
{
	bool state = cp_wait_get();
	LOG(L_ECTL, "ECTL WAIT get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
bool ectl_q_get()
{
	bool state = cp_q_get();
	LOG(L_ECTL, "ECTL Q get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
int ectl_nb_get()
{
	int nb = cp_nb_get();
	LOG(L_ECTL, "ECTL NB get: %i", nb);
	return nb;
}

// -----------------------------------------------------------------------
int ectl_qnb_get()
{
	int qnb = cp_qnb_get();
	LOG(L_ECTL, "ECTL QNB get: %i", qnb);
	return qnb;
}

// -----------------------------------------------------------------------
void ectl_kb_set(uint16_t val)
{
	LOG(L_ECTL, "ECTL KB set: 0x%04x", val);
	cp_kb_set(val);
}

// -----------------------------------------------------------------------
void ectl_load()
{
	LOG(L_ECTL, "ECTL LOAD");
	cp_load();
}

// -----------------------------------------------------------------------
void ectl_store()
{
	LOG(L_ECTL, "ECTL STORE");
	cp_store();
}

// -----------------------------------------------------------------------
void ectl_fetch()
{
	LOG(L_ECTL, "ECTL fetch");
	cp_fetch();
}

// vim: tabstop=4 shiftwidth=4 autoindent
