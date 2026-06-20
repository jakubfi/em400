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

#ifndef EM400_H
#define EM400_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// App-level machine power, distinct from the library's em400_init/shutdown:
// these build the active machine from appcfg and are idempotent, so the UI can
// drive the ignition (and a reconfigure restart) without double init/shutdown.
int em400_power_on(void);
void em400_power_off(void);
bool em400_is_powered(void);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
