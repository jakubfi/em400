//  Copyright (c) 2017 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CP_H
#define CP_H

#include <stdint.h>
#include <stdbool.h>

// Standard control panel interface
uint16_t cp_bus_w();
void cp_kb_set(uint16_t val); // TODO: stored in CP?
// TODO: step
// TODO: mode + LED
int cp_stopn(bool state); // TODO: flip-flop
// TODO: stop LED
void cp_cycle();
void cp_load();
void cp_store();
void cp_fetch();
void cp_start(bool state); // TODO: flip-flop
void cp_bin();
void cp_clear();
void cp_clock_set(int state); // TODO: flip-flop
int cp_clock_get();
bool cp_q_get();
bool cp_p_get();
int cp_mc_get();
bool cp_irq_get();
void cp_oprq();
void cp_reg_select(int id); // TODO: store position
bool cp_run_get();
bool cp_wait_get();
bool cp_alarm_get();
void cp_off(); // TODO: switch on/off/lock?
// TODO: ON LED?

// Emulator extensions
unsigned int cp_state();
bool cp_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
bool cp_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
