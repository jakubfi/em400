//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#include "log.h"
#include "io/dev/dev.h"
#include "external/iniparser/iniparser.h"

enum dev_terminal_commands { CMD_NONE, CMD_RD, CMD_WR, CMD_QUIT };
enum dev_terminal_type { TERM_CONSOLE, TERM_TCP };

struct dev_terminal {
	int type;
	int fd_in;
	int fd_out;
	struct timespec timeout;

	int listenfd;
	struct sockaddr cliaddr;

	pthread_t th;
	pthread_cond_t cmd_cond;
	pthread_mutex_t cmd_mutex;
	int cmd;
};

void dev_terminal_destroy(void *dev);
void *dev_terminal_controller(void *ptr);

// -----------------------------------------------------------------------
int dev_terminal_open_console(struct dev_terminal *terminal)
{
	terminal->timeout.tv_sec = 0;
	terminal->timeout.tv_nsec = 100 * 1000 * 1000;
	terminal->type = TERM_CONSOLE;
	terminal->fd_in = 0;
	terminal->fd_out = 1;

	return 0;
}

// -----------------------------------------------------------------------
int dev_terminal_open_tcp(struct dev_terminal *terminal, int port, int timeout_ms)
{
	int res;

	terminal->timeout.tv_sec = 0;
	terminal->timeout.tv_nsec = timeout_ms * 1000 * 1000;
	terminal->type = TERM_TCP;
	terminal->fd_in = -1;
	terminal->fd_out = -1;

	terminal->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (!terminal->listenfd) {
		return 1;
	}

	int flags = fcntl(terminal->listenfd, F_GETFL, 0);
	fcntl(terminal->listenfd, F_SETFL, flags | O_NONBLOCK);

	int on = 1;
	res = setsockopt(terminal->listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res < 0) {
		return 1;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	res = bind(terminal->listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (res < 0) {
		return 1;
	}

	res = listen(terminal->listenfd, 16);
	if (res < 0) {
		return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
void * dev_terminal_create(dictionary *cfg, const char *section)
{
	struct dev_terminal *terminal = (struct dev_terminal *) malloc(sizeof(struct dev_terminal));
	if (!terminal) {
		goto cleanup;
	}

	char key[32];
	sprintf(key, "%s:transport", section);
	const char *transport = iniparser_getstring(cfg, key, NULL);

	if (!strcasecmp(transport, "tcp")) {
		sprintf(key, "%s:port", section);
		const int port = iniparser_getint(cfg, key, -1);
		if (dev_terminal_open_tcp(terminal, port, 100)) {
			LOGERR("Failed to open TCP terminal on port: %i.", port);
			goto cleanup;
		}
	} else if (!strcasecmp(transport, "console")) {
		if (dev_terminal_open_console(terminal)) {
			LOGERR("Failed to open console terminal.");
			goto cleanup;
		}
	} else {
		LOGERR("Unknown terminal transport: %s.", transport);
		goto cleanup;
	}

	terminal->cmd = CMD_NONE;

	if (pthread_mutex_init(&terminal->cmd_mutex, NULL)) {
		LOGERR("Failed to initialize terminal mutex.");
		goto cleanup;
	}

	if (pthread_cond_init(&terminal->cmd_cond, NULL)) {
		LOGERR("Failed to initialize terminal cmd conditional.");
		goto cleanup;
	}

	if (pthread_create(&terminal->th, NULL, dev_terminal_controller, terminal)) {
		LOGERR("Failed to spawn terminal thread.");
		goto cleanup;
	}

	pthread_setname_np(terminal->th, "term");

	return terminal;

cleanup:
	dev_terminal_destroy(terminal);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_terminal_destroy(void *dev)
{
	if (!dev) return;

	struct dev_terminal *terminal = (struct dev_terminal *) dev;
	if (terminal->th) {
		pthread_mutex_lock(&terminal->cmd_mutex);
		terminal->cmd = CMD_QUIT;
		pthread_cond_signal(&terminal->cmd_cond);
		pthread_mutex_unlock(&terminal->cmd_mutex);
		pthread_join(terminal->th, NULL);
	}

	if (terminal->listenfd > 0) {
		close(terminal->listenfd);
	}
	if (terminal->fd_in > 0) {
		close(terminal->fd_in);
	}
	if (terminal->fd_out > 0) {
		close(terminal->fd_out);
	}

	free(terminal);
}

// -----------------------------------------------------------------------
void dev_terminal_reset(void *dev)
{

}
// -----------------------------------------------------------------------
static void dev_terminal_try_accept(struct dev_terminal *terminal)
{
	if ((terminal->type == TERM_TCP) && (terminal->fd_in < 0)) {

		socklen_t clilen = sizeof(terminal->cliaddr);
		terminal->fd_in = terminal->fd_out = accept(terminal->listenfd, &terminal->cliaddr, &clilen);
	}
}

// -----------------------------------------------------------------------
void *dev_terminal_controller(void *ptr)
{
	struct dev_terminal *terminal = (struct dev_terminal *) ptr;
	struct timespec abstime;
	int res;

	pthread_mutex_lock(&terminal->cmd_mutex);

	while (terminal->cmd != CMD_QUIT) {

		clock_gettime(CLOCK_REALTIME, &abstime);
		long new_nsec = abstime.tv_nsec + 100 * 1000000L;
		abstime.tv_sec += new_nsec / 1000000000L;
		abstime.tv_nsec = new_nsec % 1000000000L;

		res = 0;

		while ((terminal->cmd == CMD_NONE) && (res != ETIMEDOUT)) {
			res = pthread_cond_timedwait(&terminal->cmd_cond, &terminal->cmd_mutex, &abstime);
		}

		dev_terminal_try_accept(terminal);

		switch (terminal->cmd) {
			case CMD_RD:
				terminal->cmd = CMD_NONE;
				break;
			case CMD_WR:
				terminal->cmd = CMD_NONE;
				break;
			default:
				break;
		}
	}

	pthread_mutex_unlock(&terminal->cmd_mutex);

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
static int dev_terminal_cmd(struct dev_terminal *terminal, int cmd)
{
	if (pthread_mutex_trylock(&terminal->cmd_mutex)) {
		return DEV_CMD_BUSY;
	}

	if (terminal->cmd != CMD_NONE) {
		pthread_mutex_unlock(&terminal->cmd_mutex);
		return DEV_CMD_BUSY;
	}

	terminal->cmd = cmd;
	pthread_cond_signal(&terminal->cmd_cond);
	pthread_mutex_unlock(&terminal->cmd_mutex);

	return DEV_CMD_OK;
}

// -----------------------------------------------------------------------
int dev_terminal_read(void *dev, uint8_t *c)
{
	struct dev_terminal *terminal = (struct dev_terminal *) dev;
	return dev_terminal_cmd(terminal, CMD_RD);
}

// -----------------------------------------------------------------------
int dev_terminal_write(void *dev, uint8_t *c)
{
	struct dev_terminal *terminal = (struct dev_terminal *) dev;
	return dev_terminal_cmd(terminal, CMD_WR);
}

struct dev_drv dev_terminal = {
	.name = "terminal",
	.create = dev_terminal_create,
	.destroy = dev_terminal_destroy,
	.reset = dev_terminal_reset,
	.sector_rd = NULL,
	.sector_wr = NULL,
	.char_rd = dev_terminal_read,
	.char_wr = dev_terminal_write,
};


// vim: tabstop=4 shiftwidth=4 autoindent
