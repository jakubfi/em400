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

#define _XOPEN_SOURCE 500
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdatomic.h>

#include <emdas.h>

#include "mem/mem.h"
#include "log.h"
#include "log_crk.h"
#include "utils/utils.h"


// low-level stuff

static const char * log_component_names[] = {
	"ALL", "EM4H", "FDBR", "CRK5",
	"MEM", "CPU", "OP", "INT", "IO",
	"MX", "CCHR", "CMEM",
	"UZDAT", "UZFX",
	"TERM", "9425", "WNCH", "FLOP", "PNCH", "PNRD","TAPE",
};

atomic_uint log_components_enabled = 0; // components currently enabled or 0 if logging is disabled
static atomic_uint log_components_selected = 1 << L_EM4H; // components selected by user, EM4H always selected
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *log_file_name;
static FILE *log_file;
static bool log_buf_type;

// high-level stuff

#define LOG_F_COMP "%5s | %8s | "
#define LOG_F_FUN "%24s() | "
#define LOG_F_CPU "%x:0x%04x %-6s            | %s"

static uint16_t log_cycle_sr;
static uint16_t log_cycle_ic;

#define LOG_INT_INDENT_MAX (4*8)
static int log_int_level = LOG_INT_INDENT_MAX;
static const char *log_int_indent = "--> --> --> --> --> --> --> --> ";

static struct emdas *emd;
static char *dasm_buf;

static void log_log_timestamp(unsigned component, const char *msg, const char *func);

// -----------------------------------------------------------------------
int log_init(const char *cfg_log_file_name, em400_log_buf_type_t cfg_log_buf_type)
{
	log_buf_type = cfg_log_buf_type;
	log_file_name = strdup(cfg_log_file_name);
	if (!log_file_name) {
		LOGERR("Memory allocation error.");
		goto cleanup;
	}

	// initialize deassembler
	emd = emdas_create(EMD_ISET_MX16, (emdas_getfun) mem_read_1);
	if (!emd) {
		LOGERR("Log deassembler initialization failed.");
		goto cleanup;
	}
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 0, 0);
	dasm_buf = emdas_get_buf(emd);

	pthread_mutex_init(&log_mutex, NULL);

	return E_OK;

cleanup:
	log_shutdown();
	return E_ERR;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	log_disable();
	emdas_destroy(emd);
	emd = NULL;
	log_crk_shutdown();
	free(log_file_name);
	log_file_name = NULL;
}

// -----------------------------------------------------------------------
void log_disable()
{
	if (!log_is_enabled()) return;

	log_log_timestamp(L_EM4H, "EM400 version " EM400_VERSION " closing log file", __func__);
	atomic_store_explicit(&log_components_enabled, 0, memory_order_relaxed);
	if (log_file) {
		fclose(log_file);
		log_file = NULL;
	}
}

// -----------------------------------------------------------------------
int log_enable()
{
	if (log_is_enabled()) return E_OK;

	// Open log file
	log_file = fopen(log_file_name, "a");
	if (!log_file) {
		return LOGERR("Failed to open log file \"%s\" for appending", log_file);
	}
	if (log_buf_type == EM400_LOG_LINE_BUFFERED) {
		if (setvbuf(log_file, NULL, _IOLBF, BUFSIZ)) {
			return LOGERR("cannot set line buffering for log file \"%s\"", log_file);
		}
	}

	atomic_store_explicit(&log_components_enabled, log_components_selected, memory_order_relaxed);

	log_log_timestamp(L_EM4H, "EM400 version " EM400_VERSION " opened log file", __func__);

	return E_OK;
}

