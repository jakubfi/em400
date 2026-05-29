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

#include <inttypes.h>
#include <stdlib.h>
#include "libem400.h"

int sound_init(struct em400_cfg_buzzer *cfg);
void sound_shutdown(void);
long sound_play(int16_t *buf, size_t frames);
void sound_start(void);
void sound_stop(void);

#endif
