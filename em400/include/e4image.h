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

#ifndef E4IMG_H
#define E4IMG_H

#include <stdio.h>
#include <inttypes.h>

#define E4I_MAGIC "E4IM"
#define E4I_IMAGE_V_MAJOR 1
#define E4I_IMAGE_V_MINOR 0

extern int e4i_err;
typedef int (e4i_id_gen_f)(uint8_t *buf, int id_len, uint32_t block);

enum e4i_errors_e {
	E4I_E_UNKNOWN = -1,
	E4I_E_OK = 0,
	E4I_E_EXISTS,
	E4I_E_OPEN,
	E4I_E_ALLOC,
	E4I_E_HEADER_WRITE,
	E4I_E_HEADER_READ,
	E4I_E_NO_SECTOR,
	E4I_E_READ,
	E4I_E_WRITE,
	E4I_E_FILL,
	E4I_E_SOURCE_OPEN,
	E4I_E_SOURCE_LEN,
	E4I_E_SOURCE_READ,
	E4I_E_MAGIC,
	E4I_E_IMAGE_V_MAJOR,
	E4I_E_IMAGE_V_MINOR,
	E4I_E_FLAGS,
	E4I_E_BLOCK_SIZE,
	E4I_E_IMG_TYPE,
	E4I_E_UNFORMATTED,
	E4I_E_FORMATTED,
	E4I_E_WRPROTECT,
	E4I_E_ACCESS,
	E4I_E_GENF_MISSING,
	E4I_E_GENF_UNNEEDED,
	E4I_E_IDGEN,
};

struct e4i_errdesc_t {
	int code;
	const char *desc;
};

enum e4i_media_flags_e {
	E4I_F_FORMATTED		= 1 << 0,	// formatted / unformatted
	E4I_F_WRPROTECT		= 1 << 1,	// write prohibited / allowed
	E4I_F_REMOVABLE		= 1 << 2,	// removable / fixed
	E4I_F_MASTERCOPY	= 1 << 3,	// master copy (protected) / working copy
	E4I_F_CHS			= 1 << 4,	// access by C/H/S / no access by C/H/S
	E4I_F_LBA			= 1 << 5,	// access by LBA / no access by LBA
	E4I_F_APPEND		= 1 << 6,	// appendable / not appendable
};

#define e4i_flags_resetable (E4I_F_WRPROTECT | E4I_F_REMOVABLE)

enum e4i_img_type_e {
	E4I_T_NONE = 0,			// no image type set
	E4I_T_UNKNOWN,			// unknown media
	E4I_T_HDD,				// hard disk drive
	E4I_T_HDC,				// hard disk cartridge
	E4I_T_FLOPPY,			// floppy disk
	E4I_T_PUNCH_CARD,		// punch cards stack
	E4I_T_PUNCH_TAPE,		// punch tape
	E4I_T_MAGNETIC_TAPE,	// magnetic tape

	E4I_T_MAX,
};

#define E4I_HEADER_SIZE 26
struct e4i_t {
	char magic[4];
	uint8_t v_major;
	uint8_t v_minor;
	uint16_t img_type;
	uint16_t img_utype;
	uint32_t flags;
	uint32_t blocks;
	uint16_t cylinders;
	uint8_t heads;
	uint8_t spt;
	uint16_t id_size;
	uint16_t block_size;
// --------------------------------
	char *img_name;
	FILE *image;
	uint32_t cur_pos;
};

// INTERNAL: header access
int __e4i_header_read(struct e4i_t *e);
int __e4i_header_write(struct e4i_t *e);
int __e4i_header_check(struct e4i_t *e);

void e4i_header_print(struct e4i_t *e);

struct e4i_t * e4i_open(char *img_name);
void e4i_close(struct e4i_t *e);
const char * e4i_get_err(int i);

int e4i_flag_set(struct e4i_t *e, uint32_t flag);
int e4i_flag_clear(struct e4i_t *e, uint32_t flag);

// INTERNAL: create media
struct e4i_t * __e4i_create(char *img_name, uint16_t id_size, uint16_t block_size, uint16_t cylinders, uint8_t heads, uint8_t spt, uint32_t blocks, uint32_t flags);

struct e4i_t * e4i_create_chs(char *img_name, uint16_t id_size, uint16_t block_size, uint16_t cylinders, uint8_t heads, uint8_t spt);
struct e4i_t * e4i_create_lba(char *img_name, uint16_t id_size, uint16_t block_size, uint32_t blocks, int append);
struct e4i_t * e4i_create_seq(char *img_name, uint16_t id_size, uint16_t block_size);

// INTERNAL: media initialization
int __e4i_init_disk(struct e4i_t *e, e4i_id_gen_f *genf, uint16_t img_type, uint16_t img_utype);
int __e4i_init_seq(struct e4i_t *e, uint16_t img_type, uint16_t img_utype);

int e4i_init(struct e4i_t *e, e4i_id_gen_f *genf, uint16_t img_type, uint16_t img_utype);
int e4i_import(struct e4i_t *e, char *src_name, uint16_t img_type, uint16_t img_utype);
int e4i_dup(struct e4i_t *e, char *dup_name);

// INTERNAL: block access
int __e4i_s2b(struct e4i_t *e, int cyl, int head, int sect);
int __e4i_read(struct e4i_t *e, uint8_t *buf, int block, int boffset, int struct_size);
int __e4i_write(struct e4i_t *e, uint8_t *buf, int block, int bytes, int boffset, int max_bytes);
int __e4i_write_ignore_flags(struct e4i_t *e, uint8_t *buf, int block, int bytes, int boffset, int max_bytes);

// CHS access
int e4i_sread(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect);
int e4i_swrite(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect, int bytes);
int e4i_sread_id(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect);
int e4i_swrite_id(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect);

// LBA access
int e4i_bread(struct e4i_t *e, uint8_t *buf, int block);
int e4i_bwrite(struct e4i_t *e, uint8_t *buf, int block, int bytes);
int e4i_bread_id(struct e4i_t *e, uint8_t *buf, int block);
int e4i_bwrite_id(struct e4i_t *e, uint8_t *buf, int block);

// sequential access
int e4i_bget(struct e4i_t *e, uint8_t *buf, int count);
int e4i_bappend(struct e4i_t *e, uint8_t *buf, int count);
int e4i_rewind(struct e4i_t *e);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
