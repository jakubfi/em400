//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "external/miniaudio/miniaudio.h"
#include "libem400.h"
#include "log.h"
#include "sound/sound.h"

// two periods: device plays one, CPU thread fills the other
#define SOUND_PERIODS 2
// 1ms is the smallest period in miniaudio
#define SOUND_LATENCY_MIN_MS (SOUND_PERIODS * 1)
#define RING_FRAMES_MIN 8192

// MERA-400 Tonsil GD 6/0,5 speaker + steel-chassis "boxiness" model.
// Tuned by ear against a real machine.
// HP: speaker resonance
// LP: top-end rolloff
// BOX: peaking filter for the enclosure's resonance
//   FREQ: air-cavity mode: lower = woodier, higher = honkier
//   GAIN: dB of boost: more = boxier
//   Q: width: higher = more resonant
#define SPEAKER_HP_FREQ	760.0
#define SPEAKER_HP_Q	1.50
#define SPEAKER_LP_FREQ	6400.0
#define SPEAKER_LP_Q	1.0
#define BOX_FREQ		760.0
#define BOX_GAIN		10.0
#define BOX_Q			3.5

static const ma_uint32 channels = 1;

static ma_context context;
static ma_device device;
static ma_pcm_rb rb;
static ma_device_id matched_device_id;
static bool initialized;
static ma_uint32 ring_target; // buffer is primed to this (cushion, latency)
static ma_uint32 ring_discard_thr; // audio is discarded above this threshold (set above the target)
static ma_uint32 ring_frames; // actual ring capacity (highwater + extra space)
static atomic_bool priming;

// Speaker-model and chassis coloration filters.
// Touched only on the audio thread (data_callback),
// so no synchronization is needed.
// State carries continuously across callbacks and across device stop/start.
static ma_hpf2 speaker_hp;
static ma_lpf2 speaker_lp;
static ma_peak2 speaker_box;

// -----------------------------------------------------------------------
static void data_callback(ma_device *dev, void *out, const void *in, ma_uint32 frame_count)
{
	(void)dev;
	(void)in;

	float *dst = (float *)out;
	ma_uint32 remaining = frame_count;

	bool ring_priming = atomic_load_explicit(&priming, memory_order_relaxed);

	// nothing is consumed until the ring fills up
	if (ring_priming && (ma_pcm_rb_pointer_distance(&rb) >= (ma_int32)ring_target)) {
		atomic_store_explicit(&priming, false, memory_order_relaxed);
		ring_priming = false;
	}

	// if a stalled sink leaves backlog > discard threshold, just drop it,
	if (!ring_priming) {
		ma_int32 fill = ma_pcm_rb_pointer_distance(&rb);
		if (fill > (ma_int32)ring_discard_thr) {
			ma_pcm_rb_seek_read(&rb, (ma_uint32)(fill - (ma_int32)ring_target));
		}
	}

	while (!ring_priming && (remaining > 0)) {
		ma_uint32 to_read = remaining;
		void *src;
		if (ma_pcm_rb_acquire_read(&rb, &to_read, &src) != MA_SUCCESS || to_read == 0) {
			break;
		}
		memcpy(dst, src, to_read * sizeof(float));
		ma_pcm_rb_commit_read(&rb, to_read);
		dst += to_read;
		remaining -= to_read;
	}

	// underrun: hold silence
	// TODO: hold last value? with filters smoothing that would hide underruns
	if (remaining > 0) {
		memset(dst, 0, remaining * sizeof(float));
	}

	ma_hpf2_process_pcm_frames(&speaker_hp, out, out, frame_count);
	ma_lpf2_process_pcm_frames(&speaker_lp, out, out, frame_count);
	ma_peak2_process_pcm_frames(&speaker_box, out, out, frame_count);
}

// -----------------------------------------------------------------------
// Returns 1 if name maps to a backend (sets *out), 0 if name means "auto"
// (empty or "auto"), -1 if name is unrecognized.
static int parse_backend(const char *name, ma_backend *out)
{
	if (!name || !*name || !strcasecmp(name, "auto")) return 0;

	static const struct { const char *name; ma_backend be; } map[] = {
		{ "wasapi",     ma_backend_wasapi     },
		{ "dsound",     ma_backend_dsound     },
		{ "winmm",      ma_backend_winmm      },
		{ "coreaudio",  ma_backend_coreaudio  },
		{ "sndio",      ma_backend_sndio      },
		{ "audio4",     ma_backend_audio4     },
		{ "oss",        ma_backend_oss        },
		{ "pulseaudio", ma_backend_pulseaudio },
		{ "pulse",      ma_backend_pulseaudio },
		{ "alsa",       ma_backend_alsa       },
		{ "jack",       ma_backend_jack       },
		{ "aaudio",     ma_backend_aaudio     },
		{ "opensl",     ma_backend_opensl     },
		{ "webaudio",   ma_backend_webaudio   },
		{ "null",       ma_backend_null       },
	};
	for (size_t i=0 ; i<sizeof(map)/sizeof(*map) ; i++) {
		if (!strcasecmp(name, map[i].name)) {
			*out = map[i].be;
			return 1;
		}
	}
	return -1;
}

