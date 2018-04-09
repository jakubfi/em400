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
#include "cfg.h"
#include "atomic.h"
#include "utils/utils.h"

// low-level stuff

#define LOG_FLUSH_DELAY_MS 200

struct log_component {
	const char *name;
	unsigned thr;
};

struct log_component log_components[] = {
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
	{ "9425", 0 },
	{ "WNCH", 0 },
	{ "FLOP", 0 },
	{ "PNCH", 0 },
	{ "PNRD", 0 },
	{ "TAPE", 0 },
	{ "CRK5", 0 },
	{ "EM4H", 1 },
	{ "ECTL", 1 },
	{ "FPGA", 0 },
	{ "ALL", 0 }
};

int log_enabled;

static pthread_t log_flusher_th;
static pthread_mutex_t log_mutex;
static int log_flusher_stop;

static char *log_file;
static FILE *log_f;

static void * log_flusher(void *ptr);

// high-level stuff

#define LOG_F_COMP "%4s %1i | "
#define LOG_F_EMPTY "                     | "
#define LOG_F_CPU "%-3s %2i:0x%04x %-6s | %s"

static uint16_t log_cycle_sr;
static uint16_t log_cycle_ic;

#define LOG_INT_INDENT_MAX (4*8)
static int log_int_level = LOG_INT_INDENT_MAX;
static const char *log_int_indent = "--> --> --> --> --> --> --> --> ";

static struct emdas *emd;
static char *dasm_buf;

static void log_log_timestamp(unsigned component, unsigned level, char *msg);

