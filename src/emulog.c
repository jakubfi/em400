//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include <inttypes.h>

#include "emulog.h"
#include "errors.h"
#include "emdas.h"
#include "atomic.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/decode.h"
#endif

// low-level stuff

#define EMULOG_FLUSH_DELAY_MS 200

struct emulog_component {
	char *name;
	unsigned thr;
};

struct emulog_component emulog_components[] = {
	{ "REG", 0 },
	{ "MEM", 0 },
	{ "CPU", 0 },
	{ "OP", 0 },
	{ "INT", 0 },
	{ "IO", 0 },
	{ "MX", 0 },
	{ "PX", 0 },
	{ "CCHR", 0 },
	{ "CMEM", 0 },
	{ "TERM", 0 },
	{ "WNCH", 0 },
	{ "FLOP", 0 },
	{ "PNCH", 0 },
	{ "PNRD", 0 },
	{ "CRK5", 0 },
	{ "EM4H", 0 },
	{ NULL, 0 }
};

int emulog_enabled;

static pthread_t emulog_flusher_th;
static pthread_mutex_t emulog_mutex;

static FILE *emulog_f;
static int emulog_quit;

static void * emulog_flusher(void *ptr);

// high-level stuff

#define EMULOG_F_COMP "%4s %1i | "
#define EMULOG_F_EMPTY "                     | "
#define EMULOG_F_CPU "%3s %2i:0x%04x %s | %s"

static uint16_t emulog_cycle_sr;
static uint16_t emulog_cycle_ic;

static int emulog_pname_offset;
static char emulog_pname[7] = "------";

#define EMULOG_INT_INDENT_MAX 4*8
static int emulog_int_level = EMULOG_INT_INDENT_MAX;
static const char *emulog_int_indent = "--> --> --> --> --> --> --> --> ";

static int emulog_exl_number = -1;
static int emulog_exl_nb;
static int emulog_exl_addr;
static int emulog_exl_r4;

static struct emdas *emd;
static char *dasm_buf;
uint16_t *mem_ptr(int nb, uint16_t addr);

static void emulog_log_timestamp(unsigned component, unsigned level, char *msg);

// -----------------------------------------------------------------------
int emulog_init(int enabled, char *filename, int level, int pname_offset, int cpu_mod)
{
	int ret = E_ALLOC;

	emulog_f = fopen(filename, "a");
	if (!emulog_f) {
		ret = E_FILE_OPEN;
		goto cleanup;
	}

	// set up thresholds
	for (int i=0 ; i<L_MAX; i++) {
		atom_store(&emulog_components[i].thr, level);
	}

	// set up flusher thread
	if (pthread_create(&emulog_flusher_th, NULL, emulog_flusher, NULL) != 0) {
		ret = E_THREAD;
		goto cleanup;
	}

	emulog_quit = 0;
	atom_store(&emulog_enabled, enabled);

	pthread_mutex_init(&emulog_mutex, NULL);

	// initialize deassembler
	emd = emdas_create(cpu_mod ? EMD_ISET_MX16 : EMD_ISET_MERA400, mem_ptr);
	if (!emd) {
		ret = E_DASM;
		goto cleanup;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 0, 0);
	dasm_buf = emdas_get_buf(emd);

	emulog_pname_offset = pname_offset;

	emulog_log_timestamp(L_EM4H, 0, "Opening log");

	return E_OK;

cleanup:
	if (emulog_f) fclose(emulog_f);
	emdas_destroy(emd);
	return ret;
}

// -----------------------------------------------------------------------
void emulog_shutdown()
{
	int i;

	emulog_log_timestamp(L_EM4H, 0, "Closing log");

	emdas_destroy(emd);

	for (i=0 ; i<L_MAX; i++) {
		atom_store(&emulog_components[i].thr, 0);
	}

	pthread_mutex_lock(&emulog_mutex);
	emulog_quit = 1;
	pthread_mutex_unlock(&emulog_mutex);

	pthread_join(emulog_flusher_th, NULL);

	fflush(emulog_f);
	fclose(emulog_f);
}

