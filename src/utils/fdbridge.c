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

#include "utils/elst.h"
#include "utils/fdbridge.h"

#define BUFSIZE 1024

struct fdpt {
	int type;
	int fd;
	struct sockaddr cliaddr;
	ELST q;
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
static char buf[BUFSIZE];

static pthread_t loop_thread;
static pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

static void * __fdbridge_loop(void *ptr);

// -----------------------------------------------------------------------
static struct fdbridge_event * __ev_new(int type, int sender, int len, void *ptr)
{
	struct fdbridge_event *e = malloc(sizeof(struct fdbridge_event));
	if (!e) {
		return NULL;
	}
	e->type = type;
	e->sender = sender;
	e->len = len;
	if (ptr && len) {
		e->ptr = malloc(len);
		if (!e->ptr) {
			free(e);
			return NULL;
		}
		memcpy(e->ptr, ptr, len);
	} else {
		e->ptr = NULL;
	}
	return e;
}

// -----------------------------------------------------------------------
void fdbridge_ev_free(struct fdbridge_event *e)
{
	if (!e) return;
	free(e->ptr);
	free(e);
}

// -----------------------------------------------------------------------
int fdbridge_init(unsigned fdcount)
{
	if (fdcount < 1) return -1;
	if (pos_count != 0) return -2;

	pos_count = fdcount;

	int res = pipe(fd_ctl);
	if (res) {
		return -3;
	}

	ufds = malloc(pos_count * sizeof(struct fdpt));
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

    if (pthread_create(&loop_thread, NULL, __fdbridge_loop, NULL)) {
		close(fd_ctl[0]);
		close(fd_ctl[1]);
		free(ufds);
        return -5;
    }

	return 0;
}

// -----------------------------------------------------------------------
static inline void __fdbridge_wakeup()
{
	if (write(fd_ctl[1], ".", 1));
}

// -----------------------------------------------------------------------
static inline void __fdbridge_quit()
{
	if (write(fd_ctl[1], "q", 1));
}

// -----------------------------------------------------------------------
void fdbridge_destroy()
{
	// "disable" fd addition
	pthread_mutex_lock(&fd_mutex);
	pos_next = pos_count;
	pthread_mutex_unlock(&fd_mutex);
	// quit the loop
	__fdbridge_quit();
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
static int __add_fd(int type, int fd, ELST q, int reserve)
{
	int id = pos_next;

	pthread_mutex_lock(&fd_mutex);

	if (pos_next+reserve-1 < pos_count) {
		ufds[pos_next].type = type;
		ufds[pos_next].fd = fd;
		ufds[pos_next].q = q;
		pos_next += reserve;
	} else {
		id = -100;
	}

	pthread_mutex_unlock(&fd_mutex);

	return id;
}

// -----------------------------------------------------------------------
int fdbridge_add_tcp(uint16_t port, ELST q)
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

	if ((id = __add_fd(FD_TCP, fd, q, 2)) > 0) {
		__fdbridge_wakeup();
	}

	return id;
}

// -----------------------------------------------------------------------
int fdbridge_add_stdin(ELST q)
{
	int id = -1;

	if ((id = __add_fd(FD_FD, 0, q, 1)) >= 0) {
		__fdbridge_wakeup();
	}

	return id;
}

// -----------------------------------------------------------------------
static int __serve_control()
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
static void __serve_conn(int i)
{
	int fd;
	struct fdbridge_event *e;

	socklen_t clilen = sizeof(ufds[i+1].cliaddr);
	fd = accept(ufds[i].fd, &ufds[i+1].cliaddr, &clilen);

	if (ufds[i+1].type != FD_NONE) {
		// client already connected - reject connection
		const char reject[] = "Endpoint already connected. Bye.\n";
		if (write(fd, reject, strlen(reject)));
		close(fd);
	} else {
		// accept new TCP connection
		ufds[i+1].fd = fd;
		ufds[i+1].type = FD_FD;
		ufds[i+1].q = ufds[i].q;

		e = __ev_new(EV_CONNECT, i, 0, NULL);
		elst_append(ufds[i].q, e);
	}
}

// -----------------------------------------------------------------------
static void __serve_data(int i)
{
	struct fdbridge_event *e = NULL;

	int res = read(ufds[i].fd, buf, BUFSIZE);
	if (res > 0) {
		// data
		e = __ev_new(EV_DATA, i, res, buf);
	} else if (res == 0) {
		// eof
		if ((i > 1) && (ufds[i-1].type == FD_TCP)) {
			// eof on tcp connection - close client connection
			close(ufds[i].fd);
			ufds[i].type = FD_NONE;
		}
		e = __ev_new(EV_EOF, i, 0, NULL);
	} else {
		// error
		// TODO: send the reason
		e = __ev_new(EV_ERROR, i, 0, NULL);
	}

	if (e) elst_append(ufds[i].q, e);
}

// -----------------------------------------------------------------------
static void * __fdbridge_loop(void *ptr)
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
							if (__serve_control()) goto loop_quit;
							break;
						case FD_FD:
							__serve_data(i);
							break;
						case FD_TCP:
							__serve_conn(i);
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

// vim: tabstop=4 shiftwidth=4 autoindent