// -----------------------------------------------------------------------
int log_init(struct cfg_em400 *cfg)
{
	int res;
	int ret = E_ERR;

	log_file = strdup(cfg->log_file);
	if (!log_file) {
		log_err("Memory allocation error.");
		goto cleanup;
	}

	// set up thresholds
	res = log_setup_levels(cfg->log_levels);
	if (res < 0) {
		log_err("Failed to parse log levels definition: \"%s\".", cfg->log_levels);
		goto cleanup;
	}

	// initialize deassembler
	emd = emdas_create(cfg->cpu_mod ? EMD_ISET_MX16 : EMD_ISET_MERA400, (emdas_getfun) mem_get);
	if (!emd) {
		log_err("Log deassembler initialization failed.");
		goto cleanup;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 0, 0);
	dasm_buf = emdas_get_buf(emd);

	pthread_mutex_init(&log_mutex, NULL);

	if (cfg->log_enabled) {
		ret = log_enable();
		if (ret != E_OK) {
			log_err("Failed to enable logging.");
			goto cleanup;
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
	// flush log every LOG_FLUSH_DELAY_MS miliseconds
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

	log_log_timestamp(L_EM4H, 0, "Closing log");

	atom_store_release(&log_enabled, 0);

	// stop flusher thread
	pthread_mutex_lock(&log_mutex);
	log_flusher_stop = 1;
	pthread_mutex_unlock(&log_mutex);
	pthread_join(log_flusher_th, NULL);

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
		return log_err("Failed to open log file: \"%s\".", log_file);
	}

	log_log_timestamp(L_EM4H, 0, "Opened log");

	// start up flusher thread
	log_flusher_stop = 0;
	if (pthread_create(&log_flusher_th, NULL, log_flusher, NULL) != 0) {
		fclose(log_f);
		return log_err("Failed to spawn log flusher thread.");
	}

	atom_store_release(&log_enabled, 1);

	return E_OK;
}

// -----------------------------------------------------------------------
int log_is_enabled()
{
	return atom_load_acquire(&log_enabled);
}

// -----------------------------------------------------------------------
int log_set_level(unsigned component, unsigned level)
{
	if ((component > L_ALL) || (level > LOG_LEVEL_MAX)) {
		return -1;
	}

	// set level for specified component
	if (component != L_ALL) {
		atom_store_release(&log_components[component].thr, level);
	// set level for all components
	} else {
		for (int i=0 ; i<L_ALL; i++) {
			atom_store_release(&log_components[i].thr, level);
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
int log_get_level(unsigned component)
{
	if (component > L_ALL) {
		return -1;
	}
	return atom_load_acquire(&log_components[component].thr);
}

// -----------------------------------------------------------------------
const char * log_get_component_name(unsigned component)
{
	if (component > L_ALL) {
		return NULL;
	}
	return log_components[component].name;
}

// -----------------------------------------------------------------------
int log_get_component_id(const char *name)
{
	int i;
	int comp = -1;

	for (i=0 ; (i<=L_ALL) ; i++) {
		if (!strcasecmp(log_components[i].name, name)) {
			comp = i;
			break;
		}
	}
	return comp;
}

// -----------------------------------------------------------------------
// I was bored.
// And then I suddenly felt like moving some chars around, hardcore style.
int log_setup_levels(char *levels)
{
	if (!levels || (*levels == '\0')) return 0;

	char *str = levels;
	const int buf_max = 8;
	char level_str[buf_max];
	char comp_name[buf_max];
	char *buf = comp_name;
	int i = 0;
	int have_comp = 0;
	int found = 0;

	while (1) {
		if (i > buf_max) {
			goto bail;
		}

		if (*str == '=') {
			if (!have_comp && (i > 0)) {
				buf[i] = '\0';
				buf = level_str;
				i = 0;
				have_comp = 1;
			} else {
				goto bail;
			}
		} else if ((*str == ',') || (*str == '\0')) {
			if (have_comp && (i > 0)) {
				buf[i] = '\0';
				buf = comp_name;
				i = 0;
				have_comp = 0;
				char *endptr;
				int level = strtol(level_str, &endptr, 10);
				if ((*endptr != '\0') || (level > 9)) {
					goto bail;
				}
				int comp_id = log_get_component_id(comp_name);
				if (comp_id < 0) {
					goto bail;
				}
				if (log_set_level(comp_id, level)) {
					goto bail;
				}
				found++;
			} else {
				goto bail;
			}
			if (*str == '\0') {
				break;
			}
		} else {
			buf[i] = *str;
			i++;
		}

		str++;
	}

	return found;

bail:
	return -(str - levels + 1);
}

// -----------------------------------------------------------------------
int log_wants(unsigned component, unsigned level)
{
	// check if message is to be logged
	if (level > atom_load_acquire(&log_components[component].thr)) {
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int log_err(char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	vfprintf(stderr, msgfmt, vl);
	fprintf(stderr, "\n");

	va_end(vl);

	if (log_is_enabled()) {
		va_start(vl, msgfmt);
		pthread_mutex_lock(&log_mutex);
		fprintf(log_f, LOG_F_COMP LOG_F_EMPTY, log_components[L_EM4H].name, 0);
		fprintf(log_f, "ERROR: ");
		vfprintf(log_f, msgfmt, vl);
		fprintf(log_f, "\n");
		pthread_mutex_unlock(&log_mutex);
		va_end(vl);
	}

	return E_ERR;
}

// -----------------------------------------------------------------------
void log_log(unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_f, LOG_F_COMP LOG_F_EMPTY, log_components[component].name, level);
	vfprintf(log_f, msgfmt, vl);
	fprintf(log_f, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
}

// -----------------------------------------------------------------------
void log_log_cpu(unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list vl;
	va_start(vl, msgfmt);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_f, LOG_F_COMP LOG_F_CPU,
		log_components[component].name, level,
		(log_cycle_sr & 0b0000000000100000) ? "USR" : "OS",
		(log_cycle_sr & 0b0000000000001111),
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
void log_splitlog(unsigned component, unsigned level, char *text)
{
	char *p;
	char *start = text;

	pthread_mutex_lock(&log_mutex);

	fprintf(log_f, LOG_F_COMP LOG_F_EMPTY ".-------------------------------------------------------------------\n", log_components[component].name, level);
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
		}
		fprintf(log_f, LOG_F_COMP LOG_F_EMPTY "| %s\n", log_components[component].name, level, start);
		if (p) {
			start = p+1;
		} else {
			start = NULL;
		}
	}
	fprintf(log_f, LOG_F_COMP LOG_F_EMPTY "`-------------------------------------------------------------------\n", log_components[component].name, level);

	pthread_mutex_unlock(&log_mutex);
}

// -----------------------------------------------------------------------
static void log_log_timestamp(unsigned component, unsigned level, char *msg)
{
	struct timeval ct;
	char date[32];
	gettimeofday(&ct, NULL);
	strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
	log_log(component, level, "%s: %s", msg, date);
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
void log_log_dasm(unsigned component, unsigned level, int mod, int norm_arg, int short_arg, int16_t n)
{
	int nb = ((log_cycle_sr & 0b0000000000100000) >> 5) * (log_cycle_sr & 0b0000000000001111);
	emdas_dasm(emd, nb, log_cycle_ic);

	if (norm_arg) {
		if (mod) {
			log_log_cpu(component, level, "    %-20s N = 0x%x = %i, MOD = 0x%x = %i", dasm_buf, (uint16_t) n, n, mod, mod);
		} else {
			log_log_cpu(component, level, "    %-20s N = 0x%x = %i", dasm_buf, (uint16_t) n, n);
		}
	} else if (short_arg) {
		if (mod) {
			log_log_cpu(component, level, "    %-20s T = %i, MOD = 0x%x = %i", dasm_buf, n, mod, mod);
		} else {
			log_log_cpu(component, level, "    %-20s T = %i", dasm_buf, n);
		}
	} else {
		log_log_cpu(component, level, "    %-20s", dasm_buf);
	}
}

// -----------------------------------------------------------------------
// called when final configuration is assembled
void log_config(unsigned component, unsigned level, struct cfg_em400 *cfg)
{
	log_log(component, level, "Program to load: %s", cfg->program_name);
	log_log(component, level, "Loaded config: %s", cfg->cfg_filename);
	log_log(component, level, "Print help: %s", cfg->print_help ? "yes" : "no");
	log_log(component, level, "Use FPGA backend: %s", cfg->fpga ? "yes" : "no");
	if (!cfg->fpga) {
		log_log(component, level, "CPU emulation:");
		log_log(component, level, "   Emulation speed: %s", cfg->speed_real ? "real" : "max");
		log_log(component, level, "   Timer step: %i (%s at power-on)", cfg->timer_step, cfg->timer_start ? "enabled" : "disabled");
		log_log(component, level, "   CPU modifications: %s", cfg->cpu_mod ? "present" : "absent");
		log_log(component, level, "   IN/OU instructions: %s for user programs", cfg->cpu_user_io_illegal ? "illegal" : "legal");
		log_log(component, level, "   Hardware AWP: %s", cfg->cpu_awp ? "present" : "absent");
		log_log(component, level, "   CPU stop on nomem in OS block: %s", cfg->cpu_stop_on_nomem ? "yes" : "no");
		log_log(component, level, "Memory emulation:");
		log_log(component, level, "   Elwro modules: %i", cfg->mem_elwro);
		log_log(component, level, "   MEGA modules: %i", cfg->mem_mega);
		log_log(component, level, "   MEGA PROM image: %s", cfg->mem_mega_prom);
		log_log(component, level, "   MEGA boot: %s", cfg->mem_mega_boot ? "true" : "false");
		log_log(component, level, "   Segments for OS: %i", cfg->mem_os);
	} else {
		log_log(component, level, "FPGA backend:");
		log_log(component, level, "   Device: %s", cfg->fpga_dev);
		log_log(component, level, "   Link speed: %i", cfg->fpga_speed);
	}
	log_log(component, level, "KB: 0x%04x", cfg->keys);
	log_log(component, level, "Logging (%s):", cfg->log_enabled ? "enabled" : "disabled");
	log_log(component, level, "   File: %s", cfg->log_file);
	log_log(component, level, "   Levels: %s", cfg->log_levels);
	log_log(component, level, "I/O:");

	char buf[4096];
	int bpos;

	struct cfg_chan *chanc = cfg->chans;
	while (chanc) {
		log_log(component, level, "   Channel %2i: %s", chanc->num, chanc->name);

		struct cfg_unit *unitc = chanc->units;
		while (unitc) {
			bpos = 0;
			struct cfg_arg *args = unitc->args;
			while (args) {
				bpos += sprintf(buf+bpos, "%s", args->text);
				args = args->next;
				if (args) {
					bpos += sprintf(buf+bpos, ", ");
				}
			}
			log_log(component, level, "      Unit %2i: %s (%s)", unitc->num, unitc->name, buf);
			unitc = unitc->next;
		}

		chanc = chanc->next;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
