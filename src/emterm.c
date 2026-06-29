//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <termios.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define HOST "127.0.0.1"
#define BUF_SIZE 4096

static struct termios tio_saved;
static bool tio_raw = false;

// -----------------------------------------------------------------------
static void tty_restore()
{
	if (tio_raw) {
		tcsetattr(STDIN_FILENO, TCSANOW, &tio_saved);
		tio_raw = false;
	}
}

// -----------------------------------------------------------------------
static void on_signal(int sig)
{
	(void) sig;
	tty_restore();
	_exit(1);
}

// -----------------------------------------------------------------------
static int tty_raw(bool no_echo)
{
	if (!isatty(STDIN_FILENO)) return 0;

	if (tcgetattr(STDIN_FILENO, &tio_saved) < 0) {
		return -1;
	}

	struct termios tio = tio_saved;
	tio.c_lflag &= ~(ICANON | ECHOCTL);
	if (no_echo) {
		tio.c_lflag &= ~ECHO;
	}
	tio.c_iflag &= ~(ICRNL);
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &tio) < 0) {
		return -1;
	}
	tio_raw = true;
	return 0;
}

// -----------------------------------------------------------------------
static int tcp_connect(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};
	if (inet_pton(AF_INET, HOST, &addr.sin_addr) != 1) {
		close(fd);
		return -1;
	}

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

// -----------------------------------------------------------------------
static int pump(int from, int to)
{
	char buf[BUF_SIZE];
	ssize_t n = read(from, buf, sizeof(buf));
	if (n <= 0) {
		return -1;
	}

	ssize_t off = 0;
	while (off < n) {
		ssize_t w = write(to, buf + off, n - off);
		if (w < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		off += w;
	}
	return 0;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	static const struct option opts[] = {
		{ "no-echo", no_argument, NULL, 'n' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	bool no_echo = false;
	int c;
	while ((c = getopt_long(argc, argv, "nh", opts, NULL)) != -1) {
		switch (c) {
			case 'n':
				no_echo = true;
				break;
			case 'h':
				printf("Usage: %s [--no-echo] <port>\n", argv[0]);
				return 0;
			default:
				return 1;
		}
	}

	if (optind != argc - 1) {
		fprintf(stderr, "Usage: %s [--no-echo] <port>\n", argv[0]);
		return 1;
	}

	int port = atoi(argv[optind]);
	if ((port <= 0) || (port > 65535)) {
		fprintf(stderr, "Invalid port: %s\n", argv[optind]);
		return 1;
	}

	int sock = tcp_connect(port);
	if (sock < 0) {
		fprintf(stderr, "Cannot connect to %s:%d: %s\n", HOST, port, strerror(errno));
		return 1;
	}

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);
	signal(SIGHUP, on_signal);

	if (tty_raw(no_echo) < 0) {
		fprintf(stderr, "Cannot set terminal mode: %s\n", strerror(errno));
		close(sock);
		return 1;
	}

	struct pollfd fds[2] = {
		{ .fd = STDIN_FILENO, .events = POLLIN },
		{ .fd = sock, .events = POLLIN },
	};

	int rc = 0;
	while (1) {
		if (poll(fds, 2, -1) < 0) {
			if (errno == EINTR) continue;
			rc = 1;
			break;
		}
		if (fds[0].revents & POLLIN) {
			if (pump(STDIN_FILENO, sock) < 0) break;
		}
		if (fds[1].revents & POLLIN) {
			if (pump(sock, STDOUT_FILENO) < 0) break;
		}
		if ((fds[0].revents | fds[1].revents) & (POLLERR | POLLHUP)) break;
	}

	tty_restore();
	close(sock);
	return rc;
}

// vim: tabstop=4 shiftwidth=4 autoindent
