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

#include "mem/mem.h"
#include "log.h"
#include "cfg.h"
#include "errors.h"
#include "emdas.h"
#include "atomic.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/decode.h"
#endif

#include <emcrk/kfind.h>
#include <emcrk/r40.h>
#include <emcrk/process.h>

// low-level stuff

#define LOG_FLUSH_DELAY_MS 200

struct log_component {
	char *name;
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
	{ "CRK5", 0 },
	{ "EM4H", 1 },
	{ "ALL", 0 }
};

int log_enabled;
int log_initialized;

static pthread_t log_flusher_th;
static pthread_mutex_t log_mutex;
static int log_flusher_stop;

static char *log_file;
static FILE *log_f;

static void * log_flusher(void *ptr);

// high-level stuff

#define LOG_F_COMP "%4s %1i | "
#define LOG_F_EMPTY "                     | "
#define LOG_F_CPU "%3s %2i:0x%04x %-6s | %s"

static uint16_t log_cycle_sr;
static uint16_t log_cycle_ic;

static struct crk5_process *process;

#define LOG_INT_INDENT_MAX 4*8
static int log_int_level = LOG_INT_INDENT_MAX;
static const char *log_int_indent = "--> --> --> --> --> --> --> --> ";

static int log_exl_number = -1;
static int log_exl_nb;
static int log_exl_addr;
static int log_exl_r4;

static struct emdas *emd;
static char *dasm_buf;

struct crk5_kern_result *kernel;

static void log_log_timestamp(unsigned component, unsigned level, char *msg);

// -----------------------------------------------------------------------
int log_init(struct cfg_em400 *cfg)
{
	int res;
	int ret = E_ALLOC;

	log_file = strdup(cfg->log_file);
	if (!log_file) {
		ret = E_ALLOC;
		goto cleanup;
	}

	if (cfg->log_enabled) {
		ret = log_enable();
		if (ret != E_OK) {
			goto cleanup;
		}
	}

	// set up thresholds
	res = log_setup_levels(cfg->log_levels);
	if (res < 0) {
		ret = E_LEVELS;
		goto cleanup;
	}

	pthread_mutex_init(&log_mutex, NULL);

	// initialize deassembler
	emd = emdas_create(cfg->cpu_mod ? EMD_ISET_MX16 : EMD_ISET_MERA400, mem_get);
	if (!emd) {
		ret = E_DASM;
		goto cleanup;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 0, 0);
	dasm_buf = emdas_get_buf(emd);

	log_initialized = 1;

	return E_OK;

cleanup:
	if (log_f) fclose(log_f);
	free(log_file);
	emdas_destroy(emd);
	return ret;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	int i;

	if (!log_initialized) return;

	emdas_destroy(emd);

	for (i=0 ; i<L_ALL; i++) {
		atom_store(&log_components[i].thr, 0);
	}

	crk5_kern_res_drop(kernel);
	log_disable();
	free(log_file);
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

	atom_store(&log_enabled, 0);

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
		return E_FILE_OPEN;
	}

	log_log_timestamp(L_EM4H, 0, "Opened log");

	// start up flusher thread
	log_flusher_stop = 0;
	if (pthread_create(&log_flusher_th, NULL, log_flusher, NULL) != 0) {
		fclose(log_f);
		return E_THREAD;
	}

	atom_store(&log_enabled, 1);

	return E_OK;
}

// -----------------------------------------------------------------------
int log_is_enabled()
{
	return atom_load(&log_enabled);
}

