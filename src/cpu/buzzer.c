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

#define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "sound/sound.h"
#include "log.h"

static const struct snd_drv *snd;

static pthread_t buzzer_th;

static unsigned rate;
static unsigned headstart;
static unsigned buffer_size;
static float sample_period;
static unsigned chunk_size;
static unsigned volume;
static int buzzer_val = 0;

static int16_t *buffer;
static int16_t *buf_end;
static int16_t *wp;
static int16_t *rp;

// -----------------------------------------------------------------------
void buzzer_update(int ir, unsigned instruction_time)
{
	static int cnt;
	static int pir;
	static double time_pool;

	if (ir == -1) {
		buzzer_val = 0;
	} else {
		int has_changed = (ir ^ pir) & 0x8000;
		if (has_changed) {
			cnt++;
			if (cnt >= 32) {
				if (buzzer_val != 0) {
					buzzer_val *= -1;
				} else {
					buzzer_val = 1;
				}
				cnt = 0;
			}
		}
		pir = ir;
	}

	time_pool += instruction_time;

	while (time_pool >= sample_period) {
		time_pool -= sample_period;
		int sample = (volume*32767/100) * buzzer_val;
		*wp++ = sample;
		*wp++ = sample;
		if (wp >= buf_end) {
			wp = buffer;
		}
	}
}

// -----------------------------------------------------------------------
void * buzzer_thread(void *ptr)
{
	size_t frames_written;
	size_t frames_requested;
	size_t frames_left;

	while (1) {
		frames_left = (buf_end - rp)/2;
		if (frames_left < chunk_size) {
			frames_requested = frames_left;
		} else {
			frames_requested = chunk_size;
		}
		frames_written = snd->play(rp, frames_requested);
		rp += frames_written*2;
		if (rp >= buf_end) {
			rp = buffer;
		}

		if ((frames_written > 0) && (frames_written < frames_requested)) {
			printf("Short write (expected %li, wrote %li)\n", frames_requested, frames_written);
		}
	}

	snd->shutdown();
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int buzzer_init(struct cfg_em400 *cfg)
{
	if ((cfg->speed_real == 0) || (cfg->cpu_speed_factor < 0.5f) || (cfg->cpu_speed_factor > 1.5f)) {
		return LOGERR("EM400 needs to be configured with speed_real=true and 1.5 >= cpu_speed_factor >= 0.5 for the buzzer emulation to work.");
	}

	if ((cfg->sound_volume > 100) || (cfg->sound_volume < 0)) {
		return LOGERR("Buzzer volume has to be set between 0 and 100.");
	}

	volume = cfg->sound_volume;
	rate = cfg->sound_rate;
	headstart = cfg->sound_buffer_size;
	sample_period = 1000000000.0f / rate;
	chunk_size = cfg->sound_chunk_size;

	buffer_size = 4 * headstart;
	buffer = malloc(sizeof(int16_t) * 2 * buffer_size);
	if (!buffer) {
		return LOGERR("Cannot allocate memory for sound buffer.");
	}

	memset(buffer, 0, sizeof(int16_t) * 2 * buffer_size);

	buf_end = buffer + buffer_size*2;
	wp = buffer + headstart*2;
	rp = buffer;

	snd = snd_init(cfg);
	if (!snd) {
		return LOGERR("Could not find sound driver: %s", cfg->sound_driver);
	}

	if (pthread_create(&buzzer_th, NULL, buzzer_thread, NULL)) {
		return LOGERR("Failed to spawn buzzer thread.");
	}

	pthread_setname_np(buzzer_th, "buzzr");

	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
