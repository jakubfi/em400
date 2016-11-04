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
#include "utils.h"
#include "atomic.h"
#include "cpu/cpu.h"
#include "cpu/timer.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl/brk.h"
#include "ectl_parser.h"

static const char *ectl_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "SR", "IR", "KB", "MOD", "MODc", "ALARM",
	"??"
};

typedef struct ectl_yy_buffer_state *YY_BUFFER_STATE;
int ectl_yyparse();
YY_BUFFER_STATE ectl_yy_scan_string(char *input);
void ectl_yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
int ectl_init()
{
	LOG(L_ECTL, 1, "ECTL init");
	return 0;
}

// -----------------------------------------------------------------------
void ectl_shutdown()
{
	LOG(L_ECTL, 1, "ECTL shutdown");
	ectl_brk_del_all();
}

// -----------------------------------------------------------------------
void ectl_regs_get(uint16_t *dest)
{
	LOG(L_ECTL, 2, "ECTL regs get");
	memcpy(dest, regs, 8 * sizeof(uint16_t));
	dest[ECTL_REG_IC] = rIC;
	dest[ECTL_REG_SR] = rSR;
	dest[ECTL_REG_IR] = rIR;
	dest[ECTL_REG_KB] = rKB;
	dest[ECTL_REG_MOD] = rMOD;
	dest[ECTL_REG_MODc] = rMODc;
	dest[ECTL_REG_ALARM] = rALARM;
}

// -----------------------------------------------------------------------
int ectl_reg_get(unsigned id)
{
	if (id >= ECTL_REG_COUNT) {
		return -1;
	}

	int reg;
	switch (id) {
		case ECTL_REG_IC: reg = rIC; break;
		case ECTL_REG_SR: reg = rSR; break;
		case ECTL_REG_IR: reg = rIR; break;
		case ECTL_REG_KB: reg = rKB; break;
		case ECTL_REG_MOD: reg = rMOD; break;
		case ECTL_REG_MODc: reg = rMODc; break;
		case ECTL_REG_ALARM: reg = rALARM; break;
		default: reg = regs[id]; break;
	}
	LOG(L_ECTL, 2, "ECTL reg get: %i = 0x%04x", id, reg);
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
	if (id >= ECTL_REG_COUNT) {
		return -1;
	}

	LOG(L_ECTL, 2, "ECTL reg set: %i = 0x%04x", id, val);
	switch (id) {
		case ECTL_REG_IC: rIC = val; break;
		case ECTL_REG_SR: rSR = val; break;
		case ECTL_REG_IR: rIR = val; break;
		case ECTL_REG_KB: rKB = val; break;
		case ECTL_REG_MOD: rMOD = val; break;
		case ECTL_REG_MODc: rMODc = val; break;
		case ECTL_REG_ALARM: rALARM = val; break;
		default: regs[id] = val; break;
	}
	return 0;
}

// -----------------------------------------------------------------------
int ectl_mem_get(int seg, uint16_t addr, uint16_t *dest, int count)
{
	LOG(L_ECTL, 2, "ECTL mem get: %i:0x%04x", seg, addr);
	return mem_mget(seg, addr, dest, count);
}

// -----------------------------------------------------------------------
int ectl_mem_set(int seg, uint16_t addr, uint16_t *src, int count)
{
	LOG(L_ECTL, 2, "ECTL mem set: %i:0x%04x, %i words", seg, addr, count);
	int ret = mem_mput(seg, addr, src, count);
	return ret;
}

// -----------------------------------------------------------------------
int ectl_mem_map(int seg)
{
	int map = mem_get_map(seg);
	LOG(L_ECTL, 2, "ECTL mem map: %i = 0x%04x", seg, map);
	return map;
}

// -----------------------------------------------------------------------
int ectl_cpu_state_get()
{
	int state = cpu_state_get();
	LOG(L_ECTL, 2, "ECTL state get: 0x%04x", state);
	return state;
}

// -----------------------------------------------------------------------
const char * ectl_cpu_state_bit_name(int bitpos)
{
	static const char *state_names[] = { "STOP", "HALT", "CLM", "CLO", "QUIT", "???" };

	if (bitpos < ECTL_STATE_COUNT) {
		return state_names[bitpos];
	} else {
		return state_names[ECTL_STATE_COUNT];
	}
}

