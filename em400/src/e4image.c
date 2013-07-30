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
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>

#include "e4image.h"

int e4i_err;

struct e4i_errdesc_t errdesc[] = {
	{ E4I_E_OK, "OK" },
	{ E4I_E_EXISTS, "image already exists" },
	{ E4I_E_OPEN, "cannot open image" },
	{ E4I_E_ALLOC, "cannot allocate memory" },
	{ E4I_E_HEADER_WRITE, "error writing header" },
	{ E4I_E_HEADER_READ, "error readig header" },
	{ E4I_E_NO_SECTOR, "sector not found" },
	{ E4I_E_READ, "read error" },
	{ E4I_E_WRITE, "write error" },
	{ E4I_E_FILL, "initial media fill failed" },
	{ E4I_E_SOURCE_OPEN, "data source open failed" },
	{ E4I_E_SOURCE_LEN, "data source length incompatibile" },
	{ E4I_E_SOURCE_READ, "data source read failed" },
	{ E4I_E_MAGIC, "e4i header not found" },
	{ E4I_E_IMAGE_V_MAJOR, "image major version mismatch" },
	{ E4I_E_IMAGE_V_MINOR, "image minor version mismatch" },
	{ E4I_E_FLAGS, "flag cannot be changed" },
	{ E4I_E_BLOCK_SIZE, "wrong block size" },
	{ E4I_E_UNFORMATTED, "media not formatted" },
	{ E4I_E_FORMATTED, "media already formatted" },
	{ E4I_E_WRPROTECT, "media write protected" },
	{ E4I_E_ACCESS, "wrong access mode (chs, lba, seq) for media" },
	{ E4I_E_GENF_MISSING, "missing ID field generatr function" },
	{ E4I_E_GENF_UNNEEDED, "ID field generatr function specified, but ID size is 0" },
	{ E4I_E_IDGEN, "could not generate ID filed for sector" },

	{ E4I_E_UNKNOWN, "unknown error" }
};

// -----------------------------------------------------------------------
void e4i_close(struct e4i_t *e)
{
	if (e) {
		if (e->image) fclose(e->image);
		if (e->img_name) free(e->img_name);
		free(e);
	}
}

// -----------------------------------------------------------------------
const char * e4i_get_err(int i)
{
	struct e4i_errdesc_t *ed = errdesc;
	while (ed && (ed->code != E4I_E_UNKNOWN)) {
		if (ed->code == i) {
			return ed->desc;
		}
		ed++;
	}
	return ed->desc;
}

// -----------------------------------------------------------------------
void e4i_header_print(struct e4i_t *e)
{
	printf("--------------------------------------\n");
	printf(" Image name   : %s\n", e->img_name);
	printf(" Header len   : %i\n", E4I_HEADER_SIZE);
	printf(" Magic        : %c%c%c%c\n", e->magic[0], e->magic[1], e->magic[2], e->magic[3]);
	printf(" Version      : %i.%i\n", e->v_major, e->v_minor);
	printf(" Image type   : %i (user type: %i)\n", e->img_type, e->img_utype);
	printf(" Flags        : %s%s%s%s%s%s%s\n",
		e->flags&E4I_F_FORMATTED ? "formatted " : "unformatted ",
		e->flags&E4I_F_WRPROTECT ? "wrprotect " : "",
		e->flags&E4I_F_REMOVABLE ? "removable " : "",
		e->flags&E4I_F_MASTERCOPY ? "master " : "",
		e->flags&E4I_F_CHS ? "chs " : "",
		e->flags&E4I_F_LBA ? "lba " : "",
		e->flags&E4I_F_APPEND ? "append " : "");
	printf(" Total blocks : %i\n", e->blocks);
	printf(" CHS geometry : %i / %i / %i\n", e->cylinders, e->heads, e->spt);
	printf(" ID size      : %i\n", e->id_size);
	printf(" Block size   : %i\n", e->block_size);
	printf("--------------------------------------\n");
}

