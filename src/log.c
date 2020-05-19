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

#include <emdas.h>

#include "mem/mem.h"
#include "log.h"
#include "log_crk.h"
#include "atomic.h"
#include "utils/utils.h"
#include "cfg.h"

// low-level stuff

#define LOG_FLUSH_DELAY_MS 200

static const char * log_component_names[] = {
	"REG",
	"MEM",
	"CPU",
	"OP",
	"INT",
	"IO",
	"MX",
	"PX",
	"CCHR",
	"CMEM",
	"TERM",
	"9425",
	"WNCH",
	"FLOP",
	"PNCH",
	"PNRD",
	"TAPE",
	"CRK5",
	"EM4H",
	"ECTL",
	"FPGA",
	"ALL",
};

unsigned log_components_enabled;
unsigned log_components_selected;

static pthread_t log_flusher_th;
static pthread_mutex_t log_mutex;
static int log_flusher_stop;

static char *log_file;
static FILE *log_f;

static void * log_flusher(void *ptr);

static int line_buffered;

// high-level stuff

#define LOG_F_COMP "%4s | %8s | "
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
static void log_components_update();

// -----------------------------------------------------------------------
int log_init(em400_cfg *cfg)
{
	int ret = E_ERR;

	const char *cfg_logfile = cfg_getstr(cfg, "log:file", CFG_DEFAULT_LOG_FILE);
	log_file = strdup(cfg_logfile);
	if (!log_file) {
		LOGERR("Memory allocation error.");
		goto cleanup;
	}

	// set up components to log
	const char *log_components = cfg_getstr(cfg, "log:components", CFG_DEFAULT_LOG_COMPONENTS);
	if (log_setup_components(log_components) < 0) {
		LOGERR("Failed to parse log component definition: \"%s\".", log_components);
		goto cleanup;
	}

	// initialize deassembler
	int cpu_mod = cfg_getbool(cfg, "cpu:modifications", CFG_DEFAULT_CPU_MODIFICATIONS);
	emd = emdas_create(cpu_mod ? EMD_ISET_MX16 : EMD_ISET_MERA400, (emdas_getfun) mem_get);
	if (!emd) {
		LOGERR("Log deassembler initialization failed.");
		goto cleanup;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 0, 0);
	dasm_buf = emdas_get_buf(emd);

	pthread_mutex_init(&log_mutex, NULL);

	line_buffered = cfg_getbool(cfg, "log:line_buffered", CFG_DEFAULT_LOG_LINE_BUFFERED);

	int log_enabled = cfg_getbool(cfg, "log:enabled", CFG_DEFAULT_LOG_ENABLED);
	if (log_enabled) {
		ret = log_enable();
		if (ret != E_OK) {
			LOGERR("Failed to enable logging.");
			goto cleanup;
		} else {
			LOG(L_EM4H, "Logging enabled. File: %s, components: %s, line buffering: %s", cfg_logfile, log_components, line_buffered ? "true" : "false");
		}
	}

	return E_OK;

cleanup:
	log_shutdown();
	return ret;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	log_disable();
	emdas_destroy(emd);
	log_crk_shutdown();
	free(log_file);
	log_file = NULL;
}

