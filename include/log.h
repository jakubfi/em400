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

#ifndef LOG_H
#define LOG_H

#include <inttypes.h>

#include "cfg.h"
#include "atomic.h"

enum log_components {
	L_REG, L_MEM, L_CPU, L_OP, L_INT,
	L_IO,
	L_MX, L_PX, L_CCHR, L_CMEM,
	L_TERM, L_9425, L_WNCH, L_FLOP, L_PNCH, L_PNRD,
	L_CRK5,
	L_EM4H,
	L_ALL
};

extern int log_enabled;

int log_init(struct cfg_em400 *cfg);
void log_shutdown();

int log_enable();
void log_disable();
int log_is_enabled();

int log_set_level(unsigned component, unsigned level);
int log_get_level(unsigned component);
int log_setup_levels(char *levels);
int log_wants(unsigned component, unsigned level);

char * log_get_component_name(unsigned component);
int log_get_component_id(char *name);

void log_log(unsigned component, unsigned level, char *format, ...);
void log_splitlog(unsigned component, unsigned level, char *text);

void log_store_cycle_state(uint16_t sr, uint16_t ic);
void log_intlevel_reset();
void log_intlevel_dec();
void log_intlevel_inc();

void log_log_dasm(unsigned component, unsigned level, int mod, int norm_arg, int short_arg, int16_t n);
void log_log_cpu(unsigned component, unsigned level, char *msgfmt, ...);
void log_log_id(unsigned component, unsigned level, char *id, char *msgfmt, ...);

void log_config(unsigned component, unsigned level, struct cfg_em400 *cfg);

#define LOG_ID_LEN 20
#define LOG_ID_NAME __logid__
#define LOG_ID_DEF char LOG_ID_NAME[LOG_ID_LEN+1]

#define LOG_ENABLED (atom_load_acquire(&log_enabled))
#define LOG_WANTS(component, level) (LOG_ENABLED && log_wants(component, level))

#define LOG(component, level, format, ...) \
	if (LOG_WANTS(component, level)) \
		log_log(component, level, format, ##__VA_ARGS__)

#define LOGCPU(component, level, format, ...) \
	if (LOG_WANTS(component, level)) \
		log_log_cpu(component, level, format, ##__VA_ARGS__)

#endif

#define LOG_SET_ID(object, format, ...) \
	snprintf((object)->LOG_ID_NAME, LOG_ID_LEN, format, ##__VA_ARGS__); \
	(object)->LOG_ID_NAME[LOG_ID_LEN] = '\0'

#define LOG_GET_ID(object) \
	(object)->LOG_ID_NAME

#define LOGID(component, level, object, format, ...) \
	if (LOG_WANTS(component, level)) \
		log_log_id(component, level, (object)->LOG_ID_NAME, format, ##__VA_ARGS__)

// vim: tabstop=4 shiftwidth=4 autoindent
