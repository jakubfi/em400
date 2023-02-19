//  Copyright (c) 2019 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "utils/utils.h"
#ifndef _WIN32
#include "utils/serial.h"
#endif

char *out_file;
char *in_file;
int encode = 1;
int progress = 1;
int verbose = 0;
int cont = 0;
int invert = 0;
#ifndef _WIN32
speed_t speed;
#endif

uint16_t w;
uint8_t b[3];

char *name_stdout = "(stdout)";

// -----------------------------------------------------------------------
void error(int e, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\nUse --help to get help on usage.\n");
	va_end(ap);
	exit(e);
}

// -----------------------------------------------------------------------
void print_help()
{
	printf(
		"embin - tool to encode, decode and upload MERA-400 binary streams\n"
		"Usage: embin [options] input\n"
		"Options:\n"
		"  --help                   : print help\n"
		"  --decode|--encode, -d|-e : decode or encode (encode by default)\n"
		"  --output, -o <output>    : output file name, stdout is used if no file name given\n"
#ifndef _WIN32
		"                             output can also be a serial port\n"
		"  --invert, -I             : invert bits in binary streams\n"
		"  --speed, -s <baud>       : baudrate to use when output is a serial port (9600 default)\n"
		"  --no-progress, -n        : don't print progress bar during serial transmission\n"
#endif
		"  --continue, -c           : continue decoding input stream after the first data block has ended.\n"
		"                             this requires a filename specified with -o filename.\n"
		"                             if more blocks are found, they are written in subsequent *_N files\n"
		"  --verbose, -v            : be more verbose\n"
	);
}

// -----------------------------------------------------------------------
void parse_opts(int argc, char **argv)
{
	int opt;
	int idx;

	static struct option opts[] = {
		{ "output",		required_argument,	0, 'o' },
		{ "invert",		no_argument,		0, 'I' },
		{ "decode",		no_argument,		0, 'd' },
		{ "encode",		no_argument,		0, 'e' },
		{ "verbose",	no_argument,		0, 'v' },
#ifndef _WIN32
		{ "speed",		required_argument,	0, 's' },
		{ "no-progress",no_argument,		0, 'n' },
#endif
		{ "continue",	no_argument,		0, 'c' },
		{ "help",		no_argument,		0, 'h' },
		{ 0,			0,					0, 0 }
	};

	while (1) {
#ifdef _WIN32
		opt = getopt_long(argc, argv,"o:Idevch", opts, &idx);
#else
		opt = getopt_long(argc, argv,"o:Idevs:nch", opts, &idx);
#endif
		if (opt == -1) {
			break;
		}
		switch (opt) {
			case 'h':
				print_help();
				exit(0);
				break;
			case 'o':
				out_file = optarg;
				break;
			case 'd':
				encode = 0;
				break;
			case 'e':
				encode = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'I':
				invert = 1;
				break;
#ifndef _WIN32
			case 's':
				speed = serial_int2speed(atoi(optarg));
				if (speed <= 0) {
					error(3, "Cannot set serial port speed: %s", optarg);
				}
				break;
			case 'n':
				progress = 0;
				break;
#endif
			case 'c':
				cont = 1;
				break;
			default:
				exit(1);
				break;
		}
	}

	if (optind >= argc) {
		error(2, "Missing input file name");
	}

	in_file = argv[optind];
}

// -----------------------------------------------------------------------
void print_bar(int total, int done)
{
	static char *bar_filld = "==================================================";
	static char *bar_empty = "                                                  ";

	int percent = done*100 / total;
	fprintf(stderr, "\r%3i%% [%s>%s] %i words",
		done*100 / total,
		bar_filld + 50-(percent>>1),
		bar_empty + (percent>>1),
		done
	);
}