// -----------------------------------------------------------------------
static bool name_matches(const char *haystack, const char *needle)
{
	size_t nlen = strlen(needle);
	if (nlen == 0) return false;
	for (const char *p=haystack ; *p ; p++) {
		if (strncasecmp(p, needle, nlen) == 0) return true;
	}
	return false;
}

// -----------------------------------------------------------------------
// matched_device_id outlives this call so the returned pointer stays valid
// across ma_device_init (which copies the id internally).
static ma_device_id *resolve_device(const char *needle)
{
	if (!needle || !*needle) return NULL;

	ma_device_info *infos = NULL;
	ma_uint32 count = 0;
	if (ma_context_get_devices(&context, &infos, &count, NULL, NULL) != MA_SUCCESS) {
		LOG(L_LIB, "Sound device enumeration failed; using default");
		return NULL;
	}

	for (ma_uint32 i=0 ; i<count ; i++) {
		if (name_matches(infos[i].name, needle)) {
			matched_device_id = infos[i].id;
			LOG(L_LIB, "Sound device matched: %s", infos[i].name);
			return &matched_device_id;
		}
	}

	LOG(L_LIB, "Sound device '%s' not found on backend %s; using default. Available device names:",
		needle, ma_get_backend_name(context.backend));
	for (ma_uint32 i=0 ; i<count ; i++) {
		LOG(L_LIB, "  - \"%s\"", infos[i].name);
	}
	return NULL;
}

