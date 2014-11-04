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

// low-level stuff

#define EMULOG_MAX_LEN (1024 * 4)
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

pthread_t emulog_flusher_th;
pthread_mutex_t emulog_mutex;
pthread_cond_t emulog_cond;

static FILE *emulog_f;
static int emulog_paused;
static int emulog_quit;

static void * emulog_flusher(void *ptr);
void emulog_log_timestamp(unsigned component, unsigned level, char *msg);

// high-level stuff

static uint16_t emulog_cycle_ic;
static char emulog_pname[7] = "------";

#define EMULOG_INT_INDENT_MAX 4*8
static int emulog_int_level = EMULOG_INT_INDENT_MAX;
static const char *emulog_int_indent = "--> --> --> --> --> --> --> --> ";

static int emulog_exl_number = -1;
static int emulog_exl_nb;
static int emulog_exl_addr;
static int emulog_exl_r4;

struct emdas *emd;
static char *dasm_buf;
uint16_t *mem_ptr(int nb, uint16_t addr);

// -----------------------------------------------------------------------
int emulog_init(int paused, char *filename, int level, int cpu_mod)
{
	int ret = E_ALLOC;

	emulog_f = fopen(filename, "a");
	if (!emulog_f) {
		ret = E_FILE_OPEN;
		goto cleanup;
	}

	// set up thresholds
	for (int i=0 ; i<L_MAX; i++) {
		emulog_components[i].thr = level;
	}

	// set up flusher thread
	if (pthread_create(&emulog_flusher_th, NULL, emulog_flusher, NULL) != 0) {
		ret = E_THREAD;
		goto cleanup;
	}

	emulog_quit = 0;
	emulog_enabled = 1;
	emulog_paused = paused;

	pthread_mutex_init(&emulog_mutex, NULL);
	pthread_cond_init(&emulog_cond, NULL);

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

	pthread_mutex_lock(&emulog_mutex);

	for (i=0 ; i<L_MAX; i++) {
		emulog_components[i].thr = 0;
	}
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
void emulog_pause()
{
	atom_store(&emulog_paused, 1);
}

// -----------------------------------------------------------------------
void emulog_rec()
{
	atom_store(&emulog_paused, 0);
}

// -----------------------------------------------------------------------
int emulog_is_paused()
{
	return atom_load(&emulog_paused);
}

// -----------------------------------------------------------------------
int emulog_set_level(int component, unsigned level)
{
	pthread_mutex_lock(&emulog_mutex);

	// set level for specified component
	if (component >= 0) {
		emulog_components[component].thr = level;
	// set level for all components
	} else {
		for (int i=0 ; i<L_MAX; i++) {
			emulog_components[i].thr = level;
		}
	}

	pthread_mutex_unlock(&emulog_mutex);

	return 0;
}

// -----------------------------------------------------------------------
int emulog_get_level(unsigned component)
{
	int level;

	pthread_mutex_lock(&emulog_mutex);
	level = emulog_components[component].thr;
	pthread_mutex_unlock(&emulog_mutex);

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
void emulog_log(unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	pthread_mutex_lock(&emulog_mutex);
	fprintf(emulog_f, "%4s %1i | ", emulog_components[component].name, level);
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

	emulog_log(component, level, EMULOG_FORMAT_SIMPLE "%s", " .---------------------------------------------------------");
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
			emulog_log(component, level, EMULOG_FORMAT_SIMPLE " | %s", start);
			start = p+1;
		} else {
			emulog_log(component, level, EMULOG_FORMAT_SIMPLE " | %s", start);
			start = NULL;
		}
	}
	emulog_log(component, level, EMULOG_FORMAT_SIMPLE "%s", " `---------------------------------------------------------");
}

// -----------------------------------------------------------------------
int emulog_wants(unsigned component, unsigned level)
{
	if (atom_load(&emulog_paused)) return 0;

	// check if message is to be logged
	pthread_mutex_lock(&emulog_mutex);
	if (level > emulog_components[component].thr) {
		pthread_mutex_unlock(&emulog_mutex);
		return 0;
	}
	pthread_mutex_unlock(&emulog_mutex);
	return 1;
}

// -----------------------------------------------------------------------
void emulog_log_timestamp(unsigned component, unsigned level, char *msg)
{
	struct timeval ct;
	char date[32];
	gettimeofday(&ct, NULL);
	strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
	emulog_log(component, level, EMULOG_FORMAT_SIMPLE "%s: %s", msg, date);
}

// -----------------------------------------------------------------------
void emulog_set_cycle_ic(uint16_t ic)
{
	emulog_cycle_ic = ic;
}

// -----------------------------------------------------------------------
uint16_t emulog_get_cycle_ic()
{
	return emulog_cycle_ic;
}

// -----------------------------------------------------------------------
void emulog_update_pname(uint16_t *r40pname)
{
	char *n1 = int2r40(r40pname[0]);
	char *n2 = int2r40(r40pname[1]);
	snprintf(emulog_pname, 7, "%s%s", n1, n2);
	free(n1);
	free(n2);
}

// -----------------------------------------------------------------------
char * emulog_get_pname()
{
	return emulog_pname;
}

// -----------------------------------------------------------------------
void emulog_exl_store(int number, int nb, int addr, int r4)
{
	emulog_exl_number = number;
	emulog_exl_nb = nb;
	emulog_exl_addr = addr;
	emulog_exl_r4 = r4;
}

// -----------------------------------------------------------------------
void emulog_exl_fetch(int *number, int *nb, int *addr, int *r4)
{
	*number = emulog_exl_number;
	*nb = emulog_exl_nb;
	*addr = emulog_exl_addr;
	*r4 = emulog_exl_r4;
}

// -----------------------------------------------------------------------
void emulog_exl_reset()
{
	emulog_exl_number = -1;
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
const char *emulog_intlevel_get_indent()
{
	return emulog_int_indent + emulog_int_level;
}

// -----------------------------------------------------------------------
void emulog_dasm(int nb)
{
	emdas_dasm(emd, nb, emulog_cycle_ic);
}

// -----------------------------------------------------------------------
char * emulog_get_dasm()
{
	return dasm_buf;
}


// vim: tabstop=4 shiftwidth=4 autoindent
