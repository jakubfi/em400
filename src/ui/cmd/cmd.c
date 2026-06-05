//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#define _POSIX_C_SOURCE 200112L
#define __BSD_VISIBLE 1 /* for INADDR_LOOPBACK */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <string.h>

#include "ui/ui.h"
#include "ui/cmd/commands.h"
#include "ui/cmd/utils.h"

#define BUF_MAX (64 * 1024)

struct ui_cmd_data {
	int quit;
	int type;
	int listenfd, fd_in, fd_out;
	FILE *out;
	struct sockaddr cliaddr;
	int tcp_port;
};

enum ui_cmd_type { UI_CMD_STDIO, UI_CMD_TCP };

// accumulates raw input and splits it into newline-terminated command lines
struct line_buf {
	char buf[BUF_MAX+2]; // +1 for '\0' and +1 for flex' '\0'
	int len;             // bytes of a not-yet-terminated line held in buf
	int overflow;        // current line overran buf; discard until next '\n'
};

void ui_cmd_destroy(void *data);

// -----------------------------------------------------------------------
static int __setup_stdio(struct ui_cmd_data *ui)
{
	ui->type = UI_CMD_STDIO;
	ui->fd_in = 0;
	ui->fd_out = dup(1);
	if (ui->fd_out < 0) {
		return -1;
	}
	ui->out = fdopen(ui->fd_out, "w");
	if (!ui->out) {
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
static int __setup_tcp(struct ui_cmd_data *ui)
{
	int res;

	ui->type = UI_CMD_TCP;

	ui->listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	if (!ui->listenfd) {
	   	return 1;
	}

#ifdef _WIN32
	char on = 1;
#else
	int on = 1;
#endif
   	res = setsockopt(ui->listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res < 0) {
   		return 1;
	}

	struct sockaddr_in servaddr;
   	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   	servaddr.sin_port = htons(ui->tcp_port);

	res = bind(ui->listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
   	if (res < 0) {
	   	return 1;
	}

	res = listen(ui->listenfd, 16);
		if (res < 0) {
   		return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
void * ui_cmd_setup(const char *call_name)
{
	int res;

	struct ui_cmd_data *ui = (struct ui_cmd_data *) calloc(1, sizeof(struct ui_cmd_data));
	if (!ui) {
		return NULL;
	}

	ui->fd_in = -1;
	ui->fd_out = -1;
	ui->listenfd = -1;

	const char *arg = strchr(call_name, ':');
	if (arg && *(arg+1)) {
		arg++;
		ui->tcp_port = atoi(arg);
		res = __setup_tcp(ui);
	} else {
		res = __setup_stdio(ui);
	}

	if (!res) {
		return ui;
	} else {
		ui_cmd_destroy(ui);
		return NULL;
	}
}

// -----------------------------------------------------------------------
static bool ui_cmd_process(char *user_input, FILE *out)
{
	char *tok_cmd, *args;

	struct ui_cmd_command *cmd_def = ui_cmd_gettok_cmd(user_input, &tok_cmd, &args);
	if (!cmd_def) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No such command: %s", tok_cmd);
	} else if (cmd_def->fun) {
		cmd_def->fun(out, args);
		if ((cmd_def->flags & UI_CMD_FLAG_QUIT)) return true;
	}
	return false;
}

// -----------------------------------------------------------------------
// Split a freshly-read chunk of `chunk_len` bytes (already stored at
// lb->buf + lb->len) into newline-terminated command lines and dispatch each.
// Any trailing bytes without a '\n' are kept for the next call. Returns true
// if a command asked the UI to quit.
static bool ui_cmd_feed(struct line_buf *lb, int chunk_len, FILE *out)
{
	int end = lb->len + chunk_len;
	int line_start = 0;
	bool quit = false;

	for (int i = lb->len; i < end; i++) {
		if (lb->buf[i] != '\n') continue;
		lb->buf[i] = '\0';
		// the line that just ended had overrun the buffer earlier
		if (lb->overflow) {
			ui_cmd_resp(out, RESP_ERR, UI_EOL, "Input too long (>%i bytes), command ignored", BUF_MAX);
			lb->overflow = 0;
		} else {
			quit = ui_cmd_process(lb->buf + line_start, out);
		}
		fflush(out);
		line_start = i + 1;
		if (quit) break;
	}

	// keep any trailing partial line for the next read()
	lb->len = end - line_start;
	if (lb->len > 0 && line_start > 0) {
		memmove(lb->buf, lb->buf + line_start, lb->len);
	}

	// a partial line filled the whole buffer with no '\n': drop it and flag,
	// so the remainder of the line is ignored up to the next '\n'
	if (lb->len >= BUF_MAX) {
		lb->len = 0;
		lb->overflow = 1;
	}

	return quit;
}

// -----------------------------------------------------------------------
// Accept one TCP connection and set up fd_in / out. Returns false on failure.
static bool ui_cmd_accept(struct ui_cmd_data *ui)
{
	socklen_t clilen = sizeof(ui->cliaddr);
	ui->fd_in = accept(ui->listenfd, &ui->cliaddr, &clilen);
	if (ui->fd_in < 0) return false;
	ui->out = fdopen(ui->fd_in, "w");
	if (!ui->out) {
		close(ui->fd_in);
		ui->fd_in = -1;
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
// Run the command loop on a single connected fd until EOF, error, or quit.
static void ui_cmd_session(struct ui_cmd_data *ui)
{
	struct line_buf lb = {0};

	while (!ui->quit) {
		int read_res = read(ui->fd_in, lb.buf + lb.len, BUF_MAX - lb.len);
		if (read_res <= 0) break; // EOF or error
		ui->quit = ui_cmd_feed(&lb, read_res, ui->out);
	}
}

// -----------------------------------------------------------------------
void ui_cmd_loop(void *data)
{
	struct ui_cmd_data *ui = (struct ui_cmd_data *) data;

	while (!ui->quit) {
		if (ui->type == UI_CMD_TCP && !ui_cmd_accept(ui)) {
			continue;
		}

		ui_cmd_session(ui);

		// connection ended: tear it down
		if (ui->out) {
			fclose(ui->out);
			ui->out = NULL;
		}
		ui->fd_in = -1;
		// stdin can't be reopened, so end the UI loop; for TCP the outer
		// loop goes back to accept() the next connection
		if (ui->type == UI_CMD_STDIO) {
			ui->quit = 1;
		}
	}
}

// -----------------------------------------------------------------------
void ui_cmd_ui_stop(void *data)
{
	struct ui_cmd_data *ui = (struct ui_cmd_data *) data;
	ui->quit = 1;
}

// -----------------------------------------------------------------------
void ui_cmd_destroy(void *data)
{
	struct ui_cmd_data *ui = (struct ui_cmd_data *) data;

	if (!ui) return;

	if (ui->listenfd > 0) {
		close(ui->listenfd);
	}
	if (ui->out) {
		fclose(ui->out);
	}

	free(ui);
}

// -----------------------------------------------------------------------
struct ui_drv ui_cmd = {
	.name = "cmd",
	.setup = ui_cmd_setup,
	.loop = ui_cmd_loop,
	.destroy = ui_cmd_destroy
};

// vim: tabstop=4 shiftwidth=4 autoindent
