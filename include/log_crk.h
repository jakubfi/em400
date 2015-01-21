//  Copyright (c) 2014-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef LOG_CRK_H
#define LOG_CRK_H

#include <inttypes.h>
#include <emcrk/process.h>

void log_crk_init();
void log_crk_shutdown();

void log_reset_process();
void log_update_process();
char * log_get_current_process();
char * log_ctx_stringify(struct crk5_process *process);
void log_log_process(unsigned component, unsigned level);
void log_handle_syscall(unsigned component, unsigned level, int number, int nb, int addr, uint16_t r4);
void log_handle_syscall_ret(unsigned component, unsigned level, uint16_t ic, uint16_t sr, uint16_t r4);
void log_syscall_reset();
void log_check_os();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
