//  Copyright (c) 2019 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef SOUND_H
#define SOUND_H

#include "libem400.h"

void buzzer_silence();
int buzzer_init(struct em400_cfg_buzzer *cfg);
void buzzer_update(int ir, unsigned instruction_time);
void buzzer_stop();
void buzzer_start();
void buzzer_shutdown();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