// -----------------------------------------------------------------------
static void * emulog_flusher(void *ptr)
{
	// flush log every EMULOG_FLUSH_DELAY_MS miliseconds
	while (1) {
		pthread_mutex_lock(&emulog_mutex);
		fflush(emulog_f);
		if (emulog_quit) {
			pthread_mutex_unlock(&emulog_mutex);
			break;
		}
		pthread_mutex_unlock(&emulog_mutex);
		usleep(EMULOG_FLUSH_DELAY_MS * 1000);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void emulog_disable()
{
	atom_store(&emulog_enabled, 0);
}

// -----------------------------------------------------------------------
void emulog_enable()
{
	atom_store(&emulog_enabled, 1);
}

// -----------------------------------------------------------------------
int emulog_is_enabled()
{
	return atom_load(&emulog_enabled);
}

// -----------------------------------------------------------------------
int emulog_set_level(int component, unsigned level)
{
	// set level for specified component
	if (component >= 0) {
		atom_store(&emulog_components[component].thr, level);
	// set level for all components
	} else {
		for (int i=0 ; i<L_MAX; i++) {
			atom_store(&emulog_components[i].thr, level);
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
int emulog_get_level(unsigned component)
{
	int level;

	level = atom_load(&emulog_components[component].thr);

	return level;
}

// -----------------------------------------------------------------------
char * emulog_get_component_name(unsigned component)
{
	return emulog_components[component].name;
}

// -----------------------------------------------------------------------
int emulog_get_component_id(char *name)
{
	int i;
	int comp = -1;

	for (i=0 ; (i<L_MAX) ; i++) {
		if (!strcasecmp(emulog_components[i].name, name)) {
			comp = i;
			break;
		}
	}
	return comp;
}

// -----------------------------------------------------------------------
int emulog_wants(unsigned component, unsigned level)
{
	// check if message is to be logged
	if (level > atom_load(&emulog_components[component].thr)) {
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
void emulog_log(unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	pthread_mutex_lock(&emulog_mutex);
	fprintf(emulog_f, EMULOG_F_COMP EMULOG_F_EMPTY, emulog_components[component].name, level);
	vfprintf(emulog_f, msgfmt, vl);
	fprintf(emulog_f, "\n");
	pthread_mutex_unlock(&emulog_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void emulog_log_cpu(unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	pthread_mutex_lock(&emulog_mutex);
	fprintf(emulog_f, EMULOG_F_COMP EMULOG_F_CPU,
		emulog_components[component].name, level,
		(emulog_cycle_sr & 0b0000000000100000) ? "USR" : "OS",
		(emulog_cycle_sr & 0b0000000000001111),
		emulog_cycle_ic,
		emulog_pname,
		emulog_int_indent + emulog_int_level
	);
	vfprintf(emulog_f, msgfmt, vl);
	fprintf(emulog_f, "\n");
	pthread_mutex_unlock(&emulog_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void emulog_splitlog(unsigned component, unsigned level, char *text)
{
	char *p;
	char *start = text;

	pthread_mutex_lock(&emulog_mutex);

	fprintf(emulog_f, EMULOG_F_COMP EMULOG_F_EMPTY " .---------------------------------------------------------\n", emulog_components[component].name, level);
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
		}
		fprintf(emulog_f, EMULOG_F_COMP EMULOG_F_EMPTY " | %s\n", emulog_components[component].name, level, start);
		if (p) {
			start = p+1;
		} else {
			start = NULL;
		}
	}
	fprintf(emulog_f, EMULOG_F_COMP EMULOG_F_EMPTY " `---------------------------------------------------------\n", emulog_components[component].name, level);

	pthread_mutex_unlock(&emulog_mutex);
}

// -----------------------------------------------------------------------
static void emulog_log_timestamp(unsigned component, unsigned level, char *msg)
{
	struct timeval ct;
	char date[32];
	gettimeofday(&ct, NULL);
	strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
	emulog_log(component, level, "%s: %s", msg, date);
}

// -----------------------------------------------------------------------
void emulog_store_cycle_state(uint16_t sr, uint16_t ic)
{
	emulog_cycle_sr = sr;
	emulog_cycle_ic = ic;
}

// -----------------------------------------------------------------------
void emulog_handle_syscall(unsigned component, unsigned level, int number, int nb, int addr, int r4)
{
	emulog_exl_number = number;
	emulog_exl_nb = nb;
	emulog_exl_addr = addr;
	emulog_exl_r4 = r4;

	char *details;
#ifdef WITH_DEBUGGER
	details = decode_exl(nb, r4, number);
#else
	details = malloc(128);
	sprintf(details, "[details missing]");
#endif
	emulog_splitlog(component, level, details);
	free(details);
}

// -----------------------------------------------------------------------
void emulog_handle_sp(unsigned component, unsigned level, uint16_t n)
{
	char *ctx;

	emulog_log_cpu(component, level, "SP: context @ 0x%04x -> IC: 0x%04x", n, emulog_cycle_ic);

#ifdef WITH_DEBUGGER
	ctx = decode_ctx(0, n, 0);
#else
	ctx = malloc(128);
	sprintf(ctx, "[details missing]");
#endif
	emulog_splitlog(component, level, ctx);
	free(ctx);
}

// -----------------------------------------------------------------------
void emulog_handle_syscall_ret(unsigned component, unsigned level, uint16_t n)
{
	if ((emulog_exl_number >= 0) && (emulog_cycle_ic == emulog_exl_addr) && ((emulog_cycle_sr & 0x1111) == emulog_exl_nb)) {
		char *details;
#ifdef WITH_DEBUGGER
		details = decode_exl(emulog_exl_nb, emulog_exl_r4, -emulog_exl_number);
#else
		details = malloc(128);
		sprintf(details, "[details missing]");
#endif
		emulog_splitlog(component, level, details);
		free(details);
		emulog_syscall_reset();
	}
}

// -----------------------------------------------------------------------
void emulog_syscall_reset()
{
	emulog_exl_number = -1; // indicate that there is no syscall in progress
}

// -----------------------------------------------------------------------
void emulog_intlevel_reset()
{
	emulog_int_level = EMULOG_INT_INDENT_MAX;
}

// -----------------------------------------------------------------------
void emulog_intlevel_dec()
{
	if (emulog_int_level < EMULOG_INT_INDENT_MAX) {
		emulog_int_level += 4;
	}
}

// -----------------------------------------------------------------------
void emulog_intlevel_inc()
{
	if (emulog_int_level > 0) {
		emulog_int_level -= 4;
	}
}

// -----------------------------------------------------------------------
void emulog_log_dasm(unsigned component, unsigned level, int mod, int norm_arg, int short_arg, int16_t n)
{
	int nb = ((emulog_cycle_sr & 0b0000000000100000) >> 5) * (emulog_cycle_sr & 0b0000000000001111);
	emdas_dasm(emd, nb, emulog_cycle_ic);

	if (norm_arg) {
		if (mod) {
			emulog_log_cpu(component, level, "    %-20s N = 0x%x = %i, MOD = 0x%x = %i", dasm_buf, (uint16_t) n, n, mod, mod);
		} else {
			emulog_log_cpu(component, level, "    %-20s N = 0x%x = %i", dasm_buf, (uint16_t) n, n);
		}
	} else if (short_arg) {
		if (mod) {
			emulog_log_cpu(component, level, "    %-20s T = %i, MOD = 0x%x = %i", dasm_buf, n, mod, mod);
		} else {
			emulog_log_cpu(component, level, "    %-20s T = %i", dasm_buf, n);
		}
	} else {
		emulog_log_cpu(component, level, "    %-20s", dasm_buf);
	}
}

// -----------------------------------------------------------------------
void emulog_update_pname()
{
	uint16_t *bprog;
	uint16_t *pname[2];
	char *n1, *n2;

	if (!emulog_pname_offset) return;

	bprog = mem_ptr(0, 0x62);
	if (!bprog) return;

	pname[0] = mem_ptr(0, *bprog + emulog_pname_offset + 0);
	pname[1] = mem_ptr(0, *bprog + emulog_pname_offset + 1);
	if (!pname[0] || !pname[1]) return;

	n1 = int2r40(*(pname[0]));
	n2 = int2r40(*(pname[1]));
	snprintf(emulog_pname, 7, "%s%s", n1, n2);
	free(n1);
	free(n2);
}


// vim: tabstop=4 shiftwidth=4 autoindent
