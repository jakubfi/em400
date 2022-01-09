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

struct fdb;
enum fdb_conditions { FDB_READY, FDB_LOST };
typedef int (*fdb_cb)(void *user_ctx, int condition);

struct fdb * fdb_open_stdin();
struct fdb * fdb_open_tcp(uint16_t port);
struct fdb * fdb_open_serial(const char *device, int speed);
int fdb_set_callback(struct fdb *fdb, fdb_cb cb, void *user_ctx);
void fdb_set_speed(struct fdb *fdb, int speed);
void fdb_reset(struct fdb *fdb);
void fdb_await_read(struct fdb *fdb);
void fdb_close(struct fdb *fdb);
int fdb_read(struct fdb *fdb);
int fdb_write(struct fdb *fdb, unsigned char c);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