// -----------------------------------------------------------------------
int log_set_level(unsigned component, unsigned level)
{
	// set level for specified component
	if (component != L_ALL) {
		atom_store(&log_components[component].thr, level);
	// set level for all components
	} else {
		for (int i=0 ; i<L_ALL; i++) {
			atom_store(&log_components[i].thr, level);
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
int log_get_level(unsigned component)
{
	int level;

	level = atom_load(&log_components[component].thr);

	return level;
}

// -----------------------------------------------------------------------
char * log_get_component_name(unsigned component)
{
	return log_components[component].name;
}

// -----------------------------------------------------------------------
int log_get_component_id(char *name)
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
				log_set_level(comp_id, level);
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
	if (level > atom_load(&log_components[component].thr)) {
		return 0;
	}
	return 1;
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
		process ? process->name : "",
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

	fprintf(log_f, LOG_F_COMP LOG_F_EMPTY " .---------------------------------------------------------\n", log_components[component].name, level);
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
		}
		fprintf(log_f, LOG_F_COMP LOG_F_EMPTY " | %s\n", log_components[component].name, level, start);
		if (p) {
			start = p+1;
		} else {
			start = NULL;
		}
	}
	fprintf(log_f, LOG_F_COMP LOG_F_EMPTY " `---------------------------------------------------------\n", log_components[component].name, level);

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
void log_handle_syscall(unsigned component, unsigned level, int number, int nb, int addr, int r4)
{
	log_exl_number = number;
	log_exl_nb = nb;
	log_exl_addr = addr;
	log_exl_r4 = r4;

	char *details;
#ifdef WITH_DEBUGGER
	details = decode_exl(nb, r4, number);
#else
	details = malloc(128);
	sprintf(details, "[details missing]");
#endif
	log_splitlog(component, level, details);
	free(details);
}

// -----------------------------------------------------------------------
void log_log_process(unsigned component, unsigned level)
{
	static int last_pid;

	if (!process || !process->name || !*process->name) return;
	if (process->num == last_pid) return;

	last_pid = process->num;

    static char buf[1024];
    char *b = buf;
    int pos = 0;

    char *r0s = int2binf("........ ........", process->r0, 16);
    char *srs = int2binf(".......... . . ....", process->sr, 16);
    char *szabme = int2binf("........ ........", process->segmap, 16);
    char *state = int2binf("........ ........", process->state, 16);
    int ctx_q = (process->sr >> 5) & 1;
    int ctx_nb = process->sr & 0b1111;

    pos += sprintf(b+pos, "Process 0x%04x %s ", process->addr, process->name);
    pos += sprintf(b+pos, "----------------------------------------------\n");
    pos += sprintf(b+pos, "Q:NB: %i:%i IC: 0x%04x R0: %s SR: %s\n", ctx_q, ctx_nb, process->ic, r0s, srs);
    pos += sprintf(b+pos, "R1-7: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", process->r1, process->r2, process->r3, process->r4, process->r5, process->r6, process->r7);
    pos += sprintf(b+pos, "----------------------------------------------\n");
    pos += sprintf(b+pos, "State: %s (0x%04x), ", state, process->state);
    pos += sprintf(b+pos, "Prio: %i, ", process->prio);
    pos += sprintf(b+pos, "Size: %i words, %i segments (%s) \n", process->size, process->segments, szabme);

    pos += sprintf(b+pos, "Next: 0x%04x ",  process->next_proc);
    pos += sprintf(b+pos, "Parent: 0x%04x ", process->parent);
    pos += sprintf(b+pos, "Children: 0x%04x, next child: 0x%04x \n", process->children, process->next_child);

    pos += sprintf(b+pos, "NUM: 0x%04x \n", process->num);
    pos += sprintf(b+pos, "ALLS: 0x%04x \n", process->ALLS);
    pos += sprintf(b+pos, "CHTIM: 0x%04x \n", process->CHTIM);
    pos += sprintf(b+pos, "DEVI: 0x%04x ", process->DEVI);
    pos += sprintf(b+pos, "DEVO: 0x%04x \n", process->DEVO);
    pos += sprintf(b+pos, "USAL: 0x%04x \n", process->USAL);
    pos += sprintf(b+pos, "STRLI: 0x%04x \n", process->STRLI);
    pos += sprintf(b+pos, "BUFLI: 0x%04x \n", process->BUFLI);
    pos += sprintf(b+pos, "LARUS: 0x%04x \n", process->LARUS);
    pos += sprintf(b+pos, "LISMEM: 0x%04x \n", process->LISMEM);
    pos += sprintf(b+pos, "NXTMEM: 0x%04x \n", process->NXTMEM);
    pos += sprintf(b+pos, "BAR: 0x%04x \n", process->BAR);
    pos += sprintf(b+pos, "BLPASC: 0x%04x \n", process->BLPASC);
    pos += sprintf(b+pos, "IC: 0x%04x R0: 0x%04x SR: 0x%04x \n", process->_ic, process->_r0, process->_sr);
    pos += sprintf(b+pos, "R1-7: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", process->_r1, process->_r2, process->_r3, process->_r4, process->_r5, process->_r6, process->_r7);
    pos += sprintf(b+pos, "JDAD: 0x%04x \n", process->JDAD);
    pos += sprintf(b+pos, "Program start (JPAD): 0x%04x \n", process->start);
    pos += sprintf(b+pos, "FILDIC position (JACN): 0x%04x \n", process->JACN);
    pos += sprintf(b+pos, "TABUJB: 0x%04x \n", process->TABUJB);

	log_splitlog(component, level, buf);

    free(r0s);
    free(srs);
    free(state);
    free(szabme);

}

// -----------------------------------------------------------------------
void log_handle_syscall_ret(unsigned component, unsigned level, uint16_t n)
{
	if ((log_exl_number >= 0) && (log_cycle_ic == log_exl_addr) && ((log_cycle_sr & 0x1111) == log_exl_nb)) {
		char *details;
#ifdef WITH_DEBUGGER
		details = decode_exl(log_exl_nb, log_exl_r4, -log_exl_number);
#else
		details = malloc(128);
		sprintf(details, "[details missing]");
#endif
		log_splitlog(component, level, details);
		free(details);
		log_syscall_reset();
	}
}

// -----------------------------------------------------------------------
void log_syscall_reset()
{
	log_exl_number = -1; // indicate that there is no syscall in progress
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
// called when context is switched to a new process (SP, LIP)
void log_update_process()
{
	uint16_t *bprog;
	uint16_t *p;
	uint16_t buf[62];

	free(process);
	process = NULL;

	if (!kernel) return;

	bprog = mem_ptr(0, 0x62);
	if (!bprog) return;

	for (int i=0 ; i<62 ; i++) {
		p = mem_ptr(0, *bprog+i);
		if (!p) return;
		buf[i] = *p;
	}

	process = crk5_process_unpack(buf, *bprog, kernel->mod);
}

// -----------------------------------------------------------------------
// called when final configuration is assembled
void log_config(unsigned component, unsigned level, struct cfg_em400 *cfg)
{
	log_log(component, level, "Program to load: %s", cfg->program_name);
	log_log(component, level, "Loaded config: %s", cfg->cfg_filename);
	log_log(component, level, "Benchmark: %s", cfg->benchmark ? "yes" : "no");
	log_log(component, level, "Print help: %s", cfg->print_help ? "yes" : "no");
	log_log(component, level, "Exit on HLT>=040: %s", cfg->exit_on_hlt ? "yes" : "no");
	log_log(component, level, "Emulation speed: %s", cfg->speed_real ? "real" : "max");
	log_log(component, level, "Timer step: %i (%s at power-on)", cfg->timer_step, cfg->timer_start ? "enabled" : "disabled");
	log_log(component, level, "CPU modifications: %s", cfg->cpu_mod ? "present" : "absent");
	log_log(component, level, "IN/OU instructions: %s for user programs", cfg->cpu_user_io_illegal ? "illegal" : "legal");
	log_log(component, level, "Hardware AWP: %s", cfg->cpu_awp ? "present" : "absent");
	log_log(component, level, "KB: 0x%04x", cfg->keys);
	log_log(component, level, "CPU stop on nomem in OS block: %s", cfg->cpu_stop_on_nomem ? "yes" : "no");
	log_log(component, level, "Memory:");
	log_log(component, level, "   Elwro modules: %i", cfg->mem_elwro);
	log_log(component, level, "   MEGA modules: %i", cfg->mem_mega);
	log_log(component, level, "   MEGA PROM image: %s", cfg->mem_mega_prom);
	log_log(component, level, "   MEGA boot: %s", cfg->mem_mega_boot ? "true" : "false");
	log_log(component, level, "   Segments for OS: %i", cfg->mem_os);
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

// -----------------------------------------------------------------------
// called at every software reset (MCL)
void log_check_os()
{
	uint16_t *seg;

	free(kernel);
	kernel = NULL;

	uint16_t *kimg = malloc(2 * sizeof(uint16_t) * 4096);
	if (!kimg) {
		goto cleanup;
	}

	for (int i=0; i<2 ; i++) {
		seg = mem_ptr(0, i*4096);
		if (!seg) {
			goto cleanup;
		}
		memcpy(kimg+i*4096, seg, 2*4096);
	}

	kernel = crk5_kern_find(kimg, 2*4096);
	if (!kernel) {
		goto cleanup;
	}

	if (LOG_WANTS(L_CRK5, 1)) {
		log_log(L_CRK5, 1, "running CROOK for %s CPU, entry point @ 0x%04x, checksum: 0x%04x (%s)",
			kernel->mod ? "MX-16" : "MERA-400",
			kernel->entry_point,
			kernel->cksum_addr,
			kernel->cksum_stored == kernel->cksum_computed ? "OK" : "incorrect"
		);
	}

cleanup:
	free(kimg);
}

// vim: tabstop=4 shiftwidth=4 autoindent
