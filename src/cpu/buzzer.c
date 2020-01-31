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
	static int buzzer_val;

	// update current level (freq divider)

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

	// fill the sound buffer (+filtering)

	time_pool += instruction_time;

	while (time_pool >= sample_period) {
		time_pool -= sample_period;
		int dst_level = (volume*32767/100) * buzzer_val;
		int level = old_level + (dst_level-old_level) * 0.7;
		// two channels
		*wp++ = level;
		*wp++ = level;
		old_level = level;
	}

	// dump buffer to the soundcard

	if (wp >= buffer_end) {
		int written = 0;
		while (written != buffer_len) {
			int res = snd->play(buffer+written, buffer_len-written);
			if (res > 0) {
				written += res;
			}
		}
		wp = buffer;
	}
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
	sample_period = 1000000000.0f / cfg->sound_rate;;
	buffer_len = cfg->sound_buffer_len;

	buffer = malloc(sizeof(int16_t) * 2 * buffer_len);
	if (!buffer) {
		return LOGERR("Cannot allocate memory for sound buffer.");
	}

	memset(buffer, 0, sizeof(int16_t) * 2 * buffer_len);
	buffer_end = buffer + buffer_len * 2;
	wp = buffer;

	snd = snd_init(cfg);
	if (!snd) {
		return LOGERR("Could not find sound driver: %s", cfg->sound_driver);
	}

	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
