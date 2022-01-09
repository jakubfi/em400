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

#define _GNU_SOURCE

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
#include <semaphore.h>
#include <errno.h>

#include "log.h"
#include "utils/utils.h"
#include "io/dev/fdbridge.h"

#define FDB_BUF_SIZE 64

enum fdb_fds {
	FDB_FD_CTL_OUT,
	FDB_FD_CTL_IN,
	FDB_FD_FD,
	FDB_FD_LISTEN,
	FDB_FD_COUNT
};

struct fdb {
	char rdbuf[FDB_BUF_SIZE];
	char *rdbuf_r;
	char *rdbuf_w;
	char *rdbuf_end;
	int rdbuf_count;
	int wrbuf;
	int type;
	int sleep_us;
	int fds[FDB_FD_COUNT];
	struct sockaddr cliaddr;
	fdb_cb cb;
	void *user_ctx;
	pthread_t loop_thread;
	pthread_mutex_t data_mutex;
	int awaiting_read;
	int awaiting_write;
};

enum fd_types {
	FD_SERIAL,
	FD_TCP,
	FD_FD,
};

static void * fdb_loop(void *ptr);

// -----------------------------------------------------------------------
int buf_get(struct fdb *fdb)
{
	int data;

	pthread_mutex_lock(&fdb->data_mutex);
	if (fdb->rdbuf_count <= 0) {
		data = -1;
		fdb->awaiting_read = 1;
	} else {
		fdb->awaiting_read = 0;
		data = (unsigned char) *fdb->rdbuf_r;
		fdb->rdbuf_r++;
		fdb->rdbuf_count--;
		if (fdb->rdbuf_r > fdb->rdbuf_end) {
			fdb->rdbuf_r = fdb->rdbuf;
		}
	}
	pthread_mutex_unlock(&fdb->data_mutex);

	return data;
}

// -----------------------------------------------------------------------
int buf_append(struct fdb *fdb, char c)
{
	int ret = 0;
	int awaiting_read = 0;

	pthread_mutex_lock(&fdb->data_mutex);
	if (fdb->rdbuf_count >= FDB_BUF_SIZE) {
		ret = -1;
	} else {
		*fdb->rdbuf_w = c;
		fdb->rdbuf_w++;
		fdb->rdbuf_count++;
		if (fdb->rdbuf_w > fdb->rdbuf_end) {
			fdb->rdbuf_w = fdb->rdbuf;
		}
		awaiting_read = fdb->awaiting_read;
	}
	pthread_mutex_unlock(&fdb->data_mutex);

	if (fdb->rdbuf_count > 1) {
		LOG(L_FDBR, "write buffer fill: %i\n", fdb->rdbuf_count);
	}

	if (awaiting_read) {
		fdb->cb(fdb->user_ctx, FDB_READY);
	}

	return ret;
}

// -----------------------------------------------------------------------
void fdb_quit(struct fdb *fdb)
{
	if (write(fdb->fds[FDB_FD_CTL_IN], "q", 1))
	;
}

// -----------------------------------------------------------------------
void fdb_reset(struct fdb *fdb)
{
	pthread_mutex_lock(&fdb->data_mutex);
	fdb->wrbuf = -1;
	fdb->awaiting_read = 0;
	fdb->awaiting_write = 0;
	fdb->rdbuf_r = fdb->rdbuf_w = fdb->rdbuf;
	pthread_mutex_unlock(&fdb->data_mutex);
}

// -----------------------------------------------------------------------
void fdb_await_read(struct fdb *fdb)
{
	pthread_mutex_lock(&fdb->data_mutex);
	fdb->awaiting_read = 1;
	pthread_mutex_unlock(&fdb->data_mutex);
}

// -----------------------------------------------------------------------
struct fdb * fdb_new(int type)
{
	struct fdb *fdb = calloc(1, sizeof(struct fdb));
	if (!fdb) return NULL;
	for (int i=0 ; i<FDB_FD_COUNT ; i++) {
		fdb->fds[i] = -1;
	}
	fdb->type = type;
	fdb->rdbuf_end = fdb->rdbuf + FDB_BUF_SIZE-1;
	fdb_reset(fdb);

