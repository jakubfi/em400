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
#include "cfg.h"
#include "log.h"

// Tonsil GD 6/0,5 frequency response, more or less
#define SPEAKER_HP 380.0f // 350 Hz
#define SPEAKER_HP_RES 7.0f
#define SPEAKER_LP 3200.0f // -10dB @ 4500 Hz
#define SPEAKER_LP_RES 0.0f

static const struct snd_drv *snd;

static float sample_period;
static unsigned buffer_len;
static float audio_sample;

static int16_t *snd_buf_output;
static float *snd_buf_end;
static float *snd_buf_pos;
static float *snd_buf_float;

static sf_biquad_state_st bq_lp;
static sf_biquad_state_st bq_hp;


// -----------------------------------------------------------------------
static void buzzer_flush()
{
	// apply filter
	sf_biquad_process(&bq_hp, buffer_len, snd_buf_float, snd_buf_float);
	sf_biquad_process(&bq_lp, buffer_len, snd_buf_float, snd_buf_float);

	// prepare final sample output
	for (int i=0 ; i<buffer_len ; i++) {
		snd_buf_output[i] = snd_buf_float[i];
	}

	// play the buffer
	int written = 0;
	while (written != buffer_len) {
		int res = snd->play(snd_buf_output+written, buffer_len-written);
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


	// update current level (IR0 div by 16)
	if ((ir ^ pir) & 0x8000) {
		if (++cnt >= 16) { // output changes polarity every 16 changes on the input
			cnt = 0;
			audio_sample *= -1;
		}
	}
	pir = ir;

	// fill the sound buffer with available samples
	time_pool += instruction_time;
	while (time_pool > sample_period) {
		time_pool -= sample_period;
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
	free(snd_buf_float);
	free(snd_buf_output);
	if (snd) snd->shutdown();
}

// -----------------------------------------------------------------------
int buzzer_init(em400_cfg *cfg)
{
	int volume = cfg_getint(cfg, "sound:volume", CFG_DEFAULT_SOUND_VOLUME);
	int sample_rate = cfg_getint(cfg, "sound:rate", CFG_DEFAULT_SOUND_RATE);
	sample_period = 1000000000.0f / sample_rate;
	buffer_len = cfg_getint(cfg, "sound:buffer_len", CFG_DEFAULT_SOUND_BUFFER_LEN);

	if (volume > 100) {
		LOGERR("Adjusting sound volume from %i to 100 (max allowed).", volume);
		volume = 100;
	} else if (volume < 0) {
		LOGERR("Adjusting sound volume from %i to 0 (min allowed).", volume);
		volume = 0;
	}
	audio_sample = (float) volume * (32767/100)/4; // /4 to accomodate post-processing overdrive

	snd_buf_output = malloc(sizeof(int16_t) * buffer_len);
	if (!snd_buf_output) {
		LOGERR("Cannot allocate memory for output sound buffer.");
		goto cleanup;
	}

	snd_buf_float = malloc(sizeof(float) * buffer_len);
	if (!snd_buf_float) {
		LOGERR("Cannot allocate memory for input sound buffer.");
		goto cleanup;
	}

	snd_buf_pos = snd_buf_float;
	snd_buf_end = snd_buf_float + buffer_len;

	snd = snd_init(cfg);
	if (!snd) {
		LOGERR("Could not initialize sound subsystem.");
		goto cleanup;
	}

	sf_highpass(&bq_hp, sample_rate, SPEAKER_HP, SPEAKER_HP_RES);
	sf_lowpass(&bq_lp, sample_rate, SPEAKER_LP, SPEAKER_LP_RES);

	LOG(L_CPU, "Buzzer enabled. Volume: %i, buffer length: %i frames", volume, buffer_len);

	return E_OK;

cleanup:
	buzzer_shutdown();
	return E_ERR;
}

// vim: tabstop=4 shiftwidth=4 autoindent