// -----------------------------------------------------------------------
int __e4i_header_read(struct e4i_t *e)
{
	uint8_t *buf = calloc(1, E4I_HEADER_SIZE);
	if (!buf) {
		return E4I_E_ALLOC;
	}

	// read data
	if (fseek(e->image, 0, SEEK_SET)) {
		free(buf);
		return E4I_E_HEADER_READ;
	}
	if (fread(buf, 1, E4I_HEADER_SIZE, e->image) !=  E4I_HEADER_SIZE) {
		free(buf);
		return E4I_E_HEADER_READ;
	}

	// unpack data
	uint8_t *pos = buf;
	*(uint32_t*)e->magic = *(uint32_t*)pos; pos += 4;
    e->v_major = *pos; pos += 1;
    e->v_minor = *pos; pos += 1;
    e->img_type = ntohs(*(uint16_t*)pos); pos += 2;
    e->img_utype = ntohs(*(uint16_t*)pos); pos += 2;
    e->flags = ntohl(*(uint32_t*)pos); pos += 4;
    e->blocks = ntohl(*(uint32_t*)pos); pos += 4;
    e->cylinders = ntohs(*(uint16_t*)pos); pos += 2;
    e->heads = *pos; pos += 1;
    e->spt = *pos; pos += 1;
    e->id_size = ntohs(*(uint16_t*)pos); pos += 2;
    e->block_size = ntohs(*(uint16_t*)pos); pos += 2;

	return E4I_E_OK;
}

// -----------------------------------------------------------------------
int __e4i_header_write(struct e4i_t *e)
{
	uint8_t *buf = calloc(1, E4I_HEADER_SIZE);
	if (!buf) {
		return E4I_E_ALLOC;
	}

	// pack data
	uint8_t *pos = buf;
	*(uint32_t*)pos = *(uint32_t*)e->magic; pos += 4;
	*pos = e->v_major; pos += 1;
	*pos = e->v_minor; pos += 1;
	*(uint16_t*)pos = htons(e->img_type); pos += 2;
	*(uint16_t*)pos = htons(e->img_utype); pos += 2;
	*(uint32_t*)pos = htonl(e->flags); pos += 4;
	*(uint32_t*)pos = htonl(e->blocks); pos += 4;
	*(uint16_t*)pos = htons(e->cylinders); pos += 2;
	*pos = e->heads; pos += 1;
	*pos = e->spt; pos += 1;
	*(uint16_t*)pos = htons(e->id_size); pos += 2;
	*(uint16_t*)pos = htons(e->block_size); pos += 2;

	// write data
	if (fseek(e->image, 0, SEEK_SET)) {
		free(buf);
		return E4I_E_HEADER_WRITE;
	}
	if (fwrite(buf, 1, E4I_HEADER_SIZE, e->image) !=  E4I_HEADER_SIZE) {
		free(buf);
		return E4I_E_HEADER_WRITE;
	}

	free(buf);
	return E4I_E_OK;
}

// -----------------------------------------------------------------------
int e4i_flag_set(struct e4i_t *e, uint32_t flag)
{
	if (flag & !e4i_flags_resetable) {
		return E4I_E_FLAGS;
	}
	e->flags |= flag;
	return __e4i_header_write(e);
}

// -----------------------------------------------------------------------
int e4i_flag_clear(struct e4i_t *e, uint32_t flag)
{
	if (flag & !e4i_flags_resetable) {
		return E4I_E_FLAGS;
	}
	e->flags &= !flag;
	return __e4i_header_write(e);
}

// -----------------------------------------------------------------------
int __e4i_header_check(struct e4i_t *e)
{
	if (strncmp(e->magic, E4I_MAGIC, 4) != 0) {
		return E4I_E_MAGIC;
	}
	if (e->v_major != E4I_IMAGE_V_MAJOR) {
		return E4I_E_IMAGE_V_MAJOR;
	}
	if (e->v_minor > E4I_IMAGE_V_MINOR) {
		return E4I_E_IMAGE_V_MINOR;
	}
	if (e->block_size <= 0) {
		return E4I_E_BLOCK_SIZE;
	}
	if (!(e->flags & E4I_F_FORMATTED)) {
		return E4I_E_UNFORMATTED;
	}
	return E4I_E_OK;
}

// -----------------------------------------------------------------------
struct e4i_t * e4i_open(char *img_name)
{
	int res;
	e4i_err = E4I_E_OK;

