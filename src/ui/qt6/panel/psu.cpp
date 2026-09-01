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

#include "psu.h"

#define FADE_MS 1000
#define FADE_TICK_MS 16
// abort the spin-up: if the key goes back to OFF within this window, blend the
// spin-down in over the still-rising spin-up instead of hard-cutting
#define ABORT_WINDOW_MS 1500

// -----------------------------------------------------------------------
Psu::Psu(const QUrl &snd_start_rs, const QUrl &snd_stop_rs, const QUrl &snd_loop_rs, QObject *parent)
	: QObject{parent}
{
	snd_start.setSource(snd_start_rs);
	snd_stop.setSource(snd_stop_rs);
	snd_loop.setSource(snd_loop_rs);
	snd_loop.setLoopCount(QSoundEffect::Infinite);

	fade_timer.setInterval(FADE_TICK_MS);
	connect(&fade_timer, &QTimer::timeout, this, &Psu::fade_step);
	connect(&snd_start, &QSoundEffect::playingChanged, this, &Psu::spin_up_finished);
}

// -----------------------------------------------------------------------
void Psu::set_volume(qreal linear_volume)
{
	volume = linear_volume;
	snd_loop.setVolume(volume);
	if (fade_timer.isActive()) return;
	snd_start.setVolume(volume);
	snd_stop.setVolume(volume);
}

// -----------------------------------------------------------------------
void Psu::set_enabled(bool on)
{
	pending_enabled = on;
}

// -----------------------------------------------------------------------
void Psu::crossfade(QSoundEffect *in, QSoundEffect *out)
{
	fade_in = in;
	fade_out = out;
	fade_out_volume = out->volume();
	chaining = false;
	fade_in->stop();
	fade_in->setVolume(0.0);
	fade_in->play();
	fade_clock.start();
	fade_timer.start();
}

// -----------------------------------------------------------------------
// The spin-up sample ran to its end; hand off to the sustain loop. Guarded so
// our own stop()/restart calls (which also flip isPlaying) never trigger it.
void Psu::spin_up_finished()
{
	if (!chaining || snd_start.isPlaying()) return;
	chaining = false;
	snd_loop.setVolume(volume);
	snd_loop.play();
}

// -----------------------------------------------------------------------
void Psu::fade_step()
{
	// ramp on elapsed time, not on volume-scaled steps: a mid-fade volume
	// change (0 especially) must not stall the ramp
	qreal p = qMin((qreal) 1.0, (qreal) fade_clock.elapsed() / FADE_MS);
	fade_in->setVolume(volume * p);
	fade_out->setVolume(fade_out_volume * (1.0 - p));
	if (p >= 1.0) {
		fade_out->stop();
		fade_timer.stop();
	}
}

// -----------------------------------------------------------------------
void Psu::slot_set_power(bool on)
{
	if (on == running) return;
	running = on;
	if (on) enabled = pending_enabled;
	if (!enabled) return;
	if (on && snd_stop.isPlaying()) {
		// blend the spin-up in over a still-ringing spin-down
		crossfade(&snd_start, &snd_stop);
		start_clock.start();
		chaining = true;
	} else if (on) {
		chaining = false;
		snd_stop.stop();
		snd_start.stop();
		snd_start.setVolume(volume);
		snd_start.play();
		start_clock.start();
		chaining = true;
	} else if (snd_start.isPlaying() && (start_clock.elapsed() < ABORT_WINDOW_MS)) {
		// aborted spin-up: crossfade the spin-down in over it
		chaining = false;
		snd_loop.stop();
		crossfade(&snd_stop, &snd_start);
	} else {
		chaining = false;
		fade_timer.stop();
		snd_start.stop();
		snd_loop.stop();
		snd_stop.stop();
		snd_stop.setVolume(volume);
		snd_stop.play();
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
