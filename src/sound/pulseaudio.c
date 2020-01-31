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
#include <pulse/simple.h>
#include <pulse/error.h>

#include "log.h"
#include "cfg.h"
#include "sound/sound.h"

static pa_simple *s;
static int bytes_per_frame = 4;

// -----------------------------------------------------------------------
int pulseaudio_init(struct cfg_em400 *cfg)
{
	int err;

	// Don't know the relation between tlength and latency.
	// This seems to work fine.
	const unsigned tlength = cfg->sound_buffer_len * 1000000/cfg->sound_rate;

	const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = cfg->sound_rate,
		.channels = 2
	};
	const pa_buffer_attr ba = {
		.maxlength = -1,
		.tlength = tlength,
		.prebuf = -1,
		.minreq = -1,
		.fragsize = -1,
	};

	s = pa_simple_new(NULL, "EM400", PA_STREAM_PLAYBACK, NULL, "buzzer", &ss, NULL, &ba, &err);
	if (!s) {
		return LOGERR("pa_simple_new() failed: %s\n", pa_strerror(err));
	}

	LOG(L_EM4H, "PulseAudio initialized. Buffer size: %i samples, rate: %i Hz, tlength: %i",
		cfg->sound_buffer_len,
		cfg->sound_rate,
		tlength
	);

	return E_OK;
}

// -----------------------------------------------------------------------
void pulseaudio_shutdown()
{
	int err;
	pa_simple_flush(s, &err);
	pa_simple_free(s);
}

// -----------------------------------------------------------------------
long pulseaudio_play(int16_t *buf, size_t frames)
{
	int err;
	long res;

	res = pa_simple_write(s, buf, frames*bytes_per_frame, &err);
	if (res < 0) {
		printf("pa_simple_write() failed: %s\n", pa_strerror(err));
		exit(1);
	}

	return frames;
}

// -----------------------------------------------------------------------
const struct snd_drv snd_drv_pulseaudio = {
	.name = "pulseaudio",
	.init = pulseaudio_init,
	.shutdown = pulseaudio_shutdown,
	.play = pulseaudio_play,
};

// vim: tabstop=4 shiftwidth=4 autoindent
