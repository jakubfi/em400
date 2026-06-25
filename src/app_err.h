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

#ifndef APP_ERR_H
#define APP_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

// App-side error reporting. Writes the message to stderr unconditionally (so
// failures are visible even with logging off, which is the default), also logs
// it as a diagnostic line under L_APP via the library, and returns E_ERR so it
// reads as `return app_err(...)`. The stderr write is a placeholder for a future
// UI error handler (cmd -> stderr, qt -> dialog).
int app_errf(const char *func, const char *fmt, ...);
#define app_err(fmt, ...) app_errf(__func__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
