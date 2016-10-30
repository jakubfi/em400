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
	return 0;
}

// -----------------------------------------------------------------------
void ectl_shutdown()
{
	ectl_brk_del_all();
}

// -----------------------------------------------------------------------
void ectl_regs_get(uint16_t *dregs)
{
	memcpy(dregs, regs, 8 * sizeof(uint16_t));
	dregs[ECTL_REG_IC] = rIC;
	dregs[ECTL_REG_SR] = rSR;
	dregs[ECTL_REG_IR] = rIR;
	dregs[ECTL_REG_KB] = rKB;
	dregs[ECTL_REG_MOD] = rMOD;
	dregs[ECTL_REG_MODc] = rMODc;
	dregs[ECTL_REG_ALARM] = rALARM;
}

// -----------------------------------------------------------------------
int ectl_reg_get(unsigned reg)
{
	if (reg >= ECTL_REG_COUNT) {
		return -1;
	}

	switch (reg) {
		case ECTL_REG_IC: return rIC;
		case ECTL_REG_SR: return rSR;
		case ECTL_REG_IR: return rIR;
		case ECTL_REG_KB: return rKB;
		case ECTL_REG_MOD: return rMOD;
		case ECTL_REG_MODc: return rMODc;
		case ECTL_REG_ALARM: return rALARM;
		default: return regs[reg];
	}
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
const char * ectl_reg_name(unsigned n)
{
	if (n < ECTL_REG_COUNT) {
		return ectl_reg_names[n];
	} else {
		return ectl_reg_names[ECTL_REG_COUNT];
	}
}

// -----------------------------------------------------------------------
int ectl_reg_set(unsigned reg, uint16_t val)
{
	if (reg >= ECTL_REG_COUNT) {
		return -1;
	}

	switch (reg) {
		case ECTL_REG_IC: rIC = val; break;
		case ECTL_REG_SR: rSR = val; break;
		case ECTL_REG_IR: rIR = val; break;
		case ECTL_REG_KB: rKB = val; break;
		case ECTL_REG_MOD: rMOD = val; break;
		case ECTL_REG_MODc: rMODc = val; break;
		case ECTL_REG_ALARM: rALARM = val; break;
		default: regs[reg] = val; break;
	}
	return 0;
}

// -----------------------------------------------------------------------
int ectl_mem_get(int nb, uint16_t addr, uint16_t *dest, int count)
{
	return mem_mget(nb, addr, dest, count);
}

// -----------------------------------------------------------------------
int ectl_mem_set(int nb, uint16_t addr, uint16_t *src, int count)
{
	int ret = mem_mput(nb, addr, src, count);
	return ret;
}

// -----------------------------------------------------------------------
int ectl_mem_map(int nb)
{
	return mem_get_map(nb);
}

// -----------------------------------------------------------------------
int ectl_cpu_state_get()
{
	return cpu_state_get();
}

// -----------------------------------------------------------------------
void ectl_cpu_stop()
{
	cpu_trigger_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void ectl_cpu_start()
{
	cpu_clear_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void ectl_cpu_cycle()
{

}

// -----------------------------------------------------------------------
void ectl_cpu_quit()
{
	cpu_trigger_state(STATE_QUIT);
}

// -----------------------------------------------------------------------
void ectl_clock_set(int state)
{
	if (state == ECTL_OFF) {
		timer_off();
	} else {
		timer_on();
	}
}

// -----------------------------------------------------------------------
int ectl_clock_get()
{
	if (timer_get_state()) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
void ectl_cpu_clear()
{
	cpu_trigger_state(STATE_CLO);
}

// -----------------------------------------------------------------------
void ectl_bootstrap(int chan, int unit)
{

}

// -----------------------------------------------------------------------
void ectl_oprq()
{
	int_set(INT_OPRQ);
}

// -----------------------------------------------------------------------
int ectl_int_set(unsigned interrupt)
{
	if (interrupt < 32) {
		int_set(interrupt);
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
uint32_t ectl_int_get()
{
	return RZ;
}

// -----------------------------------------------------------------------
const char * ectl_version()
{
	static const char *ver = EM400_VERSION;
	return ver;
}

// -----------------------------------------------------------------------
const char * ectl_capa_name(unsigned c)
{
	char *capa_names[] = { "MX16", "CRON", "AWP", "UIO", "MEGABOOT", "NOMEMSTOP", "???" };

	if (c < ECTL_CAPA_COUNT) {
		return capa_names[c];
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

	return capa;
}

// -----------------------------------------------------------------------
int ectl_load(FILE *f, int seg, uint16_t saddr)
{
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
	if (log_is_enabled()) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
int ectl_log_state_set(int state)
{
	int res = -1;
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
	return log_get_level(component);
}

// -----------------------------------------------------------------------
int ectl_log_level_set(unsigned component, unsigned level)
{
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

	return ips;
}

// -----------------------------------------------------------------------
static struct ectl_est * __ectl_parse(char *input, char **error_msg, int *err_beg, int *err_end)
{
	struct ectl_est *tree;

	YY_BUFFER_STATE yb = ectl_yy_scan_string(input);
	ectl_yyparse(&tree);
	ectl_yy_delete_buffer(yb);

	if (!tree) {
		*error_msg = strdup("Fatal error, parser did not return anything");
		return NULL;
	}

	if (tree->type == ECTL_AST_N_ERR) {
		*error_msg = strdup(tree->err);
		*err_beg = tree->c_beg;
		*err_end = tree->c_end;
		ectl_est_delete(tree);
		return NULL;
	}

	return tree;
}

// -----------------------------------------------------------------------
int ectl_eval(char *input, char **error_msg, int *err_beg, int *err_end)
{
	int res = -1;

	struct ectl_est *tree = __ectl_parse(input, error_msg, err_beg, err_end);
	if (!tree) {
		goto fin;
	}

	res = ectl_est_eval(tree);
	if (res < 0) {
		struct ectl_est *err_node = ectl_est_get_eval_err();
		if (err_node) {
			*error_msg = strdup(err_node->err);
			*err_beg = err_node->c_beg;
			*err_end = err_node->c_end;
		} else {
			*error_msg = strdup("Evaluation failed");
		}
		goto fin;
	}

fin:
	ectl_est_delete(tree);
	return res;
}

// -----------------------------------------------------------------------
int ectl_brk_add(char *input, char **error_msg, int *err_beg, int *err_end)
{
	struct ectl_est *tree = __ectl_parse(input, error_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	int id = ectl_brk_insert(tree, input);
	if (id < 0) {
		*error_msg = strdup("Cannot add new breakpoint");
		ectl_est_delete(tree);
		return -1;
	}

	return id;
}

// -----------------------------------------------------------------------
int ectl_brk_del(unsigned id)
{
	return ectl_brk_delete(id);
}

// -----------------------------------------------------------------------
void ectl_stoponhlt040_set(int state)
{
	if (state == ECTL_OFF) {
		atom_store_release(&stop_on_hlt040, 0);
	} else {
		atom_store_release(&stop_on_hlt040, 1);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
