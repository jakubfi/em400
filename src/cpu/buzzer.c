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
#include "log.h"

static const struct snd_drv *snd;

static float sample_period;
static unsigned buffer_len;
static unsigned volume;

static int16_t *buffer;
static int16_t *buffer_end;
static int16_t *wp;

// -----------------------------------------------------------------------
void buzzer_update(int ir, unsigned instruction_time)
{
	static int cnt;
	static int pir;
	static double time_pool;
	static int old_level;
	static int dst_level;

	static int odst_level;
	static int lpcounter;

	// update current level (freq divider)

	if (ir == -1) {
		dst_level = 0;
	} else {
		int level_has_changed = (ir ^ pir) & 0x8000;
		pir = ir;
		if (level_has_changed) {
			if (++cnt >= 16) {
				cnt = 0;
				if (dst_level) dst_level *= -1;
				else dst_level = volume*32767/100;
			}
		}
	}

	// fill the sound buffer (+apply filter)

	time_pool += instruction_time;

	while (time_pool >= sample_period) {
		if (odst_level != dst_level) {
			lpcounter = 0;
		} else {
			lpcounter++;
		}
		time_pool -= sample_period;
		int level = old_level + (dst_level-old_level) * 0.6;
		if (lpcounter > 20) {
			level = level * 15/(lpcounter);
		}
		// two channels
		*wp++ = level;
		*wp++ = level;
		old_level = level;
		odst_level = dst_level;
	}

	// dump buffer to the soundcard

	if (wp >= buffer_end) {
		wp = buffer;
		int written = 0;
		while (written != buffer_len) {
			int res = snd->play(buffer+written, buffer_len-written);
			if (res > 0) {
				written += res;
			}
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
int buzzer_init(struct cfg_em400 *cfg)
{
	if (cfg->sound_volume > 100) {
		LOGERR("Adjusting sound volume from %i to 100 (max allowed).", cfg->sound_volume);
		cfg->sound_volume = 100;
	} else if (cfg->sound_volume < 0) {
		LOGERR("Adjusting sound volume from %i to 0 (min allowed).", cfg->sound_volume);
		cfg->sound_volume = 0;
	}

	volume = cfg->sound_volume;
	sample_period = 1000000000.0f / cfg->sound_rate;;
	buffer_len = cfg->sound_buffer_len;

	buffer = malloc(sizeof(int16_t) * 2 * buffer_len);
	if (!buffer) {
		return LOGERR("Cannot allocate memory for sound buffer.");
	}
	buffer_end = buffer + buffer_len * 2;
	wp = buffer;

	snd = snd_init(cfg);
	if (!snd) {
		return LOGERR("Could not find sound driver: %s", cfg->sound_driver);
	}

	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