// -----------------------------------------------------------------------
unsigned log_is_enabled()
{
	return atomic_load_explicit(&log_components_enabled, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void log_component_enable(unsigned component)
{
	if (component == L_ALL) {
		atomic_store_explicit(&log_components_selected, -1, memory_order_relaxed);
	} else {
		atomic_fetch_or_explicit(&log_components_selected, 1 << component, memory_order_relaxed);
	}
	if (log_is_enabled()) {
		atomic_store_explicit(&log_components_enabled, log_components_selected, memory_order_relaxed);
	}
}

// -----------------------------------------------------------------------
void log_component_disable(unsigned component)
{
	if (component == L_ALL) {
		atomic_store_explicit(&log_components_selected, (1 << L_EM4H), memory_order_relaxed);
	} else {
		atomic_fetch_and_explicit(&log_components_selected, ~(1 << component) | (1 << L_EM4H), memory_order_relaxed);
	}
	if (log_is_enabled()) {
		atomic_store_explicit(&log_components_enabled, log_components_selected, memory_order_relaxed);
	}
}

// -----------------------------------------------------------------------
unsigned log_component_get(unsigned component)
{
	return atomic_load_explicit(&log_components_selected, memory_order_relaxed) & (1 << component);
}

// -----------------------------------------------------------------------
const char * log_get_component_name(unsigned component)
{
	if (component > L_COUNT) {
		return NULL;
	}
	return log_component_names[component];
}

// -----------------------------------------------------------------------
int log_get_component_id(const char *name)
{
	int comp = -1;

	for (unsigned i=0 ; i<L_COUNT ; i++) {
		if (name && !strcasecmp(log_component_names[i], name)) {
			comp = i;
			break;
		}
	}

	return comp;
}

// -----------------------------------------------------------------------
int log_setup_components(const char *components)
{
	if (!components || !*components) return E_OK;

	char *cmp = strdup(components);
	if (!cmp) {
		return LOGERR("Memory allocation error");
	}

	const char *delims = ", ";
	char *token = strtok(cmp, delims);

	while (token != NULL) {
		bool neg = false;

		if (*token == '-') {
			neg = true;
			token++;
		}

		if (*token != '\0') {
			int id = log_get_component_id(token);
			if (id < 0) {
				return LOGERR("Unknown log component: %s", token);
			}
			if (neg) log_component_disable(id);
			else log_component_enable(id);
		}

		token = strtok(NULL, delims);
	}

	free(cmp);
	return E_OK;
}

// -----------------------------------------------------------------------
int log_err(const char *func, const char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	char thname[16];
	pthread_getname_np(pthread_self(), thname, 16);

	vfprintf(stderr, msgfmt, vl);
	fprintf(stderr, "\n");

	va_end(vl);

	if (log_is_enabled()) {
		va_start(vl, msgfmt);
		pthread_mutex_lock(&log_mutex);
		fprintf(log_file, LOG_F_COMP LOG_F_FUN, log_component_names[L_EM4H], thname, func);
		fprintf(log_file, "ERROR: ");
		vfprintf(log_file, msgfmt, vl);
		fprintf(log_file, "\n");
		pthread_mutex_unlock(&log_mutex);

		va_end(vl);
	}

	return E_ERR;
}

// -----------------------------------------------------------------------
void log_log(unsigned component, const char *func, const char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	char thname[16];
	pthread_getname_np(pthread_self(), thname, 16);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_file, LOG_F_COMP LOG_F_FUN, log_component_names[component], thname, func);
	vfprintf(log_file, msgfmt, vl);
	fprintf(log_file, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void log_cpu(unsigned component, const char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	char thname[16];
	pthread_getname_np(pthread_self(), thname, 16);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_file, LOG_F_COMP LOG_F_CPU,
		log_component_names[component],
		thname,
		(log_cycle_sr & 0b0000000000100000) ? (log_cycle_sr & 0b0000000000001111) : 0,
		log_cycle_ic,
		log_get_current_process(),
		log_int_indent + log_int_level
	);
	vfprintf(log_file, msgfmt, vl);
	fprintf(log_file, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void log_splitlog(unsigned component, const char *func, const char *text)
{
	if (!LOG_WANTS(component)) return;

	char *p;
	const char *start = text;
    char thname[16];

    pthread_getname_np(pthread_self(), thname, 16);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_file,
		LOG_F_COMP LOG_F_FUN ".-------------------------------------------------------------------\n",
		log_component_names[component],
		thname,
		func
	);
	while (start && *start) {
		p = (char *) strchr(start, '\n');
		if (p) {
			*p = '\0';
			fprintf(log_file,
				LOG_F_COMP LOG_F_FUN "| %s\n",
				log_component_names[component],
				thname,
				func,
				start
			);
			start = p+1;
		} else {
			start = NULL;
		}
	}
	fprintf(log_file,
		LOG_F_COMP LOG_F_FUN "`-------------------------------------------------------------------\n",
		log_component_names[component],
		thname,
		func
	);
	pthread_mutex_unlock(&log_mutex);
}

// -----------------------------------------------------------------------
static void log_log_timestamp(unsigned component, const char *msg, const char *func)
{
	struct timeval ct;
	char date[32];
	gettimeofday(&ct, NULL);
	strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
	log_log(component, func, "%s %s", date, msg);
}

// -----------------------------------------------------------------------
void log_store_cycle_state(uint16_t sr, uint16_t ic)
{
	log_cycle_sr = sr;
	log_cycle_ic = ic;
}

// -----------------------------------------------------------------------
void log_intlevel_reset()
{
	log_int_level = LOG_INT_INDENT_MAX;
}

// -----------------------------------------------------------------------
void log_intlevel_dec()
{
	if (log_int_level < LOG_INT_INDENT_MAX) {
		log_int_level += 4;
	}
}

// -----------------------------------------------------------------------
void log_intlevel_inc()
{
	if (log_int_level > 0) {
		log_int_level -= 4;
	}
}

// -----------------------------------------------------------------------
void log_dasm(int arg, int16_t ac, const char *comment)
{
	const int nb = (log_cycle_sr & 0b0000000000100000) ? (log_cycle_sr & 0b0000000000001111) : 0;
	emdas_dasm(emd, nb, log_cycle_ic);

	if (arg) {
		log_cpu(L_CPU, "    %s%-20s AC = 0x%04x = %i", comment, dasm_buf, (uint16_t) ac, ac);
	} else {
		log_cpu(L_CPU, "    %s%-20s", comment, dasm_buf);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
