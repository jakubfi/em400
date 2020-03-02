//  Copyright (c) 2018 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef FDBRIDGE_H
#define FDBRIDGE_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

enum fdb_conditions { FDB_DATA, FDB_CLOSE, FDB_ERROR };

typedef int (*fdb_cb)(int fdb, int condition);

int fdb_init(unsigned fdcount);
void fdb_destroy();

int fdb_add_stdin(fdb_cb cb);
int fdb_add_tcp(uint16_t port, fdb_cb cb);

int fdb_read(int fdb, char *buf, int count);
int fdb_write(int fdb, char c);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
