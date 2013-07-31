//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <stdarg.h>

#include "e4image.h"

enum flags_e {
	FCLR = 0,
	FSET = 1,
};

#define BAD_FLAG 0b11111111111111111111111111111111

static struct option opts[] = {
	{ "image",		required_argument,	0, 'i' },
	{ "preset",		required_argument,	0, 'p' },
	{ "src",		required_argument,	0, 'r' },
	{ "dup",		required_argument,	0, 'd' },
	{ "nomaster",	no_argument,		0, 'n' },
	{ "blocks",		required_argument,	0, 'b' },
	{ "cyls",		required_argument,	0, 'c' },
	{ "heads",		required_argument,	0, 'h' },
	{ "spt",		required_argument,	0, 's' },
	{ "sector",		required_argument,	0, 'l' },
	{ "id",			required_argument,	0, 'x' },
	{ "flag",		required_argument,	0, 'f' },
	{ "append",		no_argument,		0, 'a' },
	{ "help",		no_argument,		0, 0 },
	{ 0,			0,					0, 0 }
};

struct preset_t {
	int type;
	char *name;
	char *description;
	int cyls;
	int heads;
	int spt;
	int sector;
	int id;
	uint32_t flags;
};

struct flags_t {
	char *name;
	uint32_t flag;
};

struct preset_t known_presets[] = {
	{ E4I_T_HDD,"win20", "Winchester 20MB", 615, 4, 17, 512, 0, 0 },
	{ E4I_T_HDD,"m9425", "MERA 9425 (IBM 5440) fixed plate", 204, 2, 32, 192, 0, 0 },
	{ E4I_T_HDC,"m9425", "MERA 9425 (IBM 5440) removable cardridge", 204, 2, 32, 192, 0, E4I_F_REMOVABLE },
	{ E4I_T_FLOPPY, "flop5dsdd", "Floppy 5.25\" DSDD 360KB", 40, 2, 9, 512, 0, E4I_F_REMOVABLE },
	{ E4I_T_FLOPPY, "flop5dshd", "Floppy 5.25\" DSHD 1.2MB", 80, 2, 15, 512, 0, E4I_F_REMOVABLE },
	{ E4I_T_FLOPPY, "flop3dsdd", "Floppy 3.5\" DSDD 720KB", 80, 2, 9, 512, 0, E4I_F_REMOVABLE },
	{ E4I_T_FLOPPY, "flop3dshd", "Floppy 3.5\" DSHD 1.44MB", 80, 2, 18, 512, 0, E4I_F_REMOVABLE },
	{ E4I_T_NONE, NULL, NULL, 0, 0, 0, 0, 0, 0 }
};

struct flags_t known_flags[] = {
	{ "removable", E4I_F_REMOVABLE },
	{ "ro", E4I_F_WRPROTECT },
	{ NULL, 0 }
};

// -----------------------------------------------------------------------
void error(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	printf("Error: ");
	vprintf(format, ap);
	printf("\nUse --help to get help on usage.\n");
	va_end(ap);
	exit(1);
}
// -----------------------------------------------------------------------
void print_help()
{
	printf("\nOptions:\n");
	printf("  --help                    : print help\n");
	printf("  --image, -i <filename>    : e4i working media file name\n");
	printf("  --preset, -p <name>       : select media preset\n");
	printf("  --src, -r <filename>      : read raw input data from file <filename>\n");
	printf("  --dup, -d <filename>      : duplicate existing e4i media <filename> new image\n");
	printf("  --nomaster, -n            : set image as not master (--src sets master by default)\n");
	printf("  --blocks, -b <blocks>     : total blocks on media (LBA adressing)\n");
	printf("  --cyls, -c <cylinders>    : number of cylinders (CHS addressing)\n");
	printf("  --heads, -h <heads>       : number of heads (CHS addressing)\n");
	printf("  --spt, -s <sectors>       : sectors per track (CHS addressing)\n");
	printf("  --sector, -l <bytes>      : sector/block length\n");
	printf("  --id, -x <bytes>          : sector ID field length\n");
	printf("  --flag, -f <flag>|<^flag> : set/clear flag\n");
	printf("  --append, -a              : appendable media\n");
	printf("\n");
	printf("Usage scenarios:\n");
	printf(" * Create empty media:\n");
	printf("     e4itool --image <filename> --blocks <blocks> --sector <bytes> [--id <bytes>]\n");
	printf("     e4itool --image <filename> --cyls <c> --heads <h> --spt <sectors> --sector <bytes> [--id <bytes>]\n");
	printf("     e4itool --image <filename> --append --sector <bytes> [--id <bytes>]\n");
	printf(" * Create empty media using a preset:\n");
	printf("     e4itool --image <filename> --preset <name> [--cyls <c>] [--heads <h>] [--spt <sectors>] [--sector <bytes>] [--id <bytes>]\n");
	printf(" * Create media from raw data:\n");
	printf("     e4itool --image <filename> --src <filename> --sector <bytes> --id <bytes>\n");
	printf("     e4itool --image <filename> --src <filename> --cyls <c> --heads <h> --spt <sectors> --sector <bytes> --id <bytes>\n");
	printf(" * Duplicate existing image:\n");
	printf("     e4itool --image <filename> --dup <filename>\n");
	printf(" * Change flags:\n");
	printf("     e4itool --image <filename> --flag <name>|<^name> --flag <name>|<^name> ...\n");
	printf("\n");
	printf("Known image presets (C/H/S/block):\n");
	struct preset_t *p = known_presets;
	while (p && p->name) {
		printf("  * %s (%i/%i/%i/%i): %s\n", p->name, p->cyls, p->heads, p->spt, p->sector, p->description);
		p++;
	}
	printf("\n");
}

