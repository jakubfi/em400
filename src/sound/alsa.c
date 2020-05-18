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
#include <alsa/asoundlib.h>
#include <alsa/error.h>

#include "log.h"
#include "external/iniparser/iniparser.h"
#include "sound/sound.h"

static snd_pcm_t *handle;

// -----------------------------------------------------------------------
int alsa_init(dictionary *cfg)
{
	int err;
	const char *cfg_output = iniparser_getstring(cfg, "sound:output", SOUND_DEFAULT_OUTPUT);
	err = snd_pcm_open(&handle, cfg_output, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		return LOGERR("ALSA snd_pcm_open() error: %s", snd_strerror(err));
	}

	const unsigned pcm_format = SND_PCM_FORMAT_S16;
	const unsigned pcm_access = SND_PCM_ACCESS_RW_INTERLEAVED;
	const unsigned channels = 2;
	const unsigned rate = iniparser_getint(cfg, "sound:rate", SOUND_DEFAULT_RATE);
	const unsigned resample = 1;
	const unsigned latency = iniparser_getint(cfg, "sound:latency", SOUND_DEFAULT_LATENCY);
	const unsigned latency_us = 1000 * latency;

	err = snd_pcm_set_params(handle, pcm_format, pcm_access, channels, rate, resample, latency_us);
	if (err < 0) {
		return LOGERR("ALSA snd_pcm_set_params() error: %s", snd_strerror(err));
	}

	LOG(L_EM4H, "ALSA sound output initialized. Output: %s, rate: %i, latency: %i ms", cfg_output, rate, latency);

	return E_OK;
}

// -----------------------------------------------------------------------
void alsa_shutdown()
{
	if (handle) {
		snd_pcm_close(handle);
	}
}

// -----------------------------------------------------------------------
long alsa_play(int16_t *buf, size_t frames)
{
	long res;

	res = snd_pcm_writei(handle, buf, frames);
	if (res < 0) {
		int rres = snd_pcm_recover(handle, res, 1);
		if (rres < 0) {
			LOG(L_EM4H, "ALSA stream recovery failed: %s", snd_strerror((int)rres));
		} else {
			LOG(L_EM4H, "ALSA playback failed (recovered): %s", snd_strerror((int)res));
		}
	}

	return res;
}

// -----------------------------------------------------------------------
void alsa_start()
{
	snd_pcm_prepare(handle);
}

// -----------------------------------------------------------------------
void alsa_stop()
{
	snd_pcm_drain(handle);
}

// -----------------------------------------------------------------------
const struct snd_drv snd_drv_alsa = {
	.name = "alsa",
	.init = alsa_init,
	.shutdown = alsa_shutdown,
	.play = alsa_play,
	.start = alsa_start,
	.stop = alsa_stop,
};

// vim: tabstop=4 shiftwidth=4 autoindent
