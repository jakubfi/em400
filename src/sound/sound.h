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
#include "cfg.h"

#define SOUND_DEFAULT_DRIVER "pulseaudio"
#define SOUND_DEFAULT_OUTPUT "default"
#define SOUND_DEFAULT_VOLUME 30
#define SOUND_DEFAULT_RATE 44100
#define SOUND_DEFAULT_LATENCY 20
#define SOUND_DEFAULT_BUFFER_LEN 128

typedef int (*snd_drv_init)(dictionary *cfg);
typedef void (*snd_drv_shutdown)(void);
typedef long (*snd_drv_play)(int16_t *buf, size_t frames);
typedef void (*snd_drv_stop)(void);

struct snd_drv {
	char *name;
	snd_drv_init init;
	snd_drv_shutdown shutdown;
	snd_drv_play play;
	snd_drv_stop start;
	snd_drv_stop stop;
};

const struct snd_drv * snd_init(dictionary *cfg);

#endif