	struct e4i_t *e = calloc(1, sizeof(struct e4i_t));
	if (!e) {
		e4i_err = E4I_E_ALLOC;
		return NULL;
	}

	// open image
	e->image = fopen(img_name, "r+");
	if (!e->image) {
		e4i_err = E4I_E_OPEN;
		e4i_close(e);
		return NULL;
	}

	// read header
	res = __e4i_header_read(e);
	if (res != E4I_E_OK) {
		e4i_err = res;
		e4i_close(e);
		return NULL;
	}

	// check header sanity
	res = __e4i_header_check(e);
	if (res != E4I_E_OK) {
		e4i_err = res;
		e4i_close(e);
		return NULL;
	}

	e->cur_pos = 0;
	e->img_name = strdup(img_name);
	return e;
}

// -----------------------------------------------------------------------
struct e4i_t * __e4i_create(char *img_name, uint16_t id_size, uint16_t block_size, uint16_t cylinders, uint8_t heads, uint8_t spt, uint32_t blocks, uint32_t flags)
{
	e4i_err = E4I_E_OK;

	// we don't destroy images
	struct stat st;
	if (stat(img_name, &st) == 0) {
		e4i_err = E4I_E_EXISTS;
		return NULL;
	}

	struct e4i_t *e = calloc(1, sizeof(struct e4i_t));
	if (!e) {
		e4i_err = E4I_E_ALLOC;
		return NULL;
	}

	// create image file
	e->image = fopen(img_name, "w+");
	if (!e->image) {
		free(e);
		e4i_err = E4I_E_OPEN;
		return NULL;
	}

	// fill header data
	strncpy(e->magic, E4I_MAGIC, 4);
	e->v_major = E4I_IMAGE_V_MAJOR;
	e->v_minor = E4I_IMAGE_V_MINOR;
	e->flags = flags;
	e->cylinders = cylinders;
	e->heads = heads;
	e->spt = spt;
	e->blocks = blocks;
	e->id_size = id_size;
	e->block_size = block_size;
	e->cur_pos = 0;
	e->img_name = strdup(img_name);

	int res;

	// write header
	res = __e4i_header_write(e);
	if (res != E4I_E_OK) {
		e4i_close(e);
		e4i_err = res;
		return NULL;
	}

	return e;
}

// -----------------------------------------------------------------------
struct e4i_t * e4i_create_chs(char *img_name, uint16_t id_size, uint16_t block_size, uint16_t cylinders, uint8_t heads, uint8_t spt)
{
	return __e4i_create(img_name, id_size, block_size, cylinders, heads, spt, cylinders*heads*spt, E4I_F_CHS | E4I_F_LBA);
}

// -----------------------------------------------------------------------
struct e4i_t * e4i_create_lba(char *img_name, uint16_t id_size, uint16_t block_size, uint32_t blocks, int append)
{
	return __e4i_create(img_name, id_size, block_size, 0, 0, 0, blocks, E4I_F_LBA | append ? E4I_F_APPEND : 0);
}

// -----------------------------------------------------------------------
struct e4i_t * e4i_create_seq(char *img_name, uint16_t id_size, uint16_t block_size)
{
	return __e4i_create(img_name, id_size, block_size, 0, 0, 0, 0, E4I_F_APPEND);
}

