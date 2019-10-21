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
#include <alsa/asoundlib.h>
#include <time.h>
#include <signal.h>

#include "log.h"

pthread_t sound_th;

static int buzzer_val = 0;

snd_pcm_t *handle;
static char *device = "default";
snd_output_t *output = NULL;
#define BUF_SIZE 100*1024*1024
#define headstart 4*1024
unsigned char buffer[BUF_SIZE];
unsigned char zbuffer[16*1024];
unsigned char *buffer_end = buffer + BUF_SIZE;
unsigned char *wp = buffer+headstart;
unsigned char *rp = buffer;

#define VOLUME 10
#define FREQ 48000
#define SAMPLE_PERIOD (1000000000/FREQ)

// -----------------------------------------------------------------------
void buzzer_silence()
{
	buzzer_val = 0;
}

// -----------------------------------------------------------------------
void buzzer_update(uint16_t ir, unsigned instruction_time)
{
	static int cnt;
	static uint16_t pir;
	static int time_pool;

	if ((ir ^ pir) & 0x8000) {
		cnt++;
		if (cnt >= 32) {
			if (buzzer_val) {
				buzzer_val *= -1;
			} else {
				buzzer_val = VOLUME;
			}
			cnt = 0;
		}
	}
	pir = ir;

	time_pool += instruction_time;

	while (time_pool >= SAMPLE_PERIOD) {
		time_pool -= SAMPLE_PERIOD;
		*wp++ = buzzer_val;
		if (wp == buffer_end) {
			wp = buffer;
		}
	}
}

// -----------------------------------------------------------------------
void * sound_thread(void *ptr)
{
	int err;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;

	err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	err = snd_pcm_set_params(handle, SND_PCM_FORMAT_S8, SND_PCM_ACCESS_RW_INTERLEAVED, 1, FREQ, 1, 50000);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	for (int i=0 ; i<16*1024 ; i++) zbuffer[i] = 0;

	while (1) {
		int wsize;
		if (buzzer_val == 0) {
			wsize = 512;
			frames = snd_pcm_writei(handle, zbuffer, wsize);
		} else {
			wsize = 512;
			frames = snd_pcm_writei(handle, rp, wsize);
			rp += frames;
		}
		if (frames < 0) {
			printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
		}
		if (frames > 0 && (frames < wsize)) {
			printf("Short write (expected %i, wrote %li)\n", wsize, frames);
		}
	}

	snd_pcm_close(handle);
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int buzzer_init()
{
	if (pthread_create(&sound_th, NULL, sound_thread, NULL)) {
		return LOGERR("Failed to spawn sound thread.");
	}

	pthread_setname_np(sound_th, "sound");

	return E_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
