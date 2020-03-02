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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#include "utils/fdbridge.h"

struct fdpt {
	int type;
	int fd;
	struct sockaddr cliaddr;
	fdb_cb cb;
};

enum fd_types {
	FD_NONE,
	FD_CTL,
	FD_FD,
	FD_TCP,
};

static int fd_ctl[2];

static int pos_count;
static int pos_next;
static struct fdpt *ufds;

static pthread_t loop_thread;
static pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

static void * fdb_loop(void *ptr);

// -----------------------------------------------------------------------
int fdb_init(unsigned fdcount)
{
	if (fdcount < 1) return -1;
	if (pos_count != 0) return -2;

	pos_count = fdcount;

	int res = pipe(fd_ctl);
	if (res) {
		return -3;
	}

	ufds = (struct fdpt *) malloc(pos_count * sizeof(struct fdpt));
	if (!ufds) {
		close(fd_ctl[0]);
		close(fd_ctl[1]);
		return -4;
	}

	// clear all FDs
	for (int i=0 ; i<pos_count ; i++) {
		ufds[i].type = FD_NONE;
	}

	// setup control FD
	ufds[0].fd = fd_ctl[0];
	ufds[0].type = FD_CTL;
	pos_next = 1;

    if (pthread_create(&loop_thread, NULL, fdb_loop, NULL)) {
		close(fd_ctl[0]);
		close(fd_ctl[1]);
		free(ufds);
        return -5;
    }

	return 0;
}

// -----------------------------------------------------------------------
static inline void fdb_wakeup()
{
	if (write(fd_ctl[1], ".", 1))
	;
}

// -----------------------------------------------------------------------
static inline void fdb_quit()
{
	if (write(fd_ctl[1], "q", 1))
	;
}

// -----------------------------------------------------------------------
void fdb_destroy()
{
	// "disable" fd addition
	pthread_mutex_lock(&fd_mutex);
	pos_next = pos_count;
	pthread_mutex_unlock(&fd_mutex);
	// quit the loop
	fdb_quit();
	// wait for the loop to end
	pthread_join(loop_thread, NULL);

	// close user FDs
	for (int i=1 ; i<pos_count ; i++) {
		if (ufds[i].type == FD_TCP) {
			close(ufds[i].fd);
		} else if ((ufds[i].type == FD_FD) && (ufds[i-1].type == FD_TCP)) {
			close(ufds[i].fd);
			ufds[i].type = FD_NONE;
			ufds[i-1].type = FD_NONE;
		}
	}

	// close both sides of the control pipe
	close(fd_ctl[0]);
	close(fd_ctl[1]);

	free(ufds);
}

// -----------------------------------------------------------------------
static int add_fd(int type, int fd, fdb_cb cb, int reserve)
{
	int id = pos_next;

	pthread_mutex_lock(&fd_mutex);

	if (pos_next+reserve-1 < pos_count) {
		ufds[pos_next].type = type;
		ufds[pos_next].fd = fd;
		ufds[pos_next].cb = cb;
		pos_next += reserve;
	} else {
		id = -100;
	}

	pthread_mutex_unlock(&fd_mutex);

	return id;
}

// -----------------------------------------------------------------------
int fdb_add_tcp(uint16_t port, fdb_cb cb)
{
	int res;
	int fd;
	int id = -1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	
	int on = 1;
	res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res < 0) {
		close(fd);
		return -2;
	}
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	res = bind(fd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (res < 0) {
		close(fd);
		return -3;
	}

	res = listen(fd, 1);
	if (res < 0) {
		close(fd);
		return -4;
	}

	if ((id = add_fd(FD_TCP, fd, cb, 2)) > 0) {
		fdb_wakeup();
	}

	return id;
}

// -----------------------------------------------------------------------
int fdb_add_stdin(fdb_cb cb)
{
	int id = -1;

	if ((id = add_fd(FD_FD, 0, cb, 1)) >= 0) {
		fdb_wakeup();
	}

	return id;
}

// -----------------------------------------------------------------------
static int serve_control()
{
	char c;
	if (read(ufds[0].fd, &c, 1) == 1) {
		if (c == 'q') {
			return -1;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
static void serve_conn(int i)
{
	int fd;

	socklen_t clilen = sizeof(ufds[i+1].cliaddr);
	fd = accept(ufds[i].fd, &ufds[i+1].cliaddr, &clilen);

	if (ufds[i+1].type != FD_NONE) {
		// client already connected - reject connection
		const char reject[] = "Endpoint already connected. Bye.\n";
		if (write(fd, reject, strlen(reject)))
		;
		close(fd);
	} else {
		// accept new TCP connection
		ufds[i+1].fd = fd;
		ufds[i+1].type = FD_FD;
		ufds[i+1].cb = ufds[i].cb;
	}
}

// -----------------------------------------------------------------------
static void serve_data(int i)
{
	ufds[i].cb(i, FDB_DATA);
}

// -----------------------------------------------------------------------
static void * fdb_loop(void *ptr)
{
	fd_set rfds;
	int maxfd = fd_ctl[0];

	while (1) {
		// clear FD set
		FD_ZERO(&rfds);

		pthread_mutex_lock(&fd_mutex);
		int lpos_max = pos_next;
		pthread_mutex_unlock(&fd_mutex);

		// add descriptors to FD set
		for (int i=0 ; i<lpos_max; i++) {
			if (ufds[i].type == FD_NONE) continue;
			FD_SET(ufds[i].fd, &rfds);
			if (ufds[i].fd > maxfd) {
				maxfd = ufds[i].fd;
			}
		}

		int retval = select(maxfd+1, &rfds, NULL, NULL, NULL);

		if (retval > 0) {
			// fd needs attention
			for (int i=0 ; i<lpos_max; i++) {
				if ((ufds[i].type != FD_NONE) && FD_ISSET(ufds[i].fd, &rfds)) {
					switch (ufds[i].type) {
						case FD_CTL:
							if (serve_control()) goto loop_quit;
							break;
						case FD_FD:
							serve_data(i);
							break;
						case FD_TCP:
							serve_conn(i);
							break;
						default:
							// action on unknown FD
							break;
					}
				}
			}
		} else if (retval == 0) {
			// timeout - unused
		} else {
			// error
		}
	}

loop_quit:
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int fdb_read(int fdb, char *buf, int count)
{
	return read(ufds[fdb].fd, buf, count);
}

// vim: tabstop=4 shiftwidth=4 autoindent