char *image, *preset, *src, *dup;
int append, master, blocks, cyls, heads, spt, sector, id, flags_set, flags_clear, got_flags;

// -----------------------------------------------------------------------
uint32_t decode_flag(char *name)
{
	struct flags_t *f = known_flags;
	while (f && f->name) {
		if (!strcasecmp(name, f->name)) {
			return f->flag;
		}
		f++;
	}
	return BAD_FLAG;
}

// -----------------------------------------------------------------------
struct preset_t * get_preset(char *name)
{
	struct preset_t *p = known_presets;
	while (p && p->name) {
		if (!strcasecmp(name, p->name)) {
			return p;
		}
		p++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void parse_opts(int argc, char **argv)
{
	int opt;
	int idx;
	struct preset_t *p = NULL;
	while (1) {
		opt = getopt_long(argc, argv,"i:p:r:d:nb:c:h:s:l:x:f:a", opts, &idx);
		if (opt == -1) {
			break;
		}
		switch (opt) {
			case 0:
				if (!strcmp(opts[idx].name, "help")) {
					print_help();
				}
				break;
			case 'i':
				image = optarg;
				break;
			case 'p':
				p = get_preset(optarg);
				if (!p) {
					error("Preset '%s' is unknown", optarg);
				} else {
					cyls = p->cyls;
					heads = p->heads;
					spt = p->spt;
					sector = p->sector;
					id = p->id;
					flags_set = p->flags;
					flags_clear = 0;
				}
				break;
			case 'r':
				src = optarg;
				break;
			case 'd':
				dup = optarg;
				break;
			case 'm':
				master = 1;
				break;
			case 'b':
				blocks = atoi(optarg);
				break;
			case 'c':
				cyls = atoi(optarg);
				break;
			case 'h':
				heads = atoi(optarg);
				break;
			case 's':
				spt = atoi(optarg);
				break;
			case 'l':
				sector = atoi(optarg);
				break;
			case 'x':
				id = atoi(optarg);
				break;
			case 'f':
				got_flags = 1;
				uint32_t tmp_flag = 0;
				if (*optarg == '^') {
					tmp_flag = decode_flag(optarg+1);
					if (tmp_flag == BAD_FLAG) {
						error("Unknown flag: %s", optarg+1);
					}
					flags_clear |= tmp_flag;
				} else  {
					tmp_flag = decode_flag(optarg);
					if (tmp_flag == BAD_FLAG) {
						error("Unknown flag: %s", optarg);
					}
					flags_set |= tmp_flag;
				}
				break;
			case 'a':
				append = 1;
				break;
			default:
				error("Unknown option");
				break;
		}
	}
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	printf("e4itool v0.1: e4image management tool\n");
	parse_opts(argc, argv);

	// image name is always required
	if (!image) {
		error("Image name required");
	}

	// dup or create
	if (dup && (blocks || cyls || heads || spt || sector || id || src)) {
		error("Mixing --dup with create options is not allowed");
	}

	// create new media from preset
	if (preset) {

	// create LBA media
	} else if (blocks && sector) {
		if (cyls || heads || spt) {
			error("mixing --blocks with --cyls/--heads/--spt is not allowed");
		}

	// create CHS media
	} else if (cyls && heads && spt && sector) {
		if (blocks) {
			error("mixing --blocks with --cyls/--heads/--spt is not allowed");
		}

	// set flags
	} else if (got_flags) {

	// duplicate
	} else if (dup) {

	// unknown mode of operation, missing options, wrong mix of options
	} else {
		error("Wrong usage");
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
