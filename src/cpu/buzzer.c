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
#include <limits.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cpu.h"
#include "sound/sound.h"
#include "libem400.h"
#include "log.h"

// smoothing time of the ring fill error (covering several periods)
#define ADJUST_ERROR_TIME_CONST_S 2.0f
// CPU pacing correction for each millisecond of audio above or below the ring target
// set for the "default" 48kHz sampling rate
#define ADJUST_GAIN_PPM_PER_MS 48.0f
// allowed max +/- adjust
#define ADJUST_PPM_MAX 300

static bool sound_ready;

static float sample_period_ns;
static unsigned buffer_len;
static int sample_sign = 1;
static atomic_int volume_pct;

static float ring_fill_error; // how far off from the target fill, in frames
static float adjust_error_weight; // error smoothing weight
static float adjust_gain_ppm_per_frame; // ADJUST_GAIN_PPM_PER_MS as ppm per frame
static unsigned dropped_frames;
static unsigned drop_log_threshold;
static unsigned adjust_log_interval;

static float *snd_buf_end;
static float *snd_buf_pos;
static float *snd_buf_float;


// -----------------------------------------------------------------------
int buzzer_volume_pct_get()
{
	return atomic_load_explicit(&volume_pct, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void buzzer_volume_pct_set(int volume_pct_new)
{
	if (volume_pct_new > 100) {
		volume_pct_new = 100;
	} else if (volume_pct_new < 0) {
		volume_pct_new = 0;
	}
	atomic_store_explicit(&volume_pct, volume_pct_new, memory_order_relaxed);
}


// -----------------------------------------------------------------------
static void buzzer_adjust_update()
{
	static unsigned ticks;
	static long fill_min = LONG_MAX;
	static long fill_max = 0;

	long fill = sound_ring_fill();
	long target = sound_ring_target();

	if ((fill < 0) || (target < 0)) {
		ring_fill_error = 0;
		cpu_pacing_adjust(0);
		return;
	}

	// exponential moving average of the ring error, in frames
	ring_fill_error += ((float)(fill - target) - ring_fill_error) * adjust_error_weight;

	// clamp the error, not the ppm
	// error past the ppm limit only delays recovery
	float error_limit = ADJUST_PPM_MAX / adjust_gain_ppm_per_frame;
	if (ring_fill_error > error_limit) {
		ring_fill_error = error_limit;
	} else if (ring_fill_error < -error_limit) {
		ring_fill_error = -error_limit;
	}

	int ppm = ring_fill_error * adjust_gain_ppm_per_frame;
	cpu_pacing_adjust(ppm);

	// update and log stats
	if (fill < fill_min) fill_min = fill;
	if (fill > fill_max) fill_max = fill;
	if (++ticks >= adjust_log_interval) {
		ticks = 0;
		LOG(L_LIB,
			"Sound ring min..max: %ld..%ld frames, target: %ld frames, CPU pacing adjust: %+i ppm",
			fill_min, fill_max, target, ppm
		);
		fill_min = LONG_MAX;
		fill_max = 0;
	}
}

// -----------------------------------------------------------------------
static void buzzer_flush()
{
	// Push the raw /16 square wave to the sound layer. The speaker-model
	// filtering runs on the audio thread (see src/sound/sound.c).
	long written = sound_play(snd_buf_float, buffer_len);
	if (written < 0) return;

	if ((unsigned)written < buffer_len) {
		dropped_frames += buffer_len - written;
		if (dropped_frames >= drop_log_threshold) {
			LOG(L_LIB, "Sound ring overflow, dropped %u frames", dropped_frames);
			dropped_frames = 0;
		}
	}

	buzzer_adjust_update();
}

// -----------------------------------------------------------------------
void buzzer_update(int ir, unsigned instruction_time_ns)
{
	static int cnt;
	static int prev_ir;
	static double time_elapsed_ns;

	// f32 full-scale is +/-1.0; /4 headroom to accommodate the resonant
	// high-pass overshooting in the speaker-model filter on the audio thread.
	float amplitude = (float) buzzer_volume_pct_get() / 100.0f / 4.0f;

	// update current level (IR0 div by 16)
	if ((ir ^ prev_ir) & 0x8000) {
		if (++cnt >= 16) { // output changes polarity every 16 changes on the input
			cnt = 0;
			sample_sign = -sample_sign;
		}
	}
	prev_ir = ir;

	// fill the sound buffer with available samples
	time_elapsed_ns += instruction_time_ns;
	while (time_elapsed_ns > sample_period_ns) {
		time_elapsed_ns -= sample_period_ns;
		*snd_buf_pos++ = sample_sign * amplitude;

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
	// Drop the remaining audio in the buzzer buffer,
	// so the next START doesn't start with audio garbage
	snd_buf_pos = snd_buf_float;
	ring_fill_error = 0;
	cpu_pacing_adjust(0);
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
		LOG(L_CPU, "Buzzer already initialized");
		return E_ERR;
	}

	if ((cfg->sample_rate <= 0) || (cfg->buffer_len <= 0)) {
		return LOGERR("Invalid sound configuration: rate %i, buffer length %i", cfg->sample_rate, cfg->buffer_len);
	}

	adjust_log_interval = 2 * cfg->sample_rate / cfg->buffer_len;
	if (adjust_log_interval < 1) adjust_log_interval = 1;

	sample_period_ns = 1000000000.0f / cfg->sample_rate;
	buffer_len = cfg->buffer_len;
	drop_log_threshold = cfg->sample_rate;

	adjust_gain_ppm_per_frame = ADJUST_GAIN_PPM_PER_MS * 1000.0f / cfg->sample_rate;

	float buffer_time_s = (float) buffer_len / cfg->sample_rate;
	adjust_error_weight = buffer_time_s / ADJUST_ERROR_TIME_CONST_S;
	if (adjust_error_weight > 1.0f) adjust_error_weight = 1.0f;

	buzzer_volume_pct_set(cfg->volume);

	snd_buf_float = malloc(sizeof(float) * buffer_len);
	if (!snd_buf_float) {
		LOG(L_CPU, "Cannot allocate memory for input sound buffer.");
		goto cleanup;
	}

	snd_buf_pos = snd_buf_float;
	snd_buf_end = snd_buf_float + buffer_len;

	if (sound_init(cfg) != E_OK) {
		LOG(L_CPU, "Could not initialize sound subsystem.");
		goto cleanup;
	}
	sound_ready = true;

	LOG(L_CPU, "Buzzer enabled. Volume: %i, buffer length: %i frames", buzzer_volume_pct_get(), buffer_len);

	return E_OK;

cleanup:
	buzzer_shutdown();
	return E_ERR;
}

// vim: tabstop=4 shiftwidth=4 autoindent