// -----------------------------------------------------------------------
void do_encode(int fi, int fo, int progress)
{
	int res;

	int in_size = lseek(fi, 0L, SEEK_END) / 2;
	lseek(fi, 0L, SEEK_SET);
	int in_read = 0;
	int out_bytes = 0;

	while (1) {
		w = 0;
		res = read(fi, &w, 2);

		if (res == 0) {
			// no more input data, write the ending byte
			if (invert) b[0] = ~BIN_ENDBYTE;
			else b[0] = BIN_ENDBYTE;
			if (write(fo, b, 1) != 1) {
				error(10, "Write failed");
			}
			out_bytes++;
			break;
		}

		in_read++;
		w = ntohs(w);
		word2bin(w, b);
		if (invert) {
			for (int i=0 ; i<3 ; i++) b[i] = ~b[i];
		}
		res = write(fo, b, 3);
		if (res != 3) {
			error(10, "Write failed");
		}
		out_bytes += res;
		if (progress) {
#ifndef _WIN32
			tcdrain(fo);
#endif
			print_bar(in_size, in_read);
		}
	}
	if (progress) fprintf(stderr, "\n");
	if (verbose) {
		fprintf(stderr, "%i words in, %i bytes out.\n", in_read, out_bytes);
	}
}

// -----------------------------------------------------------------------
int do_decode(int fi, int fo)
{
	int ret = 0;
	int res;
	int in_bytes_valid = 0;
	int in_bytes_invalid = 0;
	int out_words = 0;

	int pos = 0;
	while (1) {
		res = read(fi, b+pos, 1);
		if (invert) b[pos] = ~b[pos];
		if (res <= 0) {
			error(10, "Input stream ended prematurely, without the ending byte. Output is most likely invalid.");
			ret = -1;
			break;
		} else if ((pos==0) && bin_is_end(b[pos])) {
			// ending byte on position 0 means stream has ended
			in_bytes_valid++;
			ret = 0;
			break;
		} else if (bin_is_valid(b[pos])) {
			in_bytes_valid++;
			if (pos<2) {
				// collect next word
				pos++;
			} else {
				// word is complete
				pos = 0;
				w = bin2word(b);
				w = ntohs(w);
				res = write(fo, &w, 2);
				if (res != 2) {
					error(10, "Write failed");
				}
				out_words += 1;
			}
		} else {
			// Invalid bytes are silently ignored. This is how CPU works too.
			in_bytes_invalid++;
		}
	}
	if (verbose) {
		fprintf(stderr, "%i valid and %i invalid bytes in, %i words out.\n", in_bytes_valid, in_bytes_invalid, out_words);
	}
	return ret;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int fi, fo;
	int using_serial = 0;

#ifndef _WIN32
	struct stat sb;
	speed = serial_int2speed(9600);
#endif
	parse_opts(argc, argv);

	if (out_file) {
#ifndef _WIN32
		int res = lstat(out_file, &sb);
		// try as a serial port
		if (!res && ((sb.st_mode & S_IFMT) == S_IFCHR) && ((fo = serial_open(out_file, speed)) > 0)) {
			using_serial = 1;
		// if this fails, try as a regular file
		} else
#endif
		{
			fo = open(out_file, O_CREAT|O_WRONLY, 0644);
		}
	} else {
		out_file = name_stdout;
		fo = STDOUT_FILENO;
	}

	if (cont && (encode || using_serial || (out_file == name_stdout))) {
		error(3, "--continue can only be used with --decode and when file is specified as output.\n");
	}

	if (fo < 0) {
		error(2, "Cannot open output file: %s. Error: %s", out_file, strerror(errno));
	}
#ifndef _WIN32
	if (lstat(in_file, &sb)) {
		error(2, "Cannot access input file: %s. Error: %s", in_file, strerror(errno));
	}

	if (S_ISDIR(sb.st_mode)) {
		error(2, "Input %s is a directory.", in_file);
	}
#endif
	fi = open(in_file, O_RDONLY);
	if (fi < 0) {
		error(2, "Cannot open input file: %s. Error: %s", in_file, strerror(errno));
	}

	if (encode) {
		do_encode(fi, fo, using_serial && progress);
	} else {
		if (!cont) {
			do_decode(fi, fo);
		} else {
			int res;
			int seq = 0;
			const int maxlen = strlen(out_file) + 16;
			char *out_file_cont = malloc(maxlen+1);
			snprintf(out_file_cont, maxlen, "%s", out_file);
			do {
				fprintf(stderr, "Decoding data block to: %s\n", out_file_cont);
				res = do_decode(fi, fo);
				close(fo);
				seq++;
				snprintf(out_file_cont, maxlen, "%s_%i", out_file, seq);
				fo = open(out_file_cont, O_CREAT|O_WRONLY, 0644);
			} while (cont && !res);
		}
	}

	close(fi);
	close(fo);

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
