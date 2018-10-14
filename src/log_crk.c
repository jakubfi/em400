//  Copyright (c) 2014-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <emcrk/kfind.h>
#include <emcrk/obj.h>
#include <emcrk/exl.h>
#include <emcrk/process.h>

#include "mem/mem.h"
#include "log.h"
#include "utils/utils.h"

static struct crk5_process *process;

static int log_exl_number = -1;
static int log_exl_nb;
static int log_exl_addr;
static int log_exl_r4;

struct crk5_kern_result *kernel;

// -----------------------------------------------------------------------
void log_crk_init()
{

}

// -----------------------------------------------------------------------
void log_crk_shutdown()
{
	crk5_process_delete(process);
	crk5_kern_res_drop(kernel);
}

// -----------------------------------------------------------------------
const char * log_get_current_process()
{
	static const char *process_name_none = "";
	return process ? process->name : process_name_none;
}

// -----------------------------------------------------------------------
char * log_ctx_stringify(struct crk5_process *process)
{
	char *buf = (char *) malloc(1024);
	char *b = buf;
	int pos = 0;

	char r0s[64];
	char srs[64];
	char szabme[64];
	char state[64];

	int2binf(r0s, "........ ........", process->r0, 16);
	int2binf(srs, ".......... . . ....", process->sr, 16);
	int2binf(szabme, "........ ........", process->segmap, 16);
	int2binf(state, "........ ........", process->state, 16);
	int ctx_q = (process->sr >> 5) & 1;
	int ctx_nb = process->sr & 0b1111;

	pos += sprintf(b+pos, "Process 0x%04x: %s\n", process->addr, process->name);
	pos += sprintf(b+pos, "Q:NB: %i:%i, IC: 0x%04x, R0: %s, SR: %s\n", ctx_q, ctx_nb, process->ic, r0s, srs);
	pos += sprintf(b+pos, "R1-R7: 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x\n", process->r1, process->r2, process->r3, process->r4, process->r5, process->r6, process->r7);
	pos += sprintf(b+pos, "State: %s (0x%04x), ", state, process->state);
	pos += sprintf(b+pos, "Prio: %i\n", process->prio);
	pos += sprintf(b+pos, "Size: %i words, %i segments (%s) \n", process->size, process->segments, szabme);

	pos += sprintf(b+pos, "Next: 0x%04x, ",  process->next_proc);
	pos += sprintf(b+pos, "Parent: 0x%04x, ", process->parent);
	pos += sprintf(b+pos, "Children: 0x%04x, next child: 0x%04x \n", process->children, process->next_child);

	pos += sprintf(b+pos, "NUM: 0x%04x \n", process->num);
	pos += sprintf(b+pos, "ALLS: 0x%04x \n", process->ALLS);
	pos += sprintf(b+pos, "CHTIM: 0x%04x \n", process->CHTIM);
	pos += sprintf(b+pos, "DEVI: 0x%04x, ", process->DEVI);
	pos += sprintf(b+pos, "DEVO: 0x%04x \n", process->DEVO);
	pos += sprintf(b+pos, "USAL: 0x%04x \n", process->USAL);
	pos += sprintf(b+pos, "STRLI: 0x%04x \n", process->STRLI);
	pos += sprintf(b+pos, "BUFLI: 0x%04x \n", process->BUFLI);
	pos += sprintf(b+pos, "LARUS: 0x%04x \n", process->LARUS);
	pos += sprintf(b+pos, "LISMEM: 0x%04x \n", process->LISMEM);
	pos += sprintf(b+pos, "NXTMEM: 0x%04x \n", process->NXTMEM);
	pos += sprintf(b+pos, "BAR: 0x%04x \n", process->BAR);
	pos += sprintf(b+pos, "BLPASC: 0x%04x \n", process->BLPASC);
	pos += sprintf(b+pos, "IC: 0x%04x, R0: 0x%04x, SR: 0x%04x \n", process->_ic, process->_r0, process->_sr);
	pos += sprintf(b+pos, "R1-7: 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x\n", process->_r1, process->_r2, process->_r3, process->_r4, process->_r5, process->_r6, process->_r7);
	pos += sprintf(b+pos, "JDAD: 0x%04x \n", process->JDAD);
	pos += sprintf(b+pos, "Program start (JPAD): 0x%04x \n", process->start);
	pos += sprintf(b+pos, "FILDIC position (JACN): 0x%04x \n", process->JACN);

	return buf;
}

