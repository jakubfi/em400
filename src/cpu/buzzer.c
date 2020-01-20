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
#include <alsa/asoundlib.h>

#include "log.h"

pthread_t buzzer_th;

#define HEADSTART (2*1024)
#define CHUNK_SIZE (256)
#define BUF_SIZE (4*HEADSTART)

#define FREQ 48000
#define SAMPLE_PERIOD (1000000000.0f/FREQ)
#define BUF_US (1000000*HEADSTART)/FREQ

int volume = 10;
static int buzzer_val = 0;

snd_pcm_t *handle;
static char *device = "default";
snd_output_t *output = NULL;

unsigned char buffer[BUF_SIZE];
unsigned char *buf_end = buffer + BUF_SIZE;
unsigned char *wp = buffer+HEADSTART;
unsigned char *rp = buffer;

// -----------------------------------------------------------------------
void buzzer_update(uint16_t ir, unsigned instruction_time)
{
	static int cnt;
	static uint16_t pir;
	static float time_pool;

	if ((ir ^ pir) & 0x8000) {
		cnt++;
		if (cnt >= 32) {
			if (buzzer_val) {
				buzzer_val *= -1;
			} else {
				buzzer_val = volume;
			}
			cnt = 0;
		}
	}
	pir = ir;

	time_pool += instruction_time;

	while (time_pool >= SAMPLE_PERIOD) {
		time_pool -= SAMPLE_PERIOD;
		*wp++ = buzzer_val;
		if (wp == buf_end) {
			wp = buffer;
		}
	}
}

// -----------------------------------------------------------------------
void * buzzer_thread(void *ptr)
{
	int err;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;

	err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	err = snd_pcm_set_params(handle, SND_PCM_FORMAT_S8, SND_PCM_ACCESS_RW_INTERLEAVED, 1, FREQ, 1, BUF_US);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	while (1) {
		int wsize;
		if (buf_end - rp < CHUNK_SIZE) {
			wsize = buf_end - rp;
		} else {
			wsize = CHUNK_SIZE;
		}
		frames = snd_pcm_writei(handle, rp, wsize);
		rp += frames;
		if (rp >= buf_end) {
			rp = buffer;
		}

		if (frames < 0) {
			printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
			exit(1);
		}
		if (frames > 0 && (frames < wsize)) {
			printf("Short write (expected %i, wrote %li)\n", wsize, frames);
		}
	}

	snd_pcm_close(handle);
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int buzzer_init(struct cfg_em400 *cfg)
{
	if ((cfg->speed_real == 0) || (cfg->cpu_speed_factor < 0.5) || (cfg->cpu_speed_factor > 1.5)) {
		return LOGERR("EM400 needs to be configured with speed_real=true and 1.5 >= cpu_speed_factor >= 0.5 for the buzzer emulation to work.");
	}

	if ((cfg->buzzer_volume > 127) || (cfg->buzzer_volume < 0)) {
		return LOGERR("Buzzer volume has to be set between 0 and 127.");
	}
	volume = cfg->buzzer_volume;

	if (pthread_create(&buzzer_th, NULL, buzzer_thread, NULL)) {
		return LOGERR("Failed to spawn buzzer thread.");
	}

	pthread_setname_np(buzzer_th, "buzzr");

	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