	return fdb;
}

// -----------------------------------------------------------------------
void fdb_close(struct fdb *fdb)
{
	// quit the loop
	fdb_quit(fdb);
	pthread_join(fdb->loop_thread, NULL);

	for (int i=0 ; i<FDB_FD_COUNT ; i++) {
		if (fdb->fds[i] != -1) {
			close(fdb->fds[i]);
		}
	}
	free(fdb);
}

// -----------------------------------------------------------------------
int fdb_set_callback(struct fdb *fdb, fdb_cb cb, void *user_ctx)
{
	if (!fdb) return -1;
	fdb->cb = cb;
	fdb->user_ctx = user_ctx;
	return 0;
}

// -----------------------------------------------------------------------
void fdb_set_speed(struct fdb *fdb, int speed)
{
	fdb->sleep_us = 10L * 1000 * 1000 / speed;
}

// -----------------------------------------------------------------------
int fdb_manage(struct fdb *fdb)
{
	int res = pipe(fdb->fds);
	if (res) {
		return -1;
	}

	if (pthread_create(&fdb->loop_thread, NULL, fdb_loop, fdb)) {
		close(fdb->fds[FDB_FD_CTL_IN]);
		close(fdb->fds[FDB_FD_CTL_OUT]);
		return -2;
	}

	pthread_setname_np(fdb->loop_thread, "fdbr");

	return 0;
}

// -----------------------------------------------------------------------
struct fdb * fdb_open_serial(const char *device, int speed)
{
	struct fdb *fdb = fdb_new(FD_SERIAL);
	if (!fdb) return NULL;

	fdb->fds[FDB_FD_FD] = serial_open(device, serial_int2speed(speed));
	if (fdb->fds[FDB_FD_FD] < 0) {
		free(fdb);
		return NULL;
	}

	if (fdb_manage(fdb)) {
		close(fdb->fds[FDB_FD_FD]);
		free(fdb);
		return NULL;
	}

	return fdb;
}

// -----------------------------------------------------------------------
struct fdb * fdb_open_tcp(uint16_t port)
{
	struct fdb *fdb = fdb_new(FD_TCP);
	if (!fdb) return NULL;

	int res;

	fdb->fds[FDB_FD_LISTEN] = socket(AF_INET, SOCK_STREAM, 0);
	if (fdb->fds[FDB_FD_LISTEN] < 0) {
		return NULL;
	}

	int flags = fcntl(fdb->fds[FDB_FD_LISTEN], F_GETFL, 0);
	fcntl(fdb->fds[FDB_FD_LISTEN], F_SETFL, flags | O_NONBLOCK);
	
