//  Copyright (c) 2020 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <strings.h>

#include "log.h"
#include "sound/sound.h"
#include "libem400.h"

#ifdef HAVE_ALSA
extern const struct snd_drv snd_drv_alsa;
#endif
#ifdef HAVE_PULSEAUDIO
extern const struct snd_drv snd_drv_pulseaudio;
#endif
extern const struct snd_drv snd_drv_file;

static const struct snd_drv *snd_drivers[] = {
#ifdef HAVE_ALSA
	&snd_drv_alsa,
#endif
#ifdef HAVE_PULSEAUDIO
	&snd_drv_pulseaudio,
#endif
	&snd_drv_file,
	NULL
};

// -----------------------------------------------------------------------
const struct snd_drv * snd_init(struct em400_cfg_buzzer *cfg)
{
	const struct snd_drv **snd_drv = snd_drivers;
	while (snd_drv && *snd_drv) {
		if (!strcasecmp(cfg->driver, (*snd_drv)->name)) {
			if ((*snd_drv)->init(cfg) == E_OK) {
				return *snd_drv;
			} else {
				LOGERR("Error initializing sound driver: %s", cfg->driver);
				return NULL;
			}
		}
		snd_drv++;
	}

	LOGERR("Could not find sound driver: %s", cfg->driver);
	return NULL;
}

// vim: tabstop=4 shiftwidth=4 autoindent
