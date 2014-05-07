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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>

#include "io/term.h"

struct term_t {
	int type;
	int fd_in;
	int fd_out;
	struct timespec timeout;

	int listenfd;
	struct sockaddr cliaddr;
};

// -----------------------------------------------------------------------
struct term_t * term_open_console()
{
	struct term_t *term = malloc(sizeof(struct term_t));
	if (!term) {
		goto fail;
	}

	term->timeout.tv_sec = 0;
	term->timeout.tv_nsec = 100 * 1000 * 1000;
	term->type = TERM_CONSOLE;
	term->fd_in = 0;
	term->fd_out = 1;

	return term;

fail:
	term_close(term);
	return NULL;
}

// -----------------------------------------------------------------------
struct term_t * term_open_tcp(int port, int timeout_ms)
{
	int res;

	struct term_t *term = malloc(sizeof(struct term_t));
	if (!term) {
		goto fail;
	}

	term->timeout.tv_sec = 0;
	term->timeout.tv_nsec = timeout_ms * 1000 * 1000;
	term->type = TERM_TCP;
	term->fd_in = -1;
	term->fd_out = -1;

	term->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (!term->listenfd) {
		goto fail;
	}

	int flags = fcntl(term->listenfd, F_GETFL, 0);
	fcntl(term->listenfd, F_SETFL, flags | O_NONBLOCK);

	int on = 1;
	res = setsockopt(term->listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res < 0) {
		goto fail;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	res = bind(term->listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (res < 0) {
		goto fail;
	}

	res = listen(term->listenfd, 16);
	if (res < 0) {
		goto fail;
	}

	return term;

fail:
	term_close(term);
	return NULL;
}

// -----------------------------------------------------------------------
void term_close(struct term_t *term)
{
	if (term->listenfd > 0) {
		close(term->listenfd);
	}
	if (term->fd_in > 0) {
		close(term->fd_in);
	}
	if (term->fd_out > 0) {
		close(term->fd_out);
	}
	free(term);
}

// -----------------------------------------------------------------------
void term_try_accept(struct term_t *term)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(term->listenfd, &fds);

	int res = pselect(term->listenfd+1, &fds, NULL, NULL, &term->timeout, NULL);

	if (res > 0) {
		socklen_t clilen = sizeof(term->cliaddr);
		term->fd_in = term->fd_out = accept(term->listenfd, &term->cliaddr, &clilen);
	} else if (res < 0) {
		nanosleep(&term->timeout, NULL);
	} else {
		// pselect() timed out
	}
}

// -----------------------------------------------------------------------
int term_read(struct term_t *term, char *buf, int len)
{
	int res;

	if ((term->type == TERM_TCP) && (term->fd_in < 0)) {
		term_try_accept(term);
	}

	if (term->fd_in < 0) {
		return 0;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(term->fd_in, &fds);
	res = pselect(term->fd_in+1, &fds, NULL, NULL, &term->timeout, NULL);
	if (res > 0) {
		res = read(term->fd_in, buf, len);
		if (res > 0) {
			return res;
		} else {
			close(term->fd_in);
			term->fd_in = -1;
			return 0;
		}
	} else if (res < 0) {
		nanosleep(&term->timeout, NULL);
		return 0;
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
int term_write(struct term_t *term, char *buf, int len)
{
	int res;

	if ((term->type == TERM_TCP) && (term->fd_out < 0)) {
		term_try_accept(term);
	}

	if (term->fd_out) {
		res = write(term->fd_out, buf, len);
		if (res >= 0) {
			fsync(term->fd_out);
			return res;
		} else {
			close(term->fd_out);
			term->fd_out = -1;
			return 0;
		}
	} else {
		return 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
