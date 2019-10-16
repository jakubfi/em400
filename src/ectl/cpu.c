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
#include "mem/mem.h"
#include "io/defs.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl/brk.h"
#include "ectl_parser.h"

// this must match register order in ectl.h
static const char *ectl_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "AC", "AR", "IR", "SR", "RZ", "KB", "KB",
	"MOD", "MODc", "ALARM", "RM", "Q", "BS", "NB",
	"P", "RZ_IO",
	"??"
};

typedef struct ectl_yy_buffer_state *YY_BUFFER_STATE;
int ectl_yyparse();
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
int ectl_mem_get(int seg, uint16_t addr, uint16_t *dest, unsigned count)
{
	LOG(L_ECTL, "ECTL mem get: %i:0x%04x, %i words", seg, addr, count);
	return cp_mem_mget(seg, addr, dest, count);
}

// -----------------------------------------------------------------------
int ectl_mem_set(int seg, uint16_t addr, uint16_t *src, unsigned count)
{
	LOG(L_ECTL, "ECTL mem set: %i:0x%04x, %i words", seg, addr, count);
	return cp_mem_mput(seg, addr, src, count);
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
int ectl_cpu_state_get()
{
	LOG(L_ECTL, "ECTL state get");
	int state = cp_state();
	LOG(L_ECTL, "ECTL state get: 0x%04x", state);
	return state;
}

// -----------------------------------------------------------------------
const char * ectl_cpu_state_bit_name(int bitpos)
{
	const char *state_names[] = { "STOP", "WAIT", "CLM", "CLO", "OFF", "BRK", "CYCLE" };
	const char *state_unknown = "???";
	const int state_count = sizeof(state_names) / sizeof(char*);

	if (bitpos < state_count) {
		return state_names[bitpos];
	} else {
		return state_unknown;
	}
}

// -----------------------------------------------------------------------
void ectl_cpu_stop()
{
	LOG(L_ECTL, "ECTL cpu STOP");
	cp_stop();
}

// -----------------------------------------------------------------------
void ectl_cpu_start()
{
	LOG(L_ECTL, "ECTL cpu START");
	cp_start();
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
int ectl_clock_get()
{
	LOG(L_ECTL, "ECTL clock get");
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
void ectl_bootstrap(int chan, int unit)
{
	LOG(L_ECTL, "ECTL bootstrap");
	cp_bin();
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
	LOG(L_ECTL, "ECTL int set %i", interrupt);
	return cp_int_set(interrupt);
}

// -----------------------------------------------------------------------
int ectl_int_clear(unsigned interrupt)
{
	LOG(L_ECTL, "ECTL int clear %i", interrupt);
	return cp_int_clear(interrupt);
}

// -----------------------------------------------------------------------
uint32_t ectl_int_get32()
{
	LOG(L_ECTL, "ECTL interrupts get");
	uint16_t rz = ectl_reg_get(ECTL_REG_RZ);
	uint16_t rz_io = ectl_reg_get(ECTL_REG_RZ_IO);
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
	if (awp) capa |= 1 << ECTL_CAPA_AWP;
	if (!cpu_user_io_illegal) capa |= 1 << ECTL_CAPA_UIO;
	if (mem_mega_boot()) capa |= 1 << ECTL_CAPA_MEGABOOT;
	//TODO: if (nomem_stop) capa |= 1 << ECTL_CAPA_NOMEMSTOP;

	LOG(L_ECTL, "ECTL capabilities: 0x%04x", capa);
	return capa;
}

// -----------------------------------------------------------------------
int ectl_load(FILE *f, const char *name, int seg, uint16_t saddr)
{
	LOG(L_ECTL, "ECTL load: %i:0x%04x %s", seg, saddr, name);
	uint16_t *bufw = (uint16_t *) malloc(sizeof(uint16_t) * 0x10000);
	uint16_t *bufr = (uint16_t *) malloc(sizeof(uint16_t) * 0x10000);

	int res = fread(bufw, sizeof(uint16_t), 0x10000, f);
	if (res > 0) {
		endianswap(bufw, res);
		res = ectl_mem_set(seg, saddr, bufw, res);
		LOG(L_ECTL, "ECTL verify");
		ectl_mem_get(seg, saddr, bufr, res);
		int cmpres = memcmp(bufw, bufr, res * sizeof(uint16_t));
		LOG(L_ECTL, "ECTL verify (%i words): %s", res, cmpres ? "FAILED" : "OK");
		if (cmpres != 0) {
			res = -1;
		}
	}

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
int ectl_stopn(uint16_t addr)
{
	LOG(L_ECTL, "ECTL stopn (%s) @ 0x%04x", (addr&0x8000) ? "OS" : "USER", addr&0x7fff);
	return cp_stopn(addr);
}

// -----------------------------------------------------------------------
int ectl_stopn_off()
{
	LOG(L_ECTL, "ECTL stopn off");
	return cp_stopn_off();
}

// vim: tabstop=4 shiftwidth=4 autoindent
