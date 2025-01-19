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

#define _WITH_DPRINTF
#include <stdio.h>
#include <inttypes.h>
#include <stdatomic.h>

#include "libem400.h"
#include "cfg.h"

#define E_OK 0
#define E_ERR -1

#ifdef __cplusplus
extern "C" {
#endif

extern atomic_uint log_components_enabled;

int log_init(em400_cfg *cfg);
void log_shutdown();

int log_enable();
void log_disable();
unsigned log_is_enabled();

void log_component_enable(unsigned component);
void log_component_disable(unsigned component);
unsigned log_component_get(unsigned component);
int log_setup_components(const char *components);

const char * log_get_component_name(unsigned component);
int log_get_component_id(const char *name);

int log_err(const char *func, const char *msgfmt, ...);
void log_log(unsigned component, const char *func, const char *format, ...);
void log_splitlog(unsigned component, const char *func, const char *text);

void log_store_cycle_state(uint16_t sr, uint16_t ic);
void log_intlevel_reset();
void log_intlevel_dec();
void log_intlevel_inc();

void log_dasm(int arg, int16_t n, const char *comment);
void log_cpu(unsigned component, const char *msgfmt, ...);

#define LOG_ENABLED atomic_load_explicit(&log_components_enabled, memory_order_relaxed)
#define LOG_WANTS(component) (LOG_ENABLED & (1 << (component)))

#define LOG(component, format, ...) log_log(component, __func__, format, ##__VA_ARGS__)
#define LOGERR(format, ...) log_err(__func__, format, ##__VA_ARGS__)
#define LOGBLOB(component, txt) log_splitlog(component, __func__, txt)

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