	int on = 1;
	res = setsockopt(fdb->fds[FDB_FD_LISTEN], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res < 0) {
		close(fdb->fds[FDB_FD_LISTEN]);
		return NULL;
	}
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	res = bind(fdb->fds[FDB_FD_LISTEN], (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (res < 0) {
		close(fdb->fds[FDB_FD_LISTEN]);
		return NULL;
	}

	res = listen(fdb->fds[FDB_FD_LISTEN], 1);
	if (res < 0) {
		close(fdb->fds[FDB_FD_LISTEN]);
		return NULL;
	}

	if (fdb_manage(fdb)) {
		close(fdb->fds[FDB_FD_LISTEN]);
		free(fdb);
		return NULL;
	}

	return fdb;
}

// -----------------------------------------------------------------------
struct fdb * fdb_open_stdin()
{
	struct fdb *fdb = fdb_new(FD_FD);
	if (!fdb) return NULL;

	fdb->fds[FDB_FD_FD] = 0;

	if (fdb_manage(fdb)) {
		free(fdb);
		return NULL;
	}

	return fdb;
}

// -----------------------------------------------------------------------
static int serve_control(struct fdb *fdb)
{
	char c;
	unsigned char data;
	if (read(fdb->fds[FDB_FD_CTL_OUT], &c, 1) == 1) {
		switch (c) {
			case 'q':
				return -1;
			case 'w':
				pthread_mutex_lock(&fdb->data_mutex);
				data = fdb->wrbuf;
				pthread_mutex_unlock(&fdb->data_mutex);
				if (fdb->fds[FDB_FD_FD] >= 0) {
					if (write(fdb->fds[FDB_FD_FD], &data, 1))
					;
					LOG(L_FDBR, "transmitting data: %i (#%02x)", data, data);
				} else {
					LOG(L_FDBR, "no client connected, data lost: %i (#%02x)", data, data);
				}
				if (fdb->sleep_us > 0) {
					usleep(fdb->sleep_us);
				}
				pthread_mutex_lock(&fdb->data_mutex);
				fdb->wrbuf = -1;
				int awaiting_write = fdb->awaiting_write;
				pthread_mutex_unlock(&fdb->data_mutex);
				LOG(L_FDBR, "data transmitted");
				if (awaiting_write) {
					fdb->cb(fdb->user_ctx, FDB_READY);
				}
				break;
			default:
				break;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
static void serve_conn(struct fdb *fdb)
{
	socklen_t clilen = sizeof(fdb->cliaddr);
	int fd = accept(fdb->fds[FDB_FD_LISTEN], &fdb->cliaddr, &clilen);

	if (fdb->fds[FDB_FD_FD] >= 0) {
		// client already connected - reject connection
		const char reject[] = "Endpoint already connected. Bye.\n";
		if (write(fd, reject, strlen(reject)))
		;
		close(fd);
	} else {
		// accept new TCP connection
		fdb->fds[FDB_FD_FD] = fd;
	}
}

// -----------------------------------------------------------------------
static void serve_data(struct fdb *fdb)
{
	unsigned char data;
	int res;

	res = read(fdb->fds[FDB_FD_FD], &data, 1);
	if (res == 0) {
		LOG(L_FDBR, "client disconnected, empty read");
		close(fdb->fds[FDB_FD_FD]);
		fdb->fds[FDB_FD_FD] = -2;
	} else {
		LOG(L_FDBR, "receiver active");
		if (fdb->sleep_us > 0) {
			usleep(fdb->sleep_us);
		}
		if (!buf_append(fdb, data)) {
			LOG(L_FDBR, "data ready: %i (#%02x)", data, data);
		} else {
			fdb->cb(fdb->user_ctx, FDB_LOST);
		}
	}
}

// -----------------------------------------------------------------------
static void * fdb_loop(void *ptr)
{
	struct fdb *fdb = (struct fdb *) ptr;
	fd_set rfds;
	int maxfd = 0;

	while (1) {
		FD_ZERO(&rfds);
		for (int i=0 ; i<FDB_FD_COUNT ; i++) {
			if ((fdb->fds[i] != -1) && (i != FDB_FD_CTL_IN)) {
				FD_SET(fdb->fds[i], &rfds);
				if (fdb->fds[i] > maxfd) maxfd = fdb->fds[i];
			}
		}

		int retval = select(maxfd+1, &rfds, NULL, NULL, NULL);
		if (retval > 0) {
			// fd needs attention
			for (int i=FDB_FD_CTL_OUT ; i<FDB_FD_COUNT ; i++) {
				if ((fdb->fds[i] != -1) && FD_ISSET(fdb->fds[i], &rfds)) {
					switch (i) {
						case FDB_FD_CTL_OUT:
							if (serve_control(fdb)) {
								goto loop_quit;
							}
							break;
						case FDB_FD_FD:
							serve_data(fdb);
							break;
						case FDB_FD_LISTEN:
							serve_conn(fdb);
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
int fdb_read(struct fdb *fdb)
{
	return buf_get(fdb);
}

// -----------------------------------------------------------------------
int fdb_write(struct fdb *fdb, unsigned char c)
{
	pthread_mutex_lock(&fdb->data_mutex);
	fdb->awaiting_read = 0;
	if (fdb->wrbuf >= 0) {
		fdb->awaiting_write = 1;
		pthread_mutex_unlock(&fdb->data_mutex);
		return -1;
	} else {
		fdb->awaiting_write = 0;
		fdb->wrbuf = c;
		pthread_mutex_unlock(&fdb->data_mutex);
		int res = write(fdb->fds[FDB_FD_CTL_IN], "w", 1);
		if (res != 1) {
			return 0;
		} else {
			return 0;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
