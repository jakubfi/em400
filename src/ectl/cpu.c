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
#include <stdatomic.h>

#include "log.h"
#include "utils/utils.h"
#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "io/defs.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl/brk.h"
#include "ectl_parser.h"
#include "libem400.h"

const char *state_names[] = {
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
	LOG(L_ECTL, "ECTL state get: %s", state_names[state]);
	return state;
}

// -----------------------------------------------------------------------
void ectl_cpu_off()
{
	LOG(L_ECTL, "ECTL cpu OFF");
	cp_off();
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
bool ectl_log_state_get()
{
	int state = log_is_enabled();
	LOG(L_ECTL, "ECTL log state get: %i", state);
	return state;
}

// -----------------------------------------------------------------------
int ectl_log_state_set(bool state)
{
	int res = -1;
	LOG(L_ECTL, "ECTL log state set: %i", state);
	if (state) {
		res = log_enable();
	} else {
		log_disable();
		res = 0;
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
int ectl_log_component_set(unsigned component, bool state)
{
	LOG(L_ECTL, "ECTL log component %s: %i", ectl_log_component_name(component), state);
	if (state) {
		log_component_enable(component);
	} else {
		log_component_disable(component);
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


// vim: tabstop=4 shiftwidth=4 autoindent
