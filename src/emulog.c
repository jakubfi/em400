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
#include <assert.h>

#include <inttypes.h>

#include "emulog.h"
#include "errors.h"

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
static char *emulog_format;
static int emulog_paused;
static int emulog_quit;

static void * emulog_flusher(void *ptr);

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

// -----------------------------------------------------------------------
int emulog_init(int paused, char *filename, char *format, int level)
{
	int ret = E_ALLOC;

	emulog_f = fopen(filename, "a");
	if (!emulog_f) {
		ret = E_FILE_OPEN;
		goto cleanup;
	}

	emulog_format = strdup(format);
	if (!emulog_format) {
		ret = E_ALLOC;
		goto cleanup;
	}

	// set up thresholds
	for (int i=0 ; i<L_MAX; i++) {
		emulog_components[i].thr = level;
		//emulog_components[i].thr = 100;
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

	return E_OK;

cleanup:
	if (emulog_f) fclose(emulog_f);
	free(emulog_format);
	return ret;
}

// -----------------------------------------------------------------------
void emulog_shutdown()
{
	int i;

	pthread_mutex_lock(&emulog_mutex);

	for (i=0 ; i<L_MAX; i++) {
		emulog_components[i].thr = 0;
	}
	emulog_quit = 1;

	pthread_mutex_unlock(&emulog_mutex);

	pthread_join(emulog_flusher_th, NULL);

	fflush(emulog_f);
	fclose(emulog_f);
	free(emulog_format);
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
	assert(component < L_MAX);

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

	assert(component < L_MAX);

	pthread_mutex_lock(&emulog_mutex);
	level = emulog_components[component].thr;
	pthread_mutex_unlock(&emulog_mutex);

	return level;
}

// -----------------------------------------------------------------------
char * emulog_get_component_name(unsigned component)
{
	assert(component < L_MAX);

	return emulog_components[component].name;
}

// -----------------------------------------------------------------------
int emulog_get_component_id(char *name)
{
	int i;
	int comp = -1;

	assert(name);

	for (i=0 ; (i<L_MAX) ; i++) {
		if (!strcasecmp(emulog_components[i].name, name)) {
			comp = i;
			break;
		}
	}
	return comp;
}

// -----------------------------------------------------------------------
static char * log_format(char *comp_name, unsigned level, char *msg, va_list ap)
{
	// needs to be thread-safe
	char *buf = malloc(EMULOG_MAX_LEN+1);
	int bpos = 0;
	char *esc = malloc(EMULOG_MAX_LEN+1);
	int epos = -1;
	char *date = malloc(32);

	char *in = emulog_format;
	char *out = buf;
	struct timeval ct;

	while (in && *in && (EMULOG_MAX_LEN-bpos > 0)) {
		if (epos >= 0) { // processing escape sequence
			if ((*in == '-') || ((*in >= '0') && (*in <= '9'))) {
				epos += snprintf(esc+epos, EMULOG_MAX_LEN-epos, "%c", *in);
			} else {
				switch (*in) {
					case 'c': // component
						epos += snprintf(esc+epos, EMULOG_MAX_LEN-epos, "s");
						bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, esc, comp_name);
						break;
					case 'l': // level
						epos += snprintf(esc+epos, EMULOG_MAX_LEN-epos, "i");
						bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, esc, level);
						break;
					case 't': // timestamp
						gettimeofday(&ct, NULL);
						strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
						bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, "%s", date);
						break;
					case 'm': // message
						bpos += vsnprintf(out+bpos, EMULOG_MAX_LEN-bpos, msg, ap);
						break;
					case '%': // literal %
						bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, "%%");
						break;
					default: // print out unknown escape sequence
						bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, "%s%c", esc, *in);
						break;
				}
				epos = -1;
			}
		} else if (*in == '%') { // start escape sequence
			epos = 0;
			epos += snprintf(esc+epos, EMULOG_MAX_LEN-epos, "%%");
		} else { // literal
			bpos += snprintf(out+bpos, EMULOG_MAX_LEN-bpos, "%c", *in);
		}
		in++;
	}

	free(esc);
	free(date);

	return buf;
}

// -----------------------------------------------------------------------
void emulog_log(unsigned component, unsigned level, char *msgfmt, ...)
{
	char *buf;
	va_list vl;

	assert(component < L_MAX);

	// format log entry outside the lock
	va_start(vl, msgfmt);
	buf = log_format(emulog_components[component].name, level, msgfmt, vl);
	va_end(vl);

	if (buf) {
		// write log
		pthread_mutex_lock(&emulog_mutex);
		fprintf(emulog_f, "%s\n", buf);
		pthread_mutex_unlock(&emulog_mutex);
		free(buf);
	}
}

// -----------------------------------------------------------------------
void emulog_splitlog(unsigned component, unsigned level, char *text)
{
	char *p;
	char *start = text;

	assert(component < L_MAX);

	emulog_log(component, level, EMULOG_FORMAT_NONCPU "%s", " .---------------------------------------------------------");
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
			emulog_log(component, level, EMULOG_FORMAT_NONCPU " | %s", start);
			start = p+1;
		} else {
			emulog_log(component, level, EMULOG_FORMAT_NONCPU " | %s", start);
			start = NULL;
		}
	}
	emulog_log(component, level, EMULOG_FORMAT_NONCPU "%s", " `---------------------------------------------------------");
}

// -----------------------------------------------------------------------
int emulog_wants(unsigned component, unsigned level)
{
	assert(component < L_MAX);

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
	emulog_int_level += 4;
}

// -----------------------------------------------------------------------
void emulog_intlevel_inc()
{
	emulog_int_level -= 4;
}

// -----------------------------------------------------------------------
const char *emulog_intlevel_get_indent()
{
	return emulog_int_indent + emulog_int_level;
}

// vim: tabstop=4 shiftwidth=4 autoindent
