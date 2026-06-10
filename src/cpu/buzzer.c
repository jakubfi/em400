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

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sound/sound.h"
#include "libem400.h"
#include "log.h"

static bool sound_ready;

static float sample_period_ns;
static unsigned buffer_len;
static float audio_sample;

static float *snd_buf_end;
static float *snd_buf_pos;
static float *snd_buf_float;


// -----------------------------------------------------------------------
static void buzzer_flush()
{
	// Push the raw /16 square wave to the sound layer. The speaker-model
	// filtering runs on the audio thread (see src/sound/sound.c).
	int written = 0;
	while (written != buffer_len) {
		long res = sound_play(snd_buf_float+written, buffer_len-written);
		if (res > 0) {
			written += res;
		} else {
			// res <= 0: sound_play made no progress (shutdown or a
			// ring error); retrying with the same args can't help.
			break;
		}
	}
}

// -----------------------------------------------------------------------
void buzzer_update(int ir, unsigned instruction_time_ns)
{
	static int cnt;
	static int prev_ir;
	static double time_elapsed_ns;

	// update current level (IR0 div by 16)
	if ((ir ^ prev_ir) & 0x8000) {
		if (++cnt >= 16) { // output changes polarity every 16 changes on the input
			cnt = 0;
			audio_sample *= -1;
		}
	}
	prev_ir = ir;

	// fill the sound buffer with available samples
	time_elapsed_ns += instruction_time_ns;
	while (time_elapsed_ns > sample_period_ns) {
		time_elapsed_ns -= sample_period_ns;
		*snd_buf_pos++ = audio_sample;

		// if buffer is full, flush it
		if (snd_buf_pos >= snd_buf_end) {
			snd_buf_pos = snd_buf_float;
			buzzer_flush();
		}
	}
}

// -----------------------------------------------------------------------
void buzzer_start()
{
	sound_start();
}

// -----------------------------------------------------------------------
void buzzer_stop()
{
	sound_stop();
}

// -----------------------------------------------------------------------
void buzzer_shutdown()
{
	free(snd_buf_float);
	snd_buf_float = NULL;

	if (sound_ready) {
		sound_shutdown();
		sound_ready = false;
	}
}

// -----------------------------------------------------------------------
int buzzer_init(const struct em400_sound_cfg *cfg)
{
	if (sound_ready) {
		return LOGERR("Buzzer already initialized");
	}

	sample_period_ns = 1000000000.0f / cfg->sample_rate;
	buffer_len = cfg->buffer_len;

	int volume = cfg->volume;
	if (volume > 100) {
		LOGERR("Adjusting sound volume from %i to 100 (max allowed).", volume);
		volume = 100;
	} else if (volume < 0) {
		LOGERR("Adjusting sound volume from %i to 0 (min allowed).", volume);
		volume = 0;
	}
	// f32 full-scale is +/-1.0; /4 headroom to accommodate the resonant
	// high-pass overshooting in the speaker-model filter on the audio thread.
	audio_sample = (float) volume / 100.0f / 4.0f;

	snd_buf_float = malloc(sizeof(float) * buffer_len);
	if (!snd_buf_float) {
		LOGERR("Cannot allocate memory for input sound buffer.");
		goto cleanup;
	}

	snd_buf_pos = snd_buf_float;
	snd_buf_end = snd_buf_float + buffer_len;

	if (sound_init(cfg) != E_OK) {
		LOGERR("Could not initialize sound subsystem.");
		goto cleanup;
	}
	sound_ready = true;

	LOG(L_CPU, "Buzzer enabled. Volume: %i, buffer length: %i frames", volume, buffer_len);

	return E_OK;

cleanup:
	buzzer_shutdown();
	return E_ERR;
}

// vim: tabstop=4 shiftwidth=4 autoindent
