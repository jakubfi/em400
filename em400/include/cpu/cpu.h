//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CPU_H
#define CPU_H

#include <inttypes.h>

void cpu_disable_sint();
void cpu_reset();
void cpu_halt();
void cpu_step();

extern int16_t N;
extern int cpu_stop;

#ifdef WITH_DEBUGGER
extern uint16_t cycle_ic;
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
