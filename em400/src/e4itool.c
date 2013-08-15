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

enum actions_e{
	A_UNKNOWN = -1,
	A_FLAGS = 1,
	A_CREATE_CHS = 100,
	A_CREATE_LBA,
	A_CREATE_SEQ,
};

enum flags_e {
	FCLR = 0,
	FSET = 1,
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
	e4i_id_gen_f *genf;
};

struct kv_t {
	char *name;
	int value;
	char *description;
};

int m9425_idgenf(struct e4i_t *e, uint8_t *buf, int id_len, uint32_t block);

struct preset_t known_presets[] = {
	{ E4I_T_HDD,"win20", "Winchester 20MB", 615, 4, 17, 512, 0, 0, NULL },
	{ E4I_T_HDD,"m9425f", "MERA 9425 (IBM 5440 14\") fixed plate", 203, 2, 12, 512, 10, 0, m9425_idgenf },
	{ E4I_T_HDC,"m9425r", "MERA 9425 (IBM 5440 14\") removable cardridge", 203, 2, 12, 512, 10, E4I_F_REMOVABLE, m9425_idgenf },
	{ E4I_T_FLOPPY, "flop5dsdd", "Floppy 5.25\" DSDD 360KB", 40, 2, 9, 512, 0, E4I_F_REMOVABLE, NULL },
	{ E4I_T_FLOPPY, "flop5dshd", "Floppy 5.25\" DSHD 1.2MB", 80, 2, 15, 512, 0, E4I_F_REMOVABLE, NULL },
	{ E4I_T_FLOPPY, "flop3dsdd", "Floppy 3.5\" DSDD 720KB", 80, 2, 9, 512, 0, E4I_F_REMOVABLE, NULL },
	{ E4I_T_FLOPPY, "flop3dshd", "Floppy 3.5\" DSHD 1.44MB", 80, 2, 18, 512, 0, E4I_F_REMOVABLE, NULL },
	{ E4I_T_NONE, NULL, NULL, 0, 0, 0, 0, 0, 0, NULL }
};

struct kv_t known_flags[] = {
	{ "removable", E4I_F_REMOVABLE },
	{ "ro", E4I_F_WRPROTECT },
	{ "master", E4I_F_MASTERCOPY },
	{ NULL, 0 }
};

struct kv_t known_types[] = {
	{ "none", E4I_T_NONE, "none" },
	{ "unknown", E4I_T_UNKNOWN, "unknown media" },
	{ "hdd", E4I_T_HDD, "hard disk drive" },
	{ "hdc", E4I_T_HDC, "hard disk cartridge" },
	{ "floppy", E4I_T_FLOPPY, "floppy disk" },
	{ "pcard", E4I_T_PUNCH_CARD, "punch cards stack" },
	{ "ptape", E4I_T_PUNCH_TAPE, "punch tape" },
	{ "mtape", E4I_T_MAGNETIC_TAPE, "magnetic tape" },
	{ NULL, 0 }
};

char *image, *preset, *src;
int append, blocks, cyls, heads, spt, sector, id, flags_set, flags_clear, got_flags, type, utype;
e4i_id_gen_f *genf = NULL;

// -----------------------------------------------------------------------
int m9425_idgenf(struct e4i_t *e, uint8_t *buf, int id_len, uint32_t block)
{
	uint16_t cylinder = block / (e->heads * e->spt);
	int rem = block % (e->heads * e->spt);
	uint8_t head = rem / e->spt;
	uint8_t sector = rem / e->spt;

	*(buf+0) = (cylinder>>8) & 1;
	*(buf+1) = cylinder & 255;

	*(buf+2) = head | (e->flags & E4I_F_REMOVABLE) ? 0b100 : 0;
	*(buf+3) = sector;

	*(buf+4) = 0; // key
	*(buf+5) = 0;
	*(buf+6) = 0; // user status
	*(buf+7) = 0;
	*(buf+8) = 0; // crc
	*(buf+9) = 0;
	
	return 10;
}

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
	printf("  --blocks, -b <blocks>     : total blocks on media (LBA adressing)\n");
	printf("  --cyls, -c <cylinders>    : number of cylinders (CHS addressing)\n");
	printf("  --heads, -h <heads>       : number of heads (CHS addressing)\n");
	printf("  --spt, -s <sectors>       : sectors per track (CHS addressing)\n");
	printf("  --sector, -l <bytes>      : sector/block length\n");
	printf("  --id, -x <bytes>          : sector ID field length\n");
	printf("  --flag, -f <flag>|<^flag> : set/clear flag\n");
	printf("  --append, -a              : appendable media\n");
	printf("  --type, -t <type>         : image type\n");
	printf("  --utype, -u <type>        : user image type\n");
	printf("\n");
	printf("Usage scenarios:\n");
	printf("  * Create empty media:\n");
	printf("      e4itool --image <filename> --blocks <blocks> --sector <bytes> [--id <bytes>]\n");
	printf("      e4itool --image <filename> --cyls <c> --heads <h> --spt <sectors> --sector <bytes> [--id <bytes>]\n");
	printf("      e4itool --image <filename> --append --sector <bytes> [--id <bytes>]\n");
	printf("  * Create empty media using a preset:\n");
	printf("      e4itool --image <filename> --preset <name> [--cyls <c>] [--heads <h>] [--spt <sectors>] [--sector <bytes>] [--id <bytes>]\n");
	printf("  * Create media from raw data:\n");
	printf("      e4itool --image <filename> --src <source> --sector <bytes> --id <bytes>\n");
	printf("      e4itool --image <filename> --src <source> --cyls <c> --heads <h> --spt <sectors> --sector <bytes> --id <bytes>\n");
	printf("  * Change flags:\n");
	printf("      e4itool --image <filename> --flag <name>|<^name> --flag <name>|<^name> ...\n");
	printf("\n");
	printf("Known image presets (--preset) (C/H/S, id size, sector size):\n");
	struct preset_t *p = known_presets;
	while (p && p->name) {
		printf("  * %s (%i/%i/%i, %i, %i) : %s\n", p->name, p->cyls, p->heads, p->spt, p->id, p->sector, p->description);
		p++;
	}
	printf("\n");
	printf("Known image types (--type):\n");
	struct kv_t *k = known_types;
	while (k && k->name) {
		printf("  * %s : %s\n", k->name, k->description);
		k++;
	}
	printf("\n");
}