// -----------------------------------------------------------------------
void log_log_process(unsigned component)
{
	static int last_pid;

	if (!process || !process->name || !*process->name) return;
	if (process->num == last_pid) return;

	last_pid = process->num;

	char *buf = log_ctx_stringify(process);

	LOGBLOB(component, buf);

	free(buf);
}

// -----------------------------------------------------------------------
static int decode_exl_tmem(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_tmem *exl = crk5_exl_tmem_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Err: %i\n", exl->err);
	pos += sprintf(b+pos, "Stream ID: %i (%s)\n", exl->stream_id, exl->stream_name);
	pos += sprintf(b+pos, "Addr: 0x%04x\n", exl->addr);
	pos += sprintf(b+pos, "Segment number: %i\n", exl->seg);

	crk5_exl_tmem_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_tim(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	if (exl_ret) {
		struct crk5_exl_tim *exl = crk5_exl_tim_unpack(data);
		if (!exl) return 0;
		pos += sprintf(b+pos, "%02i:%02i:%02i", exl->hour, exl->minute, exl->second);
		crk5_exl_tim_delete(exl);
	}

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_dat(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	if (exl_ret) {
		struct crk5_exl_dat *exl = crk5_exl_dat_unpack(data);
		if (!exl) return 0;
		pos += sprintf(b+pos, "%02i-%02i-%02i", exl->year, exl->month, exl->day);
		crk5_exl_dat_delete(exl);
	}

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_met(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_met *exl = crk5_exl_met_unpack(data);
	if (!exl) return 0;

	if (exl->disk_name && *(exl->disk_name)) {
		pos += sprintf(b+pos, "Disk: %s\n", exl->disk_name);
	} else {
		pos += sprintf(b+pos, "Disk: %i\n", exl->disk_id);
	}

	if (exl_ret) {
		pos += sprintf(b+pos, "DICDIC: %i\n", exl->dicdic);
		pos += sprintf(b+pos, "FILDIC: %i\n", exl->fildic);
		pos += sprintf(b+pos, "MAP: %i\n", exl->map);
		pos += sprintf(b+pos, "Length: %i\n", exl->len);
	}

	crk5_exl_met_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_pinf(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	if (exl_ret) {
		struct crk5_exl_pinf *exl = crk5_exl_pinf_unpack(data);
		if (!exl) return 0;
		pos += sprintf(b+pos, "System version: %i/%i\n", exl->sys_ver_maj, exl->sys_ver_min);
		pos += sprintf(b+pos, "Mem available: %i segments\n", exl->seg_total);
		pos += sprintf(b+pos, "User rights: 0%03o\n", exl->user_rights);
		pos += sprintf(b+pos, "Priority: %i\n", exl->prio);
		pos += sprintf(b+pos, "Special file len: %i sectors\n", exl->spec_file_len);
		pos += sprintf(b+pos, "Load address: 0x%04x\n", exl->entry_addr);
		pos += sprintf(b+pos, "Mem used: %i words\n", exl->words_used);
		pos += sprintf(b+pos, "Rights: 0x%04x\n", exl->attr);
		pos += sprintf(b+pos, "Area name: %s\n", exl->space_name);
		pos += sprintf(b+pos, "User name: %s\n", exl->user_name);
		pos += sprintf(b+pos, "Process name: %s\n", exl->proc_name);
		crk5_exl_pinf_delete(exl);
	}

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_proc(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_proc *exl = crk5_exl_proc_unpack(data);
	if (!exl) return 0;

	// TODO: what is provided and what is returned?

	pos += sprintf(b+pos, "Err: %i\n", exl->err);
	pos += sprintf(b+pos, "PID: 0x%04x\n", exl->num);
	pos += sprintf(b+pos, "IC: 0x%04x, R0: 0x%04x\n", exl->ic, exl->r0);
	if (num == CRK5_EXL_TREG) {
		pos += sprintf(b+pos, "SR: 0x%04x\n", exl->prio_sr);
	} else if (num == CRK5_EXL_SREG) {
		// ignored
	} else {
		pos += sprintf(b+pos, "Prio: 0x%04x\n", exl->prio_sr);
	}
	pos += sprintf(b+pos, "R1-R7: 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x\n", exl->uregs[0], exl->uregs[1], exl->uregs[2], exl->uregs[3], exl->uregs[4], exl->uregs[5], exl->uregs[6]);

	crk5_exl_proc_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_block(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_block *exl = crk5_exl_block_unpack(data);
	if (!exl) return 0;

	if (exl_ret) {
		pos += sprintf(b+pos, "Transmitted: %i\n", exl->transmitted);
	}

	if ((num == CRK5_EXL_WRIT) || (num == CRK5_EXL_READ)) {
		pos += sprintf(b+pos, "Stream ID: %i (%s)\n", exl->stream_id, exl->stream_name);
	} else if ((num == CRK5_EXL_WRIP) || (num == CRK5_EXL_REAP)) {
		pos += sprintf(b+pos, "PID: %i\n", exl->stream_id);
	} else {
		// ignored for OVL (and probably for WADR)
	}

	pos += sprintf(b+pos, "Buf addr: 0x%04x\n", exl->addr);
	pos += sprintf(b+pos, "Count: %i\n", exl->count);
	pos += sprintf(b+pos, "Start sector (into file): %i\n", exl->start_sector);

	crk5_exl_block_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int log_print_mem(char *b, int nb, uint16_t addr, int max_len, int terminator)
{
	int pos = 0;
	uint16_t ch;
	uint16_t word;
	int need = 1;
	int chars = 0;

	pos += sprintf(b+pos, "Data: \"");

	while (1) {
		if (chars >= max_len) break;
		if (need) {
			need = 0;
			if (!mem_get(nb, addr, &word)) break;
			addr++;
			ch = word >> 8;
		} else {
			need = 1;
			ch = word & 0xff;
		}
		if (ch == terminator) break;
		pos += sprintf(b+pos, "%c", ch);
		chars++;
	}

	pos += sprintf(b+pos, "\"\n");

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_rec(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_rec *exl = crk5_exl_rec_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Position: %i\n", exl->position);
	pos += sprintf(b+pos, "Stream ID: %i (%s)\n", exl->stream_id, exl->stream_name);
	pos += sprintf(b+pos, "Buf addr: 0x%04x\n", exl->buf_addr);
	pos += sprintf(b+pos, "Ending char: #%02x\n", exl->end_char);
	pos += sprintf(b+pos, "Max bytes: %i\n", exl->max_len);

	if (num == CRK5_EXL_PINP) {
		pos += sprintf(b+pos, "Prechars: \"%s\"\n", exl->prechars);
	}

	// if we print...
	if ((num == CRK5_EXL_PRIR) || (num == CRK5_EXL_PRIN)) {
		pos += log_print_mem(b+pos, log_exl_nb, exl->buf_addr, exl->max_len, exl->end_char);
	// ...or read, and return from EXL
	} else if (exl_ret && ((num == CRK5_EXL_INPR) || (num= CRK5_EXL_PINP) || (num == CRK5_EXL_INAM) || (num == CRK5_EXL_INUM))) {
		pos += log_print_mem(b+pos, log_exl_nb, exl->buf_addr, exl->position, exl->end_char);
	}

	crk5_exl_rec_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_obj(char *b, void *obj, int obj_type)
{
	int pos = 0;

	switch (obj_type) {
		case CRK5_OBJ_STREAM:
			pos += sprintf(b+pos, "STR %i",
				((struct crk5_obj_stream*)obj)->id
			);
			break;
		case CRK5_OBJ_DISK:
			pos += sprintf(b+pos, "DISK \"%s\" (0x%04x), sectors %i-%i",
				((struct crk5_obj_disk*)obj)->space_name,
				((struct crk5_obj_disk*)obj)->space,
				((struct crk5_obj_disk*)obj)->start,
				((struct crk5_obj_disk*)obj)->end
			);
			break;
		case CRK5_OBJ_FILE:
			pos += sprintf(b+pos, "%s/%s/%s",
				((struct crk5_obj_file*)obj)->space_name,
				((struct crk5_obj_file*)obj)->dir_name,
				((struct crk5_obj_file*)obj)->file_name
			);
			break;
		case CRK5_OBJ_DEVICE:
			pos += sprintf(b+pos, "DEV %i:%i",
				((struct crk5_obj_dev*)obj)->chan,
				((struct crk5_obj_dev*)obj)->unit
			);
			break;
		case CRK5_OBJ_MESSAGE:
			pos += sprintf(b+pos, "MSG len %i, addr 0x%04x, recv. process: 0x%04x",
				((struct crk5_obj_mes*)obj)->len,
				((struct crk5_obj_mes*)obj)->addr,
				((struct crk5_obj_mes*)obj)->recv_process
			);
			break;
		case CRK5_OBJ_MEM:
			pos += sprintf(b+pos, "MEM/%s",
				((struct crk5_obj_mem*)obj)->name
			);
			break;
		default:
			pos += sprintf(b+pos, "unknown type %i", obj_type);
			break;
	}

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_fil(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_fil *exl = crk5_exl_fil_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Err: %i\n", exl->err);
	pos += sprintf(b+pos, "Stream ID: %i (%s)\n", exl->stream_id, exl->stream_name);
	pos += sprintf(b+pos, "Type: %i\n", exl->type);
	pos += sprintf(b+pos, "Length: %i\n", exl->len);
	pos += sprintf(b+pos, "Parameter 1: 0x%04x (%i)\n", exl->param[0], exl->param[0]);
	pos += sprintf(b+pos, "Parameter 2: 0x%04x (%i)\n", exl->param[1], exl->param[1]);
	pos += sprintf(b+pos, "Rights : 0x%02x\n", exl->rights);
	pos += sprintf(b+pos, "Attributes: 0x%02x\n", exl->attributes);
	pos += sprintf(b+pos, "Mem: %i\n", exl->mem);

	pos += sprintf(b+pos, "Object: ");
	pos += decode_obj(b+pos, exl->obj, exl->obj_type);
	pos += sprintf(b+pos, "\n");

	crk5_exl_fil_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_str(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_str *exl = crk5_exl_str_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Err: %i\n", exl->err);
	pos += sprintf(b+pos, "Stream ID: %i (%s)\n", exl->stream_id, exl->stream_name);

	crk5_exl_str_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_err(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	struct crk5_exl_err *exl = crk5_exl_err_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Procedure address: 0x%04x\n", exl->proc_addr);
	pos += sprintf(b+pos, "Alarm address: 0x%04x\n", exl->alarm_addr);
	pos += sprintf(b+pos, "Alarm number: %i\n", exl->alarm_nr);

	crk5_exl_err_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_dir(char *b, uint16_t *data, int num, int exl_ret)
{
	int pos = 0;

	if ((num == CRK5_EXL_TDIR) && !exl_ret) return pos;

	struct crk5_exl_dir *exl = crk5_exl_dir_unpack(data);
	if (!exl) return 0;

	pos += sprintf(b+pos, "Area count: %i\n", exl->count);

	for (int i=0 ; i<exl->count ; i++) {
		pos += sprintf(b+pos, "User ID: %i, Directory ID: %i\n", exl->user_id[i], exl->dir_id[i]);
	}

	crk5_exl_dir_delete(exl);

	return pos;
}

// -----------------------------------------------------------------------
static int decode_exl_none(char *b, uint16_t *data, int num, int exl_ret)
{
	return 0;
}

// -----------------------------------------------------------------------
typedef int (*exldec)(char *b, uint16_t *data, int num, int exl_ret);
exldec exldecs[] = {
/* CRK5_EXL_ARG_NONE */ decode_exl_none,
/* CRK5_EXL_ARG_FIL  */ decode_exl_fil,
/* CRK5_EXL_ARG_TMEM */ decode_exl_tmem,
/* CRK5_EXL_ARG_STR  */ decode_exl_str,
/* CRK5_EXL_ARG_REC  */ decode_exl_rec,
/* CRK5_EXL_ARG_BLOCK*/ decode_exl_block,
/* CRK5_EXL_ARG_ERR  */ decode_exl_err,
/* CRK5_EXL_ARG_R4   */ decode_exl_none,
/* CRK5_EXL_ARG_PROC */ decode_exl_proc,
/* CRK5_EXL_ARG_DIR  */ decode_exl_dir,
/* CRK5_EXL_ARG_DAT  */ decode_exl_dat,
/* CRK5_EXL_ARG_TIM  */ decode_exl_tim,
/* CRK5_EXL_ARG_MET  */ decode_exl_met,
/* CRK5_EXL_ARG_PINF */ decode_exl_pinf,
/* CRK5_EXL_ARG_TABL */ decode_exl_none,
/* CRK5_EXL_ARG_PI   */ decode_exl_none,
/* CRK5_EXL_ARG_CAM  */ decode_exl_none,
/* CRK5_EXL_ARG_MT   */ decode_exl_none,
/* CRK5_EXL_ARG_REJE */ decode_exl_none
};

// -----------------------------------------------------------------------
static char * log_exl_decode(int nb, uint16_t addr, uint16_t r4_curr, int exl_num, int exl_ret)
{
	int pos = 0;
	char *buf = (char *) malloc(1024);
	if (!buf) return NULL;
	char *b = buf;

	const struct crk5_exl *exl = crk5_get_exl(exl_num);

	if (exl_ret) {
		pos += sprintf(b+pos, "return ");
	}

	// EXLs with no data structure
	if (exl->type == CRK5_EXL_ARG_R4) {
		pos += sprintf(b+pos, "EXL %i (%s - %s), R4 = 0x%04x\n", exl_num, exl->name, exl->desc, r4_curr);

	// EXLs with data structures
	} else {
		pos += sprintf(b+pos, "EXL %i (%s - %s), arg @ %i:0x%04x\n", exl_num, exl->name, exl->desc, nb, addr);

		uint16_t *data = (uint16_t *) malloc(sizeof(uint16_t) * exl->size);
		if (!data) {
			return buf;
		}

		if (mem_mget(nb, addr, data, exl->size) != exl->size) {
			return buf;
		}

		pos += exldecs[exl->type](b+pos, data, exl_num, exl_ret);

		free(data);
	}

	return buf;
}

// -----------------------------------------------------------------------
// called at every EXL
void log_handle_syscall(unsigned component, int num, int nb, int addr, uint16_t r4)
{
	log_exl_number = num;
	log_exl_nb = nb;
	log_exl_addr = addr;
	log_exl_r4 = r4;

	char *details = log_exl_decode(nb, r4, r4, num, 0);
	if (details) {
		LOGBLOB(component, details);
	}

	free(details);
}

// -----------------------------------------------------------------------
void log_syscall_reset()
{
	log_exl_number = -1; // indicate that there is no syscall in progress
}

// -----------------------------------------------------------------------
// called at every LIP and SP, after new process context is set
void log_handle_syscall_ret(unsigned component, uint16_t ic, uint16_t sr, uint16_t r4)
{
	// check if current context is where we left at last EXL
	if ((log_exl_number >= 0) && (ic == log_exl_addr) && ((sr & 0b1111) == log_exl_nb)) {
		char *details = log_exl_decode(log_exl_nb, log_exl_r4, r4, log_exl_number, 1);
		if (details) {
			LOGBLOB(component, details);
		}
		free(details);
		log_syscall_reset();
	}
}

// -----------------------------------------------------------------------
void log_reset_process()
{
	crk5_process_delete(process);
	process = NULL;
}

// -----------------------------------------------------------------------
// called when context is switched to a new process (SP, LIP)
void log_update_process()
{
	uint16_t bprog;
	uint16_t buf[CRK5P_PROCESS_SIZE];

	log_reset_process();

	if (!kernel) return;

	if (!mem_get(0, CRK5_BPROG, &bprog)) {
		return;
	}

	if (mem_mget(0, bprog, buf, CRK5P_PROCESS_SIZE) != CRK5P_PROCESS_SIZE) {
		return;
	}

	process = crk5_process_unpack(buf, bprog, kernel->mod);
}

// -----------------------------------------------------------------------
// called at every software reset (MCL)
void log_check_os()
{
	crk5_kern_res_drop(kernel);
	kernel = NULL;

	uint16_t *kimg = (uint16_t *) malloc(sizeof(uint16_t) * 2*4096);
	if (!kimg) {
		goto cleanup;
	}

	if (mem_mget(0, 0, kimg, 2*4096) != 2*4096) {
		goto cleanup;
	}

	kernel = crk5_kern_find(kimg, 2*4096);
	if (!kernel) {
		goto cleanup;
	}

	LOG(L_CRK5, "running CROOK for %s CPU, entry point @ 0x%04x, checksum: 0x%04x (%s)",
		kernel->mod ? "MX-16" : "MERA-400",
		kernel->entry_point,
		kernel->cksum_addr,
		kernel->cksum_stored == kernel->cksum_computed ? "OK" : "incorrect"
	);

cleanup:
	free(kimg);
}

// vim: tabstop=4 shiftwidth=4 autoindent