// -----------------------------------------------------------------------
int e4i_import(struct e4i_t *e, char *src_name, uint16_t img_type, uint16_t img_utype)
{
	if (e->flags & E4I_F_FORMATTED) {
		return E4I_E_FORMATTED;
	}

	// open
	FILE *source = fopen(src_name, "r");
	if (!source) {
		return E4I_E_SOURCE_OPEN;
	}

	if (fseek(source, 0, SEEK_END)) {
		fclose(source);
		return E4I_E_SOURCE_OPEN;
	}
	int source_len = ftell(source);

	// check source size for media with fixed size
	if (!(e->flags & E4I_F_APPEND)) {
		if (source_len != e->blocks * (e->id_size + e->block_size)) {
			fclose(source);
			return E4I_E_SOURCE_LEN;
		}
	}

	if (fseek(source, 0, SEEK_SET)) {
		fclose(source);
		return E4I_E_SOURCE_READ;
	}

	int ret = E4I_E_OK;
	int bufsize = e->id_size + e->block_size;
	uint8_t *buf = calloc(bufsize, 1);

	for (long s=0 ; s < source_len/bufsize ; s++) {
		if (fread(buf, 1, bufsize, source) != bufsize) {
			ret = E4I_E_SOURCE_READ;
			break;
		}
		// write block
		ret = __e4i_write_ignore_flags(e, buf, s, bufsize, 0, bufsize);
		if (ret != E4I_E_OK) {
			break;
		}
	}

	if (ret == E4I_E_OK) {
		e->flags |= E4I_F_FORMATTED | E4I_F_MASTERCOPY;
		e->img_type = img_type;
		e->img_utype = img_utype;
	    ret = __e4i_header_write(e);
	}

	fclose(source);
	free(buf);
	return ret;
}

// -----------------------------------------------------------------------
int __e4i_init_disk(struct e4i_t *e, e4i_id_gen_f *genf, uint16_t img_type, uint16_t img_utype)
{
	if ((e->id_size > 0) && !genf) {
		return E4I_E_GENF_MISSING;
	}
	if ((e->id_size == 0) && genf) {
		return E4I_E_GENF_UNNEEDED;
	}
	if (e->flags & E4I_F_FORMATTED) {
		return E4I_E_FORMATTED;
	}

	int ret = E4I_E_OK;
	int bufsize = e->id_size + e->block_size;
	uint8_t *buf = calloc(bufsize, 1);
	uint8_t *id_buf = calloc(e->id_size, 1);

	for (long s=0 ; s<e->blocks ; s++) {
		if (genf) {
			if (genf(id_buf, e->id_size, s) != e->id_size) {
				ret = E4I_E_IDGEN;
				break;
			}
			memcpy(buf, id_buf, e->id_size);
		}
		// write block
		ret = __e4i_write_ignore_flags(e, buf, s, bufsize, 0, bufsize);
		if (ret != E4I_E_OK) {
			break;
		}
	}

	if (ret == E4I_E_OK) {
		e->flags |= E4I_F_FORMATTED;
		e->img_type = img_type;
		e->img_utype = img_utype;
	    ret = __e4i_header_write(e);
	}

	free(buf);
	free(id_buf);
	return ret;
}

// -----------------------------------------------------------------------
int __e4i_init_seq(struct e4i_t *e, uint16_t img_type, uint16_t img_utype)
{
	e->flags |= E4I_F_FORMATTED;
	e->img_type = img_type;
	e->img_utype = img_utype;
	return __e4i_header_write(e);
}

// -----------------------------------------------------------------------
int e4i_init(struct e4i_t *e, e4i_id_gen_f *genf, uint16_t img_type, uint16_t img_utype)
{
	if (!(e->flags & E4I_F_APPEND)) {
		return __e4i_init_disk(e, genf, img_type, img_utype);
	} else {
		return __e4i_init_seq(e, img_type, img_utype);
	}
}

// -----------------------------------------------------------------------
int __e4i_s2b(struct e4i_t *e, int cyl, int head, int sect)
{
	return sect + (head * e->spt) + (cyl * e->heads * e->spt);
}

// -----------------------------------------------------------------------
int __e4i_read(struct e4i_t *e, uint8_t *buf, int block, int boffset, int struct_size)
{
	if (!(e->flags & E4I_F_FORMATTED)) {
		return E4I_E_UNFORMATTED;
	}

	int res;
	int csize = e->id_size + e->block_size;

	res = fseek(e->image, E4I_HEADER_SIZE + block*csize + boffset, SEEK_SET);
	if (res < 0) {
		return E4I_E_NO_SECTOR;
	}

	res = fread(buf, 1, struct_size, e->image);
	if (res != struct_size) {
		return E4I_E_READ;
	}

	return E4I_E_OK;
}