// -----------------------------------------------------------------------
struct kv_t * get_kv(char *name, struct kv_t *f)
{
	while (f && f->name) {
		if (!strcasecmp(name, f->name)) {
			return f;
		}
		f++;
	}
	return NULL;
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
	struct kv_t *k = NULL;

	static struct option opts[] = {
		{ "image",		required_argument,	0, 'i' },
		{ "preset",		required_argument,	0, 'p' },
		{ "src",		required_argument,	0, 'r' },
		{ "blocks",		required_argument,	0, 'b' },
		{ "cyls",		required_argument,	0, 'c' },
		{ "heads",		required_argument,	0, 'h' },
		{ "spt",		required_argument,	0, 's' },
		{ "sector",		required_argument,	0, 'l' },
		{ "id",			required_argument,	0, 'x' },
		{ "flag",		required_argument,	0, 'f' },
		{ "append",		no_argument,		0, 'a' },
		{ "type",		required_argument,	0, 't' },
		{ "utype",		required_argument,	0, 'u' },
		{ "help",		no_argument,		0, 0 },
		{ 0,			0,					0, 0 }
	};

	while (1) {
		opt = getopt_long(argc, argv,"i:p:r:b:c:h:s:l:x:f:at:u:", opts, &idx);
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
					genf = p->genf;
					type = p->type;
				}
				break;
			case 'r':
				src = optarg;
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
				if (*optarg == '^') {
					k = get_kv(optarg+1, known_flags);
					if (!k) {
						error("Unknown flag: %s", optarg+1);
					}
					flags_clear |= k->value;
				} else  {
					k = get_kv(optarg, known_flags);
					if (!k) {
						error("Unknown flag: %s", optarg);
					}
					flags_set |= k->value;
				}
				break;
			case 'a':
				append = 1;
				break;
			case 't':
				k = get_kv(optarg, known_types);
				if (!k) {
					error("Unknown media type: %s", optarg);
				}
				type = k->value;
				break;
			case 'u':
				utype = atoi(optarg);
				break;
			default:
				error("Unknown option");
				break;
		}
	}
}

// -----------------------------------------------------------------------
int get_action()
{
	// image name is always required
	if (!image) {
		error("Image name required");
	}

	// need type to create
	if ((blocks || cyls || heads || spt || sector || append) && !type) {
		error("--type is required to create new media");
	}

    // ---- ACTIONS ------------------------------------------------------

	// create LBA media
	if (blocks && sector) {
		if (cyls || heads || spt) {
			error("Mixing --blocks with --cyls/--heads/--spt");
		}
		return A_CREATE_LBA;

	// create CHS media
	} else if (cyls && heads && spt && sector) {
		if (blocks) {
			error("Mixing --blocks with --cyls/--heads/--spt (directly or with --preset)");
		}
		return A_CREATE_CHS;

	// create appendable media
	} else if (append && sector) {
		if (blocks || cyls || heads || spt) {
			error("--blocks/--cyls/--heads/--spt used with --append");
		}
		return A_CREATE_SEQ;

	// set flags
	} else if (got_flags) {
		return A_FLAGS;

	// missing options or wrong mix of options
	} else {
		return A_UNKNOWN;
	}
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct e4i_t *e = NULL;
	int res;
	printf("e4itool v0.1: e4image management tool\n");
	parse_opts(argc, argv);
	int action = get_action();

	// create
	if (action >= A_CREATE_CHS) {

		// create as requested
		if (action == A_CREATE_LBA) {
			e = e4i_create_lba(image, id, sector, blocks, append);
		} else if (action == A_CREATE_CHS) {
			e = e4i_create_chs(image, id, sector, cyls, heads, spt);
		} else if (action == A_CREATE_SEQ) {
			e = e4i_create_seq(image, id, sector);
		}

		// ok?
		if (e) {
			printf("Image created:\n");
			e4i_header_print(e);
		} else {
			error("Could not create imege: %s", e4i_get_err(e4i_err));
		}

		// source given?
		if (src) {
			res = e4i_import(e, src, type, utype);
			if (res == E4I_E_OK) {
				printf("Image contents imported\n");
			} else {
				error("Could not import image contents: %s", e4i_get_err(res));
			}
		// init without source
		} else {
			res = e4i_init(e, genf, type, utype);
			if (res != E4I_E_OK) {
				error("Could not create imege: %s", e4i_get_err(res));
			}
		}

	// set flags only
	} else if (action == A_FLAGS) {
		e = e4i_open(image);
		if (!e) {
			error("Could not open imege: %s", e4i_get_err(e4i_err));
		}
	} else {
		error("Don't know how to proceed with given option set");
	}

	// any flags to set?
	if (flags_set) {
		res = e4i_flag_set(e, flags_set);
		if (res != E4I_E_OK) {
			error("Could not set flags: %s", e4i_get_err(res));
		}
	}
	// any flags to clear?
	if (flags_clear) {
		res = e4i_flag_clear(e, flags_clear);
		if (res != E4I_E_OK) {
			error("Could not clear flags: %s", e4i_get_err(res));
		}
	}
	e4i_close(e);

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
