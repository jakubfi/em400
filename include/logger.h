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

#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>

#ifndef LOGGER_H
#define LOGGER_H

#define LOG_COMP_MAX 32

/*
	Logfile format specification:

	%t timestamp
	%c component
	%l level
	%m message
*/

struct logger {
	FILE *out;
	char *format;
	char *comp_names[LOG_COMP_MAX];
	unsigned comp_thr[LOG_COMP_MAX];
	int quit;
	int color;
	pthread_t flusher_th;
	pthread_mutex_t log_mutex;
	pthread_cond_t log_cond;
};

struct logger *log_init(FILE *out, char *format, const char **comp_names);
void log_shutdown(struct logger *l);
int log_set_level(struct logger *l, int component, unsigned level);
int log_get_level(struct logger *l, unsigned component);
void log_do(struct logger *l, unsigned component, unsigned level, char *msgfmt, ...);
void log_vdo(struct logger *l, unsigned component, unsigned level, char *msgfmt, va_list ap); 
int log_allowed(struct logger *l, unsigned component, unsigned level);
int log_get_component_id(struct logger *l, char *name);
char * log_get_component_name(struct logger *l, unsigned component);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
