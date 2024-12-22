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
#include "libem400.h"
#include "sound/sound.h"

static pa_simple *s;
static const int channels = 1;
static const int bytes_per_frame = channels * 2;

// -----------------------------------------------------------------------
int pulseaudio_init(struct em400_cfg_buzzer *cfg)
{
	int err;

	const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16NE,
		.rate = cfg->sample_rate,
		.channels = channels
	};
	const pa_buffer_attr ba = {
		.maxlength = -1,
		.tlength = cfg->latency * (cfg->sample_rate/1000),
		.prebuf = -1,
		.minreq = -1,
		.fragsize = -1,
	};

	s = pa_simple_new(NULL, "EM400", PA_STREAM_PLAYBACK, NULL, "CPU buzzer", &ss, NULL, &ba, &err);
	if (!s) {
		return LOGERR("PulseAudio stream open failed: %s", pa_strerror(err));
	}

	LOG(L_EM4H, "Pulseaudio sound output initialized. Rate: %i, latency: %i ms", cfg->sample_rate, cfg->latency);

	return E_OK;
}

// -----------------------------------------------------------------------
void pulseaudio_shutdown()
{
	int err;
	if (s) {
		pa_simple_flush(s, &err);
		pa_simple_free(s);
	}
}

// -----------------------------------------------------------------------
long pulseaudio_play(int16_t *buf, size_t frames)
{
	int err;
	long res;

	res = pa_simple_write(s, buf, frames*bytes_per_frame, &err);
	if (res < 0) {
		LOG(L_EM4H, "PulseAudio write failed: %s", pa_strerror(err));
	}

	return frames;
}

// -----------------------------------------------------------------------
void pulseaudio_start()
{

}

// -----------------------------------------------------------------------
void pulseaudio_stop()
{

}

// -----------------------------------------------------------------------
const struct snd_drv snd_drv_pulseaudio = {
	.name = "pulseaudio",
	.init = pulseaudio_init,
	.shutdown = pulseaudio_shutdown,
	.play = pulseaudio_play,
	.start = pulseaudio_start,
	.stop = pulseaudio_stop,
};

// vim: tabstop=4 shiftwidth=4 autoindent
