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

#ifndef PSU_H
#define PSU_H

#include <QObject>
#include <QUrl>
#include <QTimer>
#include <QElapsedTimer>
#include <QtMultimedia/QSoundEffect>

class Psu : public QObject
{
	Q_OBJECT

private:
	QSoundEffect snd_start, snd_stop, snd_loop;
	QTimer fade_timer;
	QElapsedTimer start_clock, fade_clock;
	QSoundEffect *fade_in = nullptr, *fade_out = nullptr;
	qreal fade_out_volume = 0.0;
	qreal volume = 1.0;
	bool running = false;
	bool enabled = false;
	bool pending_enabled = false;
	// a spin-up is in progress and should hand off to the sustain loop once it
	// finishes; cleared before any deliberate stop so restarts don't trigger it
	bool chaining = false;

	void crossfade(QSoundEffect *in, QSoundEffect *out);

public:
	explicit Psu(const QUrl &snd_start_rs, const QUrl &snd_stop_rs, const QUrl &snd_loop_rs, QObject *parent = nullptr);
	void set_volume(qreal linear_volume);
	void set_enabled(bool on);

public slots:
	void slot_set_power(bool on);

private slots:
	void fade_step();
	void spin_up_finished();
};

#endif // PSU_H

// vim: tabstop=4 shiftwidth=4 autoindent
