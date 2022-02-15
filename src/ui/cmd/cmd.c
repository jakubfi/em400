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
#include <sys/socket.h>
#include <arpa/inet.h>
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

void ui_cmd_destroy(void *data);

// -----------------------------------------------------------------------
static int __setup_stdio(struct ui_cmd_data *ui)
{
	ui->type = UI_CMD_STDIO;
	ui->fd_in = 0;
   	ui->fd_out = 1;
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

	int on = 1;
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
void ui_cmd_loop(void *data)
{
	struct ui_cmd_data *ui = (struct ui_cmd_data *) data;

	char buf[BUF_MAX+2]; // +1 for '\0' and +1 for flex' '\0'
	int recvd = 0;
	int input_invalid = 0;

	while (!ui->quit) {

		if (ui->type == UI_CMD_TCP) {
			// accept connection
			socklen_t clilen = sizeof(ui->cliaddr);
			ui->fd_in = accept(ui->listenfd, &ui->cliaddr, &clilen);
			if (ui->fd_in < 0) continue;
			ui->out = fdopen(ui->fd_in, "w");
			if (!ui->out) {
				close(ui->fd_in);
				continue;
			}
		}

		while (!ui->quit) {

			// buffer overflow
			if (recvd >= BUF_MAX) {
				recvd = 0;
				input_invalid = 1;
			// collect input
			} else {
				int read_res = read(ui->fd_in, buf+recvd, BUF_MAX-recvd);
				if (read_res > 0) {
					// input not done yet
					if (*(buf+recvd+read_res-1) != '\n') {
						recvd += read_res;
					// input done
					} else {
						// there was a buffer overflow in the meantime
						if (input_invalid) {
							ui_cmd_resp(ui->out, RESP_ERR, UI_EOL, "Input too long (>%i bytes), command ignored", BUF_MAX);
							fflush(ui->out);
							input_invalid = 0;
						// valid input
						} else {
							*(buf+recvd+read_res-1) = '\0';
							recvd = 0;
							ui->quit = ui_cmd_process(buf, ui->out);
							fflush(ui->out);
						}
					}
				// EOF
				} else {
					recvd = 0;
					fclose(ui->out);
					ui->out = NULL;
					ui->fd_in = -1;
					break;
				}
			}
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
	.stop = ui_cmd_ui_stop,
	.destroy = ui_cmd_destroy
};

// vim: tabstop=4 shiftwidth=4 autoindent