// -----------------------------------------------------------------------
void ectl_cpu_stop()
{
	LOG(L_ECTL, 2, "ECTL cpu STOP");
	cpu_trigger_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void ectl_cpu_start()
{
	LOG(L_ECTL, 2, "ECTL cpu START");
	cpu_clear_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void ectl_cpu_cycle()
{
	LOG(L_ECTL, 2, "ECTL cpu CYCLE");
	cpu_trigger_cycle();
}

// -----------------------------------------------------------------------
void ectl_cpu_quit()
{
	LOG(L_ECTL, 2, "ECTL cpu QUIT");
	cpu_trigger_state(STATE_QUIT);
}

// -----------------------------------------------------------------------
void ectl_clock_set(int state)
{
	LOG(L_ECTL, 2, "ECTL clock set: %i", state);
	if (state == ECTL_OFF) {
		timer_off();
	} else {
		timer_on();
	}
}

// -----------------------------------------------------------------------
int ectl_clock_get()
{
	int state = timer_get_state();
	LOG(L_ECTL, 2, "ECTL clock get: %i", state);
	if (state) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
void ectl_cpu_clear()
{
	LOG(L_ECTL, 2, "ECTL cpu CLEAR");
	cpu_trigger_state(STATE_CLO);
}

// -----------------------------------------------------------------------
void ectl_bootstrap(int chan, int unit)
{
	LOG(L_ECTL, 2, "ECTL bootstrap");
}

// -----------------------------------------------------------------------
void ectl_oprq()
{
	LOG(L_ECTL, 2, "ECTL OPRQ");
	int_set(INT_OPRQ);
}

// -----------------------------------------------------------------------
int ectl_int_set(unsigned interrupt)
{
	if (interrupt < 32) {
		LOG(L_ECTL, 2, "ECTL int set %i", interrupt);
		int_set(interrupt);
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
uint32_t ectl_int_get()
{
	uint32_t rz = RZ;
	LOG(L_ECTL, 2, "ECTL interrupts 0x%08x", rz);
	return rz;
}

// -----------------------------------------------------------------------
const char * ectl_version()
{
	static const char *ver = EM400_VERSION;
	LOG(L_ECTL, 2, "ECTL version: %s", ver);
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

	LOG(L_ECTL, 2, "ECTL capabilities: 0x%04x", capa);
	return capa;
}

// -----------------------------------------------------------------------
int ectl_load(FILE *f, const char *name, int seg, uint16_t saddr)
{
	LOG(L_ECTL, 2, "ECTL load: %i:0x%04x %s", seg, saddr, name);
	uint16_t *buf = malloc(sizeof(uint16_t) * 0x10000);

	int res = fread(buf, sizeof(uint16_t), 0x10000, f);
	if (res > 0) {
		endianswap(buf, res);
		res = mem_mput(seg, saddr, buf, res);
	}

	free(buf);
	return res;
}

// -----------------------------------------------------------------------
int ectl_log_state_get()
{
	int state = log_is_enabled();
	LOG(L_ECTL, 2, "ECTL log state get: %i", state);
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
	LOG(L_ECTL, 2, "ECTL log state set: %i", state);
	if (state == ECTL_OFF) {
		log_disable();
		res = 0;
	} else if (state == ECTL_ON) {
		res = log_enable();
	}
	return res;
}

// -----------------------------------------------------------------------
int ectl_log_level_get(unsigned component)
{
	int level = log_get_level(component);
	LOG(L_ECTL, 2, "ECTL log level get: %i = %i", component, level);
	return level;
}

// -----------------------------------------------------------------------
int ectl_log_level_set(unsigned component, unsigned level)
{
	LOG(L_ECTL, 2, "ECTL log level set: %i = %i", component, level);
	return log_set_level(component, level);
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

	LOG(L_ECTL, 2, "ECTL ips: %li", ips);
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
	int res = -1;
	LOG(L_ECTL, 2, "ECTL eval: %s", expression);
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
	return res;
}

// -----------------------------------------------------------------------
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, 2, "ECTL brk add: %s", expression);
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
	LOG(L_ECTL, 2, "ECTL brk del: %i", id);
	return ectl_brk_delete(id);
}

// -----------------------------------------------------------------------
void ectl_stoponhlt040_set(int state)
{
	LOG(L_ECTL, 2, "ECTL stop on HLT>=040: %i", state);
	if (state == ECTL_OFF) {
		atom_store_release(&stop_on_hlt040, 0);
	} else {
		atom_store_release(&stop_on_hlt040, 1);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
