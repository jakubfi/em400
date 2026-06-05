//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __WATCH_H_
#define __WATCH_H_

#include "cp/eval.h"

// A "watch" is a stored expression the user inspects while the machine is
// stopped (on a breakpoint or single-stepped). Unlike a breakpoint it is never
// evaluated on the CPU thread: it holds only the source text and is parsed and
// evaluated on demand, on the UI thread, by watch_eval(). The whole module is
// therefore plain (no atomics) and MUST be driven from a single thread (the UI
// thread) - see watch.c.

typedef void (*watch_iter_f)(unsigned id, const char *expr, void *ctx);

void watch_del_all();
int watch_delete(unsigned id);
int watch_edit(unsigned id, char *expression, char **err_msg, int *err_beg, int *err_end);
int watch_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end);
void watch_foreach(watch_iter_f cb, void *ctx);
int watch_add(char *expression, char **err_msg, int *err_beg, int *err_end);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
