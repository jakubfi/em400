//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef TIMER_H
#define TIMER_H

#include <signal.h>

// TIMER_PERIOD is a value (in miliseconds) from the
// following sets (different sources give different sets):
// - 2, 4, 8, 10, 20
// - 2, 10, 20, 40, 80
#define TIMER_PERIOD		10 // miliseconds

void * timer_thread(void *ptr);
int timer_init();
void timer_shutdown();

#endif

// vim: tabstop=4
