//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef TERM_H
#define TERM_H

#include <inttypes.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

enum term_type_e {
	TERM_NONE = 0,
	TERM_TCP = 1,
	TERM_CONSOLE = 2,
	TERM_SERIAL = 3,
};

struct term_t;

struct term_t * term_open_tcp(int port, int timeout_ms);
struct term_t * term_open_console();
void term_close(struct term_t *term);
void term_try_accept(struct term_t *term);
int term_read(struct term_t *term, char *buf, int len);
int term_write(struct term_t *term, char *buf, int len);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
