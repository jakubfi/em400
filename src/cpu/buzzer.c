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
#include <string.h>

#include "sound/sound.h"
#include "external/biquad/biquad.h"
#include "log.h"

// Tonsil GD 6/0,5 frequency response, more or less
#define SPEAKER_HP 400.0f // 350 Hz
#define SPEAKER_LP 3500.0f // -10dB @ 4500 Hz

static const struct snd_drv *snd;

static float sample_period;
static unsigned buffer_len;
static unsigned volume;

static float *snd_buf_tmp;
static float *snd_buf_in;
static float *snd_buf_in_end;
static float *snd_buf_in_pos;

static int speaker_filter;
static sf_biquad_state_st bq_lp;
static sf_biquad_state_st bq_hp;

static int16_t *output_buffer;

// -----------------------------------------------------------------------
static void buzzer_flush()
{
	// apply filter
	if (speaker_filter) {
		sf_biquad_process(&bq_hp, buffer_len, snd_buf_in, snd_buf_tmp);
		sf_biquad_process(&bq_lp, buffer_len, snd_buf_tmp, snd_buf_in);
	}

	// prepare final sample output
	for (int i=0 ; i<buffer_len ; i++) {
		// stereo 16-bit signed sample output at given volume
		int v = snd_buf_in[i] * volume * 32767/100;
		output_buffer[2*i] = v;
		output_buffer[2*i+1] = v;
	}

	// play the buffer
	int written = 0;
	while (written != buffer_len) {
		int res = snd->play(output_buffer+written, buffer_len-written);
		if (res > 0) {
			written += res;
		}
	}
}

// -----------------------------------------------------------------------
void buzzer_update(int ir, unsigned instruction_time)
{
	static int cnt;
	static int pir;
	static double time_pool;
	static float level = 1;
	static float level_prev = 1;

	// update current level (freq divider)
	if ((ir ^ pir) & 0x8000) {
		if (++cnt >= 16) {
			cnt = 0;
			level *= -1;
		}
	}
	pir = ir;

	// fill the sound buffer with available samples
	time_pool += instruction_time;
	while (time_pool >= sample_period) {
		time_pool -= sample_period;
		// *0.5 for slightly less aliasing
		*snd_buf_in_pos++ = level_prev + (level - level_prev) * 0.5;
		level_prev = level;

		// if buffer is full, flush it
		if (snd_buf_in_pos >= snd_buf_in_end) {
			snd_buf_in_pos = snd_buf_in;
			buzzer_flush();
		}
	}
}

// -----------------------------------------------------------------------
void buzzer_start()
{
	snd->start();
}

// -----------------------------------------------------------------------
void buzzer_stop()
{
	snd->stop();
}

// -----------------------------------------------------------------------
void buzzer_shutdown()
{
	free(snd_buf_tmp);
	free(snd_buf_in);
	free(output_buffer);
	snd->shutdown();
}

// -----------------------------------------------------------------------
int buzzer_init(struct cfg_em400 *cfg)
{
	volume = cfg->sound_volume;
	speaker_filter = cfg->speaker_filter;
	sample_period = 1000000000.0f / cfg->sound_rate;
	buffer_len = cfg->sound_buffer_len;

	if (volume > 100) {
		LOGERR("Adjusting sound volume from %i to 100 (max allowed).", volume);
		volume = 100;
	} else if (volume < 0) {
		LOGERR("Adjusting sound volume from %i to 0 (min allowed).", volume);
		volume = 0;
	}

	output_buffer = malloc(sizeof(int16_t) * 2 * buffer_len);
	if (!output_buffer) {
		LOGERR("Cannot allocate memory for output sound buffer.");
		goto cleanup;
	}
	snd_buf_tmp = malloc(sizeof(float) * buffer_len);
	if (!snd_buf_tmp) {
		LOGERR("Cannot allocate memory for temporary sound buffer.");
		goto cleanup;
	}
	snd_buf_in = malloc(sizeof(float) * buffer_len);
	if (!snd_buf_in) {
		LOGERR("Cannot allocate memory for input sound buffer.");
		goto cleanup;
	}

	snd_buf_in_pos = snd_buf_in;
	snd_buf_in_end = snd_buf_in + buffer_len;

	snd = snd_init(cfg);
	if (!snd) {
		LOGERR("Could not find sound driver: %s", cfg->sound_driver);
		goto cleanup;
	}

	sf_highpass(&bq_hp, cfg->sound_rate, SPEAKER_HP, 2.0f);
	sf_lowpass(&bq_lp, cfg->sound_rate, SPEAKER_LP, 0.0f);

	return E_OK;

cleanup:
	buzzer_shutdown();
	return E_ERR;
}

// vim: tabstop=4 shiftwidth=4 autoindent