// -----------------------------------------------------------------------
int __e4i_write_ignore_flags(struct e4i_t *e, uint8_t *buf, int block, int bytes, int boffset, int max_bytes)
{
	int res;
	int csize = e->id_size + e->block_size;

	if (bytes > max_bytes) {
		return E4I_E_WRITE;
	}

	res = fseek(e->image, E4I_HEADER_SIZE + block*csize + boffset, SEEK_SET);
	if (res < 0) {
		return E4I_E_NO_SECTOR;
	}

	res = fwrite(buf, 1, bytes, e->image);
	if (res != bytes) {
		return E4I_E_WRITE;
	}

	return E4I_E_OK;

}

// -----------------------------------------------------------------------
int __e4i_write(struct e4i_t *e, uint8_t *buf, int block, int bytes, int boffset, int max_bytes)
{
	if (e->flags & (E4I_F_WRPROTECT | E4I_F_MASTERCOPY)) {
		return E4I_E_WRPROTECT;
	}

	if (!(e->flags & E4I_F_FORMATTED)) {
		return E4I_E_UNFORMATTED;
	}

	return __e4i_write_ignore_flags(e, buf, block, bytes, boffset, max_bytes);
}

// CHS access

// -----------------------------------------------------------------------
int e4i_sread(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect)
{
	if (!(e->flags & E4I_F_CHS)) {
		return E4I_E_ACCESS;
	}
	return __e4i_read(e, buf, __e4i_s2b(e, cyl, head, sect), e->id_size, e->block_size);
}

// -----------------------------------------------------------------------
int e4i_swrite(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect, int bytes)
{
	if (!(e->flags & E4I_F_CHS)) {
		return E4I_E_ACCESS;
	}
	return __e4i_write(e, buf, __e4i_s2b(e, cyl, head, sect), bytes, e->id_size, e->block_size);
}

// -----------------------------------------------------------------------
int e4i_sread_id(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect)
{
	if (!(e->flags & E4I_F_CHS)) {
		return E4I_E_ACCESS;
	}
	return __e4i_read(e, buf, __e4i_s2b(e, cyl, head, sect), 0, e->id_size);
}

// -----------------------------------------------------------------------
int e4i_swrite_id(struct e4i_t *e, uint8_t *buf, int cyl, int head, int sect)
{
	if (!(e->flags & E4I_F_CHS)) {
		return E4I_E_ACCESS;
	}
	return __e4i_write(e, buf, __e4i_s2b(e, cyl, head, sect), e->id_size, 0, e->id_size);
}

// LBA access

// -----------------------------------------------------------------------
int e4i_bread(struct e4i_t *e, uint8_t *buf, int block)
{
	if (!(e->flags & E4I_F_LBA)) {
		return E4I_E_ACCESS;
	}
	return __e4i_read(e, buf, block, e->id_size, e->block_size);
}

// -----------------------------------------------------------------------
int e4i_bwrite(struct e4i_t *e, uint8_t *buf, int block, int bytes)
{
	if (!(e->flags & E4I_F_LBA)) {
		return E4I_E_ACCESS;
	}
	return __e4i_write(e, buf, block, bytes, e->id_size, e->block_size);
}

// -----------------------------------------------------------------------
int e4i_bread_id(struct e4i_t *e, uint8_t *buf, int block)
{
	if (!(e->flags & E4I_F_LBA)) {
		return E4I_E_ACCESS;
	}
	return __e4i_read(e, buf, block, 0, e->id_size);
}

// -----------------------------------------------------------------------
int e4i_bwrite_id(struct e4i_t *e, uint8_t *buf, int block)
{
	if (!(e->flags & E4I_F_LBA)) {
		return E4I_E_ACCESS;
	}
	return __e4i_write(e, buf, block, e->id_size, 0, e->id_size);
}

// sequential access

// -----------------------------------------------------------------------
int e4i_bget(struct e4i_t *e, uint8_t *buf, int count)
{
	return E4I_E_OK;
}

// -----------------------------------------------------------------------
int e4i_bappend(struct e4i_t *e, uint8_t *buf, int count)
{
	if (!(e->flags & E4I_F_APPEND)) {
		return E4I_E_ACCESS;
	}

	return E4I_E_OK;
}

// -----------------------------------------------------------------------
int e4i_rewind(struct e4i_t *e)
{

	return E4I_E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