// -----------------------------------------------------------------------
static void * log_flusher(void *ptr)
{
	// flush log every LOG_FLUSH_DELAY_MS miliseconds so user
	// doesn't wait indefinitely for log output when stepping the CPU
	while (1) {
		pthread_mutex_lock(&log_mutex);
		fflush(log_f);
		if (log_flusher_stop) {
			pthread_mutex_unlock(&log_mutex);
			break;
		}
		pthread_mutex_unlock(&log_mutex);
		usleep(LOG_FLUSH_DELAY_MS * 1000);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void log_disable()
{
	if (!log_is_enabled()) {
		return;
	}

	log_log_timestamp(L_EM4H, "EM400 version " EM400_VERSION " closing log file", __func__);

	atom_store_release(&log_components_enabled, 0);

	// stop flusher thread
	pthread_mutex_lock(&log_mutex);
	log_flusher_stop = 1;
	pthread_mutex_unlock(&log_mutex);
	if (!line_buffered) {
		pthread_join(log_flusher_th, NULL);
	}

	if (log_f) {
		fclose(log_f);
	}
}

// -----------------------------------------------------------------------
int log_enable()
{
	if (log_is_enabled()) {
		return E_OK;
	}

	// Open log file
	log_f = fopen(log_file, "a");
	if (!log_f) {
		return LOGERR("Failed to open log file: \"%s\".", log_file);
	}

	log_log_timestamp(L_EM4H, "EM400 version " EM400_VERSION " opened log file", __func__);

	if (line_buffered) {
		setlinebuf(log_f);
		fflush(log_f);
	} else {
		// start up flusher thread only when in fully buffered mode
		log_flusher_stop = 0;
		if (pthread_create(&log_flusher_th, NULL, log_flusher, NULL) != 0) {
			fclose(log_f);
			return LOGERR("Failed to spawn log flusher thread.");
		}
		pthread_setname_np(log_flusher_th, "lflush");
	}

	log_components_update();

	return E_OK;
}

// -----------------------------------------------------------------------
unsigned log_is_enabled()
{
	return atom_load_acquire(&log_components_enabled);
}

// -----------------------------------------------------------------------
static void log_components_update()
{
	atom_store_release(&log_components_enabled, log_components_selected);
}

// -----------------------------------------------------------------------
void log_component_enable(unsigned component)
{
	if (component == L_ALL) {
		atom_store_release(&log_components_selected, -1);
	} else {
		atom_or_release(&log_components_selected, 1 << component);
	}
	if (log_is_enabled()) log_components_update();
}

// -----------------------------------------------------------------------
void log_component_disable(unsigned component)
{
	if (component == L_ALL) {
		atom_store_release(&log_components_selected, 0);
	} else {
		atom_and_release(&log_components_selected, ~(1 << component));
	}
	if (log_is_enabled()) log_components_update();
}

// -----------------------------------------------------------------------
unsigned log_component_get(unsigned component)
{
	return atom_load_acquire(&log_components_selected) & (1 << component);
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

	for (unsigned i=0 ; i<=L_ALL ; i++) {
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
	char *cmp = strdup(components);
	char *c = strtok(cmp, ",");
	while (c && *c) {
		while (*c == ' ') c++;
		if (*c) {
			char *space = strchr(c, ' ');
			if (space) *space = '\0';
			unsigned id = log_get_component_id(c);
			log_component_enable(id);
			c = strtok(NULL, ",");
		}
	}
	free(cmp);
	return 0;
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
		fprintf(log_f, LOG_F_COMP LOG_F_FUN, log_component_names[L_EM4H], thname, func);
		fprintf(log_f, "ERROR: ");
		vfprintf(log_f, msgfmt, vl);
		fprintf(log_f, "\n");
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
	fprintf(log_f, LOG_F_COMP LOG_F_FUN, log_component_names[component], thname, func);
	vfprintf(log_f, msgfmt, vl);
	fprintf(log_f, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void log_log_cpu(unsigned component, const char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	char thname[16];
	pthread_getname_np(pthread_self(), thname, 16);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_f, LOG_F_COMP LOG_F_CPU,
		log_component_names[component],
		thname,
		(log_cycle_sr & 0b0000000000100000) ? (log_cycle_sr & 0b0000000000001111) : 0,
		log_cycle_ic,
		log_get_current_process(),
		log_int_indent + log_int_level
	);
	vfprintf(log_f, msgfmt, vl);
	fprintf(log_f, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void log_splitlog(unsigned component, const char *func, const char *text)
{
	char *p;
	const char *start = text;

    char thname[16];
    pthread_getname_np(pthread_self(), thname, 16);

	pthread_mutex_lock(&log_mutex);

	fprintf(log_f, LOG_F_COMP LOG_F_FUN ".-------------------------------------------------------------------\n", log_component_names[component], thname, func);
	while (start && *start) {
		p = (char *) strchr(start, '\n');
		if (p) {
			*p = '\0';
		}
		fprintf(log_f, LOG_F_COMP LOG_F_FUN "| %s\n", log_component_names[component], thname, func, start);
		if (p) {
			start = p+1;
		} else {
			start = NULL;
		}
	}
	fprintf(log_f, LOG_F_COMP LOG_F_FUN "`-------------------------------------------------------------------\n", log_component_names[component], thname, func);

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
void log_log_dasm(int arg, int16_t n, const char *comment)
{
	int nb = (log_cycle_sr & 0b0000000000100000) ? (log_cycle_sr & 0b0000000000001111) : 0;
	emdas_dasm(emd, nb, log_cycle_ic);

	if (arg) {
		log_log_cpu(L_CPU, "    %s%-20s N = 0x%04x = %i", comment, dasm_buf, (uint16_t) n, n);
	} else {
		log_log_cpu(L_CPU, "    %s%-20s", comment, dasm_buf);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
