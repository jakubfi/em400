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

#include "logger.h"

#define LOG_MAX_LEN (1024 * 4)
#define LOG_FLUSH_DELAY_MS 200

static void * log_flusher(void *ptr);

// -----------------------------------------------------------------------
struct logger *log_init(FILE *out, char *format, const char **comp_names)
{
	int i;
	struct logger *l = NULL;
	char **cn = (char **) comp_names;

	if (!out) {
		goto cleanup;
	}

	l = calloc(1, sizeof(struct logger));
	if (!l) {
		goto cleanup;
	}

	l->format = strdup(format);
	if (!l->format) {
		goto cleanup;
	}

	// set up component names
	for (i=0 ; i<LOG_COMP_MAX; i++) {
		if (cn && *cn) {
			l->comp_names[i] = *cn;
			cn++;
		} else {
			l->comp_names[i] = NULL;
		}
		l->comp_thr[i] = 0;
	}

	// too many components
	if (cn && *cn) {
		goto cleanup;
	}

	pthread_mutex_init(&l->log_mutex, NULL);
	pthread_cond_init(&l->log_cond, NULL);

	// set up flusher thread
	if (pthread_create(&l->flusher_th, NULL, log_flusher, l) != 0) {
		goto cleanup;
	}

	l->out = out;
	l->quit = 0;

	return l;

cleanup:
	if (l) {
		free(l->format);
		free(l);
	}
	return NULL;
}

// -----------------------------------------------------------------------
void log_shutdown(struct logger *l)
{
	int i;

	if (!l) return;

	pthread_mutex_lock(&l->log_mutex);

	for (i=0 ; i<LOG_COMP_MAX; i++) {
		l->comp_thr[i] = 0;
	}
	l->quit = 1;

	pthread_mutex_unlock(&l->log_mutex);

	pthread_join(l->flusher_th, NULL);

	fflush(l->out);
	free(l->format);
	free(l);
}

// -----------------------------------------------------------------------
int log_set_level(struct logger *l, int component, unsigned level)
{
	int i;

	if (!l) return -1;
	if (component >= LOG_COMP_MAX) return -2;

	pthread_mutex_lock(&l->log_mutex);

	// set level for specified component
	if (component >= 0) {
		l->comp_thr[component] = level;
	// set level for all components
	} else {
		for (i=0 ; i<LOG_COMP_MAX; i++) {
			l->comp_thr[i] = level;
		}
	}

	pthread_mutex_unlock(&l->log_mutex);

	return 0;
}

// -----------------------------------------------------------------------
int log_get_level(struct logger *l, unsigned component)
{
	int level;

	if (!l) return -1;
	if (component >= LOG_COMP_MAX) return -2;

	pthread_mutex_lock(&l->log_mutex);
	level = l->comp_thr[component];
	pthread_mutex_unlock(&l->log_mutex);

	return level;
}

// -----------------------------------------------------------------------
char * log_get_component_name(struct logger *l, unsigned component)
{
	if (!l) return NULL;

	char *comp_name;
	pthread_mutex_lock(&l->log_mutex);
	comp_name = l->comp_names[component];
	pthread_mutex_unlock(&l->log_mutex);
	return comp_name;
}

// -----------------------------------------------------------------------
int log_get_component_id(struct logger *l, char *name)
{
	int i;
	int comp = -1;

	if (!l) return -1;

	pthread_mutex_lock(&l->log_mutex);
	for (i=0 ; (i<LOG_COMP_MAX) && l->comp_names[i] ; i++) {
		if (!strcasecmp(l->comp_names[i], name)) {
			comp = i;
			break;
		}
	}
	pthread_mutex_unlock(&l->log_mutex);
	return comp;
}

// -----------------------------------------------------------------------
static void * log_flusher(void *ptr)
{
	struct logger *l = ptr;

	// flush log every LOG_FLUSH_DELAY_MS miliseconds
	while (1) {
		pthread_mutex_lock(&l->log_mutex);
		fflush(l->out);
		if (l->quit) {
			pthread_mutex_unlock(&l->log_mutex);
			break;
		}
		pthread_mutex_unlock(&l->log_mutex);
		usleep(LOG_FLUSH_DELAY_MS * 1000);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
static char * log_format(char *format, char *comp_name, unsigned level, char *msg, va_list ap)
{
	char *buf = malloc(LOG_MAX_LEN+1);
	int bpos = 0;
	char *esc = malloc(LOG_MAX_LEN+1);
	int epos = -1;
	char *date = malloc(32);

	char *in = format;
	char *out = buf;
	struct timeval ct;

	while (in && *in && (LOG_MAX_LEN-bpos > 0)) {
		if (epos >= 0) { // processing escape sequence
			if ((*in == '-') || ((*in >= '0') && (*in <= '9'))) {
				epos += snprintf(esc+epos, LOG_MAX_LEN-epos, "%c", *in);
			} else {
				switch (*in) {
					case 'c': // component
						epos += snprintf(esc+epos, LOG_MAX_LEN-epos, "s");
						bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, esc, comp_name);
						break;
					case 'l': // level
						epos += snprintf(esc+epos, LOG_MAX_LEN-epos, "i");
						bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, esc, level);
						break;
					case 't': // timestamp
						gettimeofday(&ct, NULL);
						strftime(date, 31, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));
						bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, "%s", date);
						break;
					case 'm': // message
						bpos += vsnprintf(out+bpos, LOG_MAX_LEN-bpos, msg, ap);
						break;
					case '%': // literal %
						bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, "%%");
						break;
					default: // print out unknown escape sequence
						bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, "%s%c", esc, *in);
						break;
				}
				epos = -1;
			}
		} else if (*in == '%') { // start escape sequence
			epos = 0;
			epos += snprintf(esc+epos, LOG_MAX_LEN-epos, "%%");
		} else { // literal
			bpos += snprintf(out+bpos, LOG_MAX_LEN-bpos, "%c", *in);
		}
		in++;
	}

	free(esc);
	free(date);

	return buf;
}

// -----------------------------------------------------------------------
int log_allowed(struct logger *l, unsigned component, unsigned level)
{
	if (!l) return 0;
	if (component >= LOG_COMP_MAX) return 0;

	// check if message is to be logged
	pthread_mutex_lock(&l->log_mutex);
	if (level > l->comp_thr[component]) {
		pthread_mutex_unlock(&l->log_mutex);
		return 0;
	}
	pthread_mutex_unlock(&l->log_mutex);
	return 1;
}

// -----------------------------------------------------------------------
void log_vdo(struct logger *l, unsigned component, unsigned level, char *msgfmt, va_list ap) 
{
	char *buf;

	if (!l) return;
	if (component >= LOG_COMP_MAX) return;

	// format log entry outside the lock
	buf = log_format(l->format, l->comp_names[component], level, msgfmt, ap);
	if (!buf) return;

	// write log
	pthread_mutex_lock(&l->log_mutex);
	fprintf(l->out, "%s\n", buf);
	pthread_mutex_unlock(&l->log_mutex);

	free(buf);
}

// -----------------------------------------------------------------------
void log_do(struct logger *l, unsigned component, unsigned level, char *msgfmt, ...)
{
	va_list ap;
	va_start(ap, msgfmt);
	log_vdo(l, component, level, msgfmt, ap);
	va_end(ap);
}


// vim: tabstop=4 shiftwidth=4 autoindent
