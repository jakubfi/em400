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
#include <stdio.h>

#include "log.h"
#include "cfg.h"
#include "sound/sound.h"

FILE *out;

// -----------------------------------------------------------------------
int file_init(em400_cfg *cfg)
{
	const char *cfg_output = cfg_getstr(cfg, "sound:output", CFG_DEFAULT_SOUND_OUTPUT_FILE);
	out = fopen(cfg_output, "w");
	if (!out) {
		return LOGERR("Error opening sound output file: %s", cfg_output);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void file_shutdown()
{
	if (out) {
		fclose(out);
	}
}

// -----------------------------------------------------------------------
long file_play(int16_t *buf, size_t frames)
{
	long res;

	res = fwrite(buf, 2, frames, out);
	if (res < 0) {
		LOG(L_EM4H, "Sound output file write failed");
	}

	return res;
}

// -----------------------------------------------------------------------
void file_stop()
{
}

// -----------------------------------------------------------------------
void file_start()
{
}

// -----------------------------------------------------------------------
const struct snd_drv snd_drv_file = {
	.name = "file",
	.init = file_init,
	.shutdown = file_shutdown,
	.play = file_play,
	.start = file_start,
	.stop = file_stop,

};

// vim: tabstop=4 shiftwidth=4 autoindent
