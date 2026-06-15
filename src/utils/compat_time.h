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

#ifndef COMPAT_TIME_H
#define COMPAT_TIME_H

#ifdef _WIN32

#include <time.h>

// Sleep until the absolute CLOCK_MONOTONIC deadline in *deadline.
// Replaces winpthreads clock_nanosleep, which rejects any non-CLOCK_REALTIME
// clock with EINVAL up front (returns immediately without sleeping).
void compat_sleep_until(const struct timespec *deadline);

#endif // _WIN32

#endif // COMPAT_TIME_H

// vim: tabstop=4 shiftwidth=4 autoindent