// -----------------------------------------------------------------------
int sound_init(const struct em400_sound_cfg *cfg)
{
	if (initialized) return E_OK;

	ma_backend forced;
	int bres = parse_backend(cfg->backend, &forced);
	if (bres < 0) {
		LOG(L_LIB, "Unknown sound backend '%s'; falling back to auto", cfg->backend);
	}

	ma_result mr;
	if (bres == 1) {
		ma_backend backends[1] = { forced };
		mr = ma_context_init(backends, 1, NULL, &context);
	} else {
		mr = ma_context_init(NULL, 0, NULL, &context);
	}
	if (mr != MA_SUCCESS) {
		if (bres == 1) {
			LOG(L_LIB, "Failed to initialize sound context for backend %s", ma_get_backend_name(forced));
		} else {
			LOG(L_LIB, "Failed to initialize sound context");
		}
		goto fail_context;
	}

	ma_device_config dc = ma_device_config_init(ma_device_type_playback);
	dc.playback.format = ma_format_f32;
	dc.playback.channels = channels;
	dc.playback.pDeviceID = resolve_device(cfg->device);
	dc.sampleRate = cfg->sample_rate;

	int latency_ms = cfg->latency;
	if (latency_ms < SOUND_LATENCY_MIN_MS) {
		LOGWARN("Sound latency of %i ms is too low, using %i ms", latency_ms, SOUND_LATENCY_MIN_MS);
		latency_ms = SOUND_LATENCY_MIN_MS;
	}
	dc.periodSizeInMilliseconds = (ma_uint32)latency_ms / SOUND_PERIODS;
	dc.periods = SOUND_PERIODS;
	dc.dataCallback = data_callback;

	if (ma_device_init(&context, &dc, &device) != MA_SUCCESS) {
		LOG(L_LIB, "Failed to initialize sound playback device");
		goto fail_device;
	}

	// internalPeriodSizeInFrames counts device-rate frames, as set by ma_device_init()
	// em400 counts client-rate ones, and the two differ if miniaudio resamples.
	ma_uint32 period_frames = device.playback.internalPeriodSizeInFrames;
	if ((device.playback.internalSampleRate > 0) && (device.playback.internalSampleRate != device.sampleRate)) {
		period_frames = (ma_uint32)((ma_uint64)period_frames * device.sampleRate / device.playback.internalSampleRate);
	}

	ring_target = SOUND_PERIODS * period_frames;
	if (ring_target < 2 * (ma_uint32)cfg->buffer_len) {
		ring_target = 2 * (ma_uint32)cfg->buffer_len;
	}

	// slack above the target, so ordinary peaks don't trigger the discard
	ring_discard_thr = ring_target + 2 * period_frames;

	ring_frames = ring_discard_thr + 2 * period_frames + 2 * (ma_uint32)cfg->buffer_len;
	if (ring_frames < RING_FRAMES_MIN) {
		ring_frames = RING_FRAMES_MIN;
	}

	if (ma_pcm_rb_init(ma_format_f32, channels, ring_frames, NULL, NULL, &rb) != MA_SUCCESS) {
		LOG(L_LIB, "Failed to initialize sound ring buffer");
		goto fail_rb;
	}
	atomic_store_explicit(&priming, true, memory_order_relaxed);

	// Init before starting the device - the callback may fire immediately.
	ma_hpf2_config hpc = ma_hpf2_config_init(ma_format_f32, channels, cfg->sample_rate, SPEAKER_HP_FREQ, SPEAKER_HP_Q);
	ma_lpf2_config lpc = ma_lpf2_config_init(ma_format_f32, channels, cfg->sample_rate, SPEAKER_LP_FREQ, SPEAKER_LP_Q);
	if (ma_hpf2_init(&hpc, NULL, &speaker_hp) != MA_SUCCESS || ma_lpf2_init(&lpc, NULL, &speaker_lp) != MA_SUCCESS) {
		LOG(L_LIB, "Failed to initialize speaker model filters");
		goto fail_start;
	}
	ma_peak2_config bc = ma_peak2_config_init(ma_format_f32, channels, cfg->sample_rate, BOX_GAIN, BOX_Q, BOX_FREQ);
	if (ma_peak2_init(&bc, NULL, &speaker_box) != MA_SUCCESS) {
		LOG(L_LIB, "Failed to initialize chassis box filter");
		goto fail_start;
	}

	if (ma_device_start(&device) != MA_SUCCESS) {
		LOG(L_LIB, "Failed to start sound playback device");
		goto fail_start;
	}

	initialized = true;
	ma_uint32 cushion_ms = 1000 * ring_target / (ma_uint32)cfg->sample_rate;
	if (cushion_ms > 250) {
		LOGWARN("Sound buffer_len/latency give %u ms of buzzer latency", cushion_ms);
	}
	LOG(L_LIB, "Sound initialized (%s). Rate: %i Hz, period: %u ms x %u, ring: %u frames, cushion: %u frames (%u ms)",
		ma_get_backend_name(context.backend),
		cfg->sample_rate, dc.periodSizeInMilliseconds, dc.periods, ring_frames, ring_target, cushion_ms);
	return E_OK;

fail_start:
	ma_pcm_rb_uninit(&rb);
fail_rb:
	ma_device_uninit(&device);
fail_device:
	ma_context_uninit(&context);
fail_context:
	return E_ERR;
}

// -----------------------------------------------------------------------
void sound_shutdown(void)
{
	if (!initialized) return;

	initialized = false;

	// Device is stopped; the callback won't touch the filters anymore.
	ma_device_uninit(&device);
	ma_hpf2_uninit(&speaker_hp, NULL);
	ma_lpf2_uninit(&speaker_lp, NULL);
	ma_peak2_uninit(&speaker_box, NULL);

	ma_pcm_rb_uninit(&rb);
	ma_context_uninit(&context);
}

// -----------------------------------------------------------------------
long sound_play(float *buf, size_t frames)
{
	if (!initialized) return -1;

	size_t written = 0;
	while (written < frames) {
		ma_uint32 to_write = (ma_uint32)(frames - written);
		void *dst;
		if (ma_pcm_rb_acquire_write(&rb, &to_write, &dst) != MA_SUCCESS) break;
		if (to_write == 0) break;
		memcpy(dst, buf + written, to_write * sizeof(float));
		ma_pcm_rb_commit_write(&rb, to_write);
		written += to_write;
	}
	return (long)written;
}

// -----------------------------------------------------------------------
long sound_ring_fill()
{
	if (!initialized) return -1;

	return ma_pcm_rb_pointer_distance(&rb);
}

// -----------------------------------------------------------------------
long sound_ring_target()
{
	if (!initialized) return -1;

	return ring_target;
}

// -----------------------------------------------------------------------
void sound_start(void)
{
	if (!initialized) return;
	if (ma_device_get_state(&device) == ma_device_state_stopped) {
		atomic_store_explicit(&priming, true, memory_order_relaxed);
		ma_device_start(&device);
	}
}

// -----------------------------------------------------------------------
void sound_stop(void)
{
	if (!initialized) return;
	if (ma_device_get_state(&device) == ma_device_state_started) {
		ma_device_stop(&device);
		ma_pcm_rb_reset(&rb);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
