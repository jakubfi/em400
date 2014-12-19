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

#include <strings.h>
#include <stdlib.h>

#include "mem/mem.h"

#include "io/multix.h"
#include "io/multix_winch.h"
#include "io/multix_term.h"
#include "io/cmem_m9425.h"

#include "utils.h"
#include "errors.h"

#include "debugger/awin.h"
#include "debugger/ui.h"
#include "debugger/decode.h"

#include <emcrk/r40.h>

struct decoder_t decoders[] = {
	{ "iv", "SYS: interrupt vectors (0x40)", decode_iv },
	{ "ctx", "SYS: process context", decode_ctx },
	{ "exl", "SYS: EXL call", decode_exl },
	{ "mxpsuk", "MULTIX: set configuration", decode_mxpsuk },
	{ "mxpsdl", "MULTIX: assign line", decode_mxpsdl },
	{ "mxpstwinch", "MULTIX: transmit (Winchester)", decode_mxpst_winch },
	{ "mxpstterm", "MULTIX: transmit (Terminal)", decode_mxpst_term },
	{ "cmempst", "MEM chan: transmit", decode_cmempst },
	{ NULL, NULL, NULL}
};

// -----------------------------------------------------------------------
struct decoder_t * find_decoder(char *name)
{
	struct decoder_t *d = decoders;
	while (d && d->name) {
		if (!strcasecmp(d->name, name)) {
			return d;
		}
		d++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
char * decode_iv(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	uint16_t data[33];
	mem_mget(nb, addr, data, 33);

	pos += sprintf(b+pos, "0  NMI        = 0x%04x    16 channel 4  = 0x%04x\n", data[0],  data[addr+16]);
	pos += sprintf(b+pos, "1  mem parity = 0x%04x    17 channel 5  = 0x%04x\n", data[1],  data[addr+17]);
	pos += sprintf(b+pos, "2  no mem     = 0x%04x    18 channel 6  = 0x%04x\n", data[2],  data[addr+18]);
	pos += sprintf(b+pos, "3  2nd CPU    = 0x%04x    19 channel 7  = 0x%04x\n", data[3],  data[addr+19]);
	pos += sprintf(b+pos, "4  I/F power  = 0x%04x    20 channel 8  = 0x%04x\n", data[4],  data[addr+20]);
	pos += sprintf(b+pos, "5  timer      = 0x%04x    21 channel 9  = 0x%04x\n", data[5],  data[addr+21]);
	pos += sprintf(b+pos, "6  illegal op = 0x%04x    22 channel 10 = 0x%04x\n", data[6],  data[addr+22]);
	pos += sprintf(b+pos, "7  div overfl = 0x%04x    23 channel 11 = 0x%04x\n", data[7],  data[addr+23]);
	pos += sprintf(b+pos, "8  FP underfl = 0x%04x    24 channel 12 = 0x%04x\n", data[8],  data[addr+24]);
	pos += sprintf(b+pos, "9  FP overfl  = 0x%04x    25 channel 13 = 0x%04x\n", data[9],  data[addr+25]);
	pos += sprintf(b+pos, "10 FP error   = 0x%04x    26 channel 14 = 0x%04x\n", data[10], data[addr+26]);
	pos += sprintf(b+pos, "11 extra int  = 0x%04x    27 channel 15 = 0x%04x\n", data[11], data[addr+27]);
	pos += sprintf(b+pos, "12 channel 0  = 0x%04x    28 OPRQ       = 0x%04x\n", data[12], data[addr+28]);
	pos += sprintf(b+pos, "13 channel 1  = 0x%04x    29 2nd CPU    = 0x%04x\n", data[13], data[addr+29]);
	pos += sprintf(b+pos, "14 channel 2  = 0x%04x    30 software H = 0x%04x\n", data[14], data[addr+30]);
	pos += sprintf(b+pos, "15 channel 3  = 0x%04x    31 software L = 0x%04x\n", data[15], data[addr+31]);
	pos += sprintf(b+pos, "EXL = 0x%04x\n",data[32]);

	return buf;
}

// -----------------------------------------------------------------------
char * decode_ctx(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	uint16_t data[56];

	mem_mget(nb, addr, data, 56);

	char *r0s = int2binf("........ ........", data[1], 16);
	char *srs = int2binf(".......... . . ....", data[2], 16);
	char *szabme = int2binf("........ ........", data[37], 16);
	int ctx_q = (data[2] >> 5) & 1;
	int ctx_nb = data[2] & 0b1111;

	// process vector
	pos += sprintf(b+pos, "Q:NB: %i:%i IC: 0x%04x R0: %s SR: %s\n", ctx_q, ctx_nb, data[0], r0s, srs);
	// user registers
	pos += sprintf(b+pos, "R1-7: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
	pos += sprintf(b+pos, "----------------------------------------------\n");

	char *name = r40_to_str(data+52, 2, NULL);
	char *state = int2binf("........ ........", data[12], 16);
	pos += sprintf(b+pos, "Process 0x%04x %s ", addr, name);
	pos += sprintf(b+pos, "State: %s (0x%04x), ", state, data[12]);
	pos += sprintf(b+pos, "Prio: %i, ", (int16_t) data[13]);
	pos += sprintf(b+pos, "Size: %iw/%iseg (%s) \n", data[54], data[34]&255, szabme);

	pos += sprintf(b+pos, "Next: 0x%04x ",  data[10]);
	pos += sprintf(b+pos, "Parent: 0x%04x ", data[15]);
	pos += sprintf(b+pos, "Children: 0x%04x Next Child: 0x%04x \n", data[16], data[11]);

	pos += sprintf(b+pos, "PID: 0x%04x \n", data[14]);
	pos += sprintf(b+pos, "ALLS: 0x%04x \n", data[17]);
	pos += sprintf(b+pos, "CHTIM: 0x%04x \n", data[18]);
	pos += sprintf(b+pos, "DEVI: 0x%04x ", data[19]);
	pos += sprintf(b+pos, "DEVO: 0x%04x \n", data[20]);
	pos += sprintf(b+pos, "USAL: 0x%04x \n", data[21]);
	// ROB (8 words)
	pos += sprintf(b+pos, "STRLI: 0x%04x \n", data[30]);
	pos += sprintf(b+pos, "BUFLI: 0x%04x \n", data[31]);
	pos += sprintf(b+pos, "LARUS: 0x%04x \n", data[32]);
	pos += sprintf(b+pos, "LISMEM: 0x%04x \n", data[33]);
	pos += sprintf(b+pos, "NXTMEM: 0x%04x \n", data[35]);
	pos += sprintf(b+pos, "BAR: 0x%04x \n", data[36]);
	pos += sprintf(b+pos, "BLPASC: 0x%04x \n", data[38]);
	pos += sprintf(b+pos, "IC: 0x%04x R0: 0x%04x SR: 0x%04x \n", data[39], data[40], data[41]);
	pos += sprintf(b+pos, "R1-7: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", data[42], data[43], data[44], data[45], data[46], data[47], data[48]);
	pos += sprintf(b+pos, "JDAD: 0x%04x \n", data[49]);
	pos += sprintf(b+pos, "Program start (JPAD): 0x%04x \n", data[50]);
	pos += sprintf(b+pos, "FILDIC position (JACN): 0x%04x \n", data[51]);
	pos += sprintf(b+pos, "TABUJB: 0x%04x \n", data[55]);

	free(r0s);
	free(srs);
	free(name);
	free(state);
	free(szabme);

	return buf;
}

// -----------------------------------------------------------------------
int decode_exl_fil(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[12];
	int pos = 0;
	mem_mget(nb, r4, data, 12);

	char *disk = r40_to_str(data+7, 1, NULL);
	char *dir = r40_to_str(data+8, 2, NULL);
	char *file = r40_to_str(data+10, 2, NULL);

	pos += sprintf(b+pos, "%s/%s/%s\n", disk, dir, file);
	pos += sprintf(b+pos, "Err: %i\n", (int16_t) data[0]);
	pos += sprintf(b+pos, "Stream ID: %i\n", data[1]);
	pos += sprintf(b+pos, "Type: %i\n", data[2]);
	pos += sprintf(b+pos, "Length: %i\n", data[3]);
	pos += sprintf(b+pos, "Parameter 1: 0x%04x (%i)\n", data[4], data[4]);
	pos += sprintf(b+pos, "Parameter 2: 0x%04x (%i)\n", data[5], data[5]);
	pos += sprintf(b+pos, "Attributes: 0x%04x\n", data[6]);

	free(disk);
	free(dir);
	free(file);

	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_proc(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[12];
	int pos = 0;
	mem_mget(nb, r4, data, 12);

	pos += sprintf(b+pos, "Err: %i\n", (int16_t) data[0]);
	pos += sprintf(b+pos, "PID: 0x%04x\n", data[1]);
	pos += sprintf(b+pos, "IC: 0x%04x R0: 0x%04x Prio/SR: 0x%04x\n", data[2], data[3], data[4]);
	pos += sprintf(b+pos, "Regs: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_rec(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[5];
	int pos = 0;
	mem_mget(nb, r4, data, 5);

	pos += sprintf(b+pos, "Start byte: %i\n", data[0]);
	pos += sprintf(b+pos, "Stream ID: %i\n", data[1]);
	pos += sprintf(b+pos, "Buf addr: 0x%04x\n", data[2]);
	pos += sprintf(b+pos, "Ending char: #%02x\n", data[3]>>8);
	pos += sprintf(b+pos, "Max bytes: %i\n", data[3]&255);

	// if we return from record write
	if ((exl_code == -139) || (exl_code == -141)) {
		int slen = data[0]/2+2;
		uint16_t *str = calloc(2, slen);
		mem_mget(nb, data[2], str, slen);
		pos += sprintf(b+pos, "Data: ");
		for (int i=0 ; i<slen ; i++) {
			if ((str[i]>>8) == (data[3]>>8)) break;
			pos += sprintf(b+pos, "%c", str[i]>>8);
			if ((str[i]&255) == (data[3]>>8)) break;
			pos += sprintf(b+pos, "%c", str[i]&255);
		}
		pos += sprintf(b+pos, "\n");
		free(str);
	}

	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_block(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[5];
	int pos = 0;
	mem_mget(nb, r4, data, 5);

	if (exl_code < 0) {
		pos += sprintf(b+pos, "Transmitted: %i\n", data[0]);
	}
	if ((abs(exl_code) == 150) || (abs(exl_code == 151))) {
		pos += sprintf(b+pos, "Stream ID: %i\n", data[1]);
	} else if ((abs(exl_code) == 148) || (abs(exl_code == 149))) {
		pos += sprintf(b+pos, "PID: %i\n", data[1]);
	} // ignored for OVL
	pos += sprintf(b+pos, "Buf addr: 0x%04x\n", data[2]);
	pos += sprintf(b+pos, "Count: %i\n", data[3]);
	pos += sprintf(b+pos, "Relative sector: %i\n", data[4]);
	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_met(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[5];
	int pos = 0;
	mem_mget(nb, r4, data, 5);

	char *disk = r40_to_str(data, 1, NULL);

	pos += sprintf(b+pos, "Disk: %s (%i)\n", disk, data[0]);
	pos += sprintf(b+pos, "DICDIC: %i\n", data[1]);
	pos += sprintf(b+pos, "FILDIC: %i\n", data[2]);
	pos += sprintf(b+pos, "MAP: %i\n", data[3]);
	pos += sprintf(b+pos, "Length: %i\n", data[4]);

	free(disk);

	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_pinf(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[12];
	int pos = 0;
	mem_mget(nb, r4, data, 12);

	char *area = r40_to_str(data+7, 1, NULL);
	char *u = r40_to_str(data+8, 2, NULL);
	char *p = r40_to_str(data+10, 2, NULL);

	pos += sprintf(b+pos, "System generation number: 0x%04x\n", data[0]);
	pos += sprintf(b+pos, "Mem available: %i segments\n", data[1]>>8);
	pos += sprintf(b+pos, "User rights: 0x%04x\n", data[1]&255);
	pos += sprintf(b+pos, "Priority: %i\n", data[2]);
	pos += sprintf(b+pos, "Special file len: %i sectors\n", data[3]);
	pos += sprintf(b+pos, "Load address: 0x%04x\n", data[4]);
	pos += sprintf(b+pos, "Mem used: %i words\n", data[5]);
	pos += sprintf(b+pos, "Rights: 0x%04x\n", data[6]);
	pos += sprintf(b+pos, "Area name: %s\n", area);
	pos += sprintf(b+pos, "User name: %s\n", u);
	pos += sprintf(b+pos, "Process name: %s\n", p);

	free(area);
	free(u);
	free(p);

	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_tmem(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[5];
	int pos = 0;
	mem_mget(nb, r4, data, 5);

	pos += sprintf(b+pos, "Err: %i\n", data[0]);
	pos += sprintf(b+pos, "Stream ID/PID: %i\n", data[1]);
	pos += sprintf(b+pos, "Addr: 0x%04x\n", data[2]);
	pos += sprintf(b+pos, "Segment number: %i\n", data[4]);
	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_time(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[3];
	int pos = 0;
	mem_mget(nb, r4, data, 3);
	if (exl_code < 0) {
		pos += sprintf(b+pos, "%02i:%02i:%02i", data[0], data[1], data[2]);
	}
	return pos;
}

// -----------------------------------------------------------------------
int decode_exl_date(int nb, uint16_t r4, char *b, int exl_code)
{
	uint16_t data[3];
	int pos = 0;
	mem_mget(nb, r4, data, 3);
	if (exl_code < 0) {
		pos += sprintf(b+pos, "%02i-%02i-%02i", data[0], data[1], data[2]);
	}
	return pos;
}

// -----------------------------------------------------------------------
char * decode_exl(int nb, uint16_t r4, int exl_code)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;
	int arg = exl_code;

	if (!buf) {
		return NULL;
	}

	if (arg < 0) {
		arg = -arg;
	}

	pos += sprintf(b+pos, "%s EXL %i @ 0x%04x ", exl_code<=0?"<-":"->", arg, r4);

	switch (arg) {
/**/		case 128: pos += sprintf(b+pos, "(ASG) Assign stream to file\n"); break;
/**/		case 129: pos += sprintf(b+pos, "(CASG) Create file and assign stream\n"); break;
/**/		case 130: pos += sprintf(b+pos, "(SETP) Set file parameters\n"); break;
/**/		case 131: pos += sprintf(b+pos, "(LOAP) Get file parameters\n"); break;
/**/		case 132: pos += sprintf(b+pos, "(TMEM) Allocate memory for process\n"); break;
/**/		case 133: pos += sprintf(b+pos, "(NASG) Add stream to file\n"); break;
		case 134: pos += sprintf(b+pos, "(ERF) Remove file\n"); break;
		case 135: pos += sprintf(b+pos, "(ERS) Remove stream\n"); break;
/**/		case 136: pos += sprintf(b+pos, "(ERAS) Remove streams\n"); break;
		case 137: pos += sprintf(b+pos, "(FBOF) Seek stream to beginning\n"); break;
/**/		case 138: pos += sprintf(b+pos, "(INPR) Read record\n"); break;
/**/		case 139: pos += sprintf(b+pos, "(PRIR) Write record\n"); break;
/**/		case 140: pos += sprintf(b+pos, "(PINP) Write 2-char and read record\n"); break;
/**/		case 141: pos += sprintf(b+pos, "(PRIN) Write record (no ending char)\n"); break;
		case 142: pos += sprintf(b+pos, "(EOF) Write end char\n"); break;
		case 143: pos += sprintf(b+pos, "(FEOF) Seek stream to end\n"); break;
/**/		case 144: pos += sprintf(b+pos, "(INAM) Get parameter from buffer\n"); break;
/**/		case 145: pos += sprintf(b+pos, "(INUM) Get number from buffer\n"); break;
/**/		case 146: pos += sprintf(b+pos, "(WADR) Write disk addresses\n"); break;
/**/		case 147: pos += sprintf(b+pos, "(OVL) Read overlay\n"); break;
/**/		case 148: pos += sprintf(b+pos, "(REAP) Read from special file\n"); break;
/**/		case 149: pos += sprintf(b+pos, "(WRIP) Write to process special file\n"); break;
/**/		case 150: pos += sprintf(b+pos, "(READ) Read block\n"); break;
/**/		case 151: pos += sprintf(b+pos, "(WRIT) Write block\n"); break;
		case 152: pos += sprintf(b+pos, "(OES) Set own alarm handler\n"); break;
/**/		case 153: pos += sprintf(b+pos, "(ERR) Handle last alarm\n"); break;
/**/		case 154: pos += sprintf(b+pos, "(CORE) Allocate memory\n"); break;
/**/		case 155: pos += sprintf(b+pos, "(CPRF) Create special file\n"); break;
/**/		case 156: pos += sprintf(b+pos, "(JUMP) Move process to separate block\n"); break;
		case 157: pos += sprintf(b+pos, "(SDIR) Set directory parameters\n"); break;
		case 158: pos += sprintf(b+pos, "(TDIR) Get directory parameters\n"); break;
		case 159: pos += sprintf(b+pos, "(CDIR) Change directory parameters\n"); break;
/**/		case 160: pos += sprintf(b+pos, "(DEFP) Define child process\n"); break;
/**/		case 161: pos += sprintf(b+pos, "(DELP) Remove child process\n"); break;
/**/		case 162: pos += sprintf(b+pos, "(SREG) Set child process registers\n"); break;
/**/		case 163: pos += sprintf(b+pos, "(TREG) Get child process registers\n"); break;
/**/		case 164: pos += sprintf(b+pos, "(RUNP) Start child process\n"); break;
/**/		case 165: pos += sprintf(b+pos, "(HANG) Stop child process\n"); break;
		case 166: pos += sprintf(b+pos, "(TERR) Check children alarm list\n"); break;
/**/		case 167: pos += sprintf(b+pos, "(WAIT) Wait %i quants\n", r4); break;
/**/		case 168: pos += sprintf(b+pos, "(STOP) Stop (and wait %i quants)\n", r4); break;
/**/		case 169: pos += sprintf(b+pos, "(RELD) Release character devices\n"); break;
/**/		case 170: pos += sprintf(b+pos, "(DATE) Get date\n"); break;
/**/		case 171: pos += sprintf(b+pos, "(TIME) Get time\n"); break;
/**/		case 173: pos += sprintf(b+pos, "(CHPI) Change priority by %i\n", r4); break;
/**/		case 174: pos += sprintf(b+pos, "(WAIS) Semaphore wait: 0x%04x\n", r4); break;
/**/		case 175: pos += sprintf(b+pos, "(SIGN) Semaphore signal: 0x%04x\n", r4); break;
/**/		case 176: pos += sprintf(b+pos, "(TLAB) Get disk label\n"); break;
/**/		case 177: pos += sprintf(b+pos, "(PINF) Get process info\n"); break;
/**/		case 178: pos += sprintf(b+pos, "(CSUM) Check OS control sum: 0x%04x\n", r4); break;
		case 179: pos += sprintf(b+pos, "(CSYS) Change system\n"); break;
		case 180: pos += sprintf(b+pos, "(UNL) Unloaddisk area\n"); break;
		case 181: pos += sprintf(b+pos, "(LOD) Load disk area\n"); break;
		case 182: pos += sprintf(b+pos, "(TAKS) Take stream semaphore\n"); break;
		case 183: pos += sprintf(b+pos, "(RELS) Release stream semaphore\n"); break;
/**/		case 184: pos += sprintf(b+pos, "(GMEM) Add memory segmets\n"); break;
/**/		case 185: pos += sprintf(b+pos, "(RMEM) Free memory segments\n"); break;
		case 186: pos += sprintf(b+pos, "(LRAM) Get RAM file parameters\n"); break;
		case 189: pos += sprintf(b+pos, "(OPPI) PI operation\n"); break;
		case 190: pos += sprintf(b+pos, "(WFPI) Get PI interrupt\n"); break;
		case 191: pos += sprintf(b+pos, "(CAMAC) CAMAC operation\n"); break;
		case 192: pos += sprintf(b+pos, "(RWMT) Rewind tape to the beginning\n"); break;
		case 193: pos += sprintf(b+pos, "(FBMT) Rewind tape by one file\n"); break;
		case 194: pos += sprintf(b+pos, "(FFMT) Forward tape by one file\n"); break;
		case 195: pos += sprintf(b+pos, "(BBMT) Rewind tape one block\n"); break;
		case 196: pos += sprintf(b+pos, "(BFMT) Forward tape one block\n"); break;
		case 197: pos += sprintf(b+pos, "(FMMT) Write file mark\n"); break;
		case 198: pos += sprintf(b+pos, "(REMT) Read block from tape\n"); break;
		case 199: pos += sprintf(b+pos, "(WRMT) Write block to tape\n"); break;
		case 249: pos += sprintf(b+pos, "(SCON) Set XOSL state word bits\n"); break;
		case 250: pos += sprintf(b+pos, "(TCON) Get XOSL state word\n"); break;
/**/		case 251: pos += sprintf(b+pos, "(END) End program 0x%04x\n", r4); break;
/**/		case 252: pos += sprintf(b+pos, "(BACK) Run in background\n"); break;
/**/		case 253: pos += sprintf(b+pos, "(ABO) Abort program: 0x%04x\n", r4); break;
/**/		case 254: pos += sprintf(b+pos, "(KILL) Kill program: 0x%04x\n", r4); break;
/**/		case 255: pos += sprintf(b+pos, "(EOSL) End program, output message: 0x%04x\n", r4); break;

		default:
			pos += sprintf(b+pos, "unknown\n");
			break;
	}

	if (((arg >= 128) && (arg <= 131)) || (arg == 133) || (arg == 155)) {
		pos += decode_exl_fil(nb, r4, b+pos, exl_code);
	} else if (arg == 132) {
		pos += decode_exl_tmem(nb, r4, b+pos, exl_code);
	} else if (((arg >= 138) && (arg <= 141)) || (arg == 144) || arg == 145) {
		pos += decode_exl_rec(nb, r4, b+pos, exl_code);
	} else if ((arg >= 146) && (arg <= 151)) {
		pos += decode_exl_block(nb, r4, b+pos, exl_code);
	} else if (((arg >= 160) && (arg <= 165)) || (arg == 156)) {
		pos += decode_exl_proc(nb, r4, b+pos, exl_code);
	} else if (arg == 154) {
		pos += sprintf(b+pos, "New block size: %i\n", r4);
	} else if ((arg == 184) || (arg == 185)) {
		char *smap = int2binf("........ ........", r4, 16);
		pos += sprintf(b+pos, "Segment map: %s\n", smap);
		free(smap);
	} else if (arg == 170) {
		pos += decode_exl_date(nb, r4, b+pos, exl_code);
	} else if (arg == 171) {
		pos += decode_exl_time(nb, r4, b+pos, exl_code);
	} else if (arg == 176) {
		pos += decode_exl_met(nb, r4, b+pos, exl_code);
	} else if (arg == 177) {
		pos += decode_exl_pinf(nb, r4, b+pos, exl_code);
	}

	return buf;
}

// -----------------------------------------------------------------------
int decode_mxpsuk_pl(struct mx_cf_sc_pl *pl, char *b)
{
	int pos = 0;
	if (pl->used) {
		pos += sprintf(b+pos, "  Direction: (%i) ", pl->dir);
		switch (pl->dir) {
			case MX_DIR_OUTPUT:
				pos += sprintf(b+pos, "output");
				break;
			case MX_DIR_INPUT:
				pos += sprintf(b+pos, "input");
				break;
			case MX_DIR_HALF_DUPLEX:
				pos += sprintf(b+pos, "half-duplex");
				break;
			case MX_DIR_FULL_DUPLEX:
				pos += sprintf(b+pos, "full-duplex");
				break;
			default:
				pos += sprintf(b+pos, "unknown");
				break;
		}
		pos += sprintf(b+pos, "\n");

		pos += sprintf(b+pos, "  Line type: (%i) ", pl->type);
		switch (pl->type) {
			case MX_PHY_USART_ASYNC:
				pos += sprintf(b+pos, "USART asynch");
				break;
			case MX_PHY_8255:
				pos += sprintf(b+pos, "8255 (parallel)");
				break;
			case MX_PHY_USART_SYNC:
				pos += sprintf(b+pos, "USART synch");
				break;
			case MX_PHY_WINCHESTER:
				pos += sprintf(b+pos, "winchester");
				break;
			case MX_PHY_MTAPE:
				pos += sprintf(b+pos, "magnetic tape");
				break;
			case MX_PHY_FLOPPY:
				pos += sprintf(b+pos, "floppy disk");
				break;
			default:
				pos += sprintf(b+pos, "unknown");
				break;
		}
		pos += sprintf(b+pos, "\n");

	} else {
		pos += sprintf(b+pos, "  not used\n");
	}
	return pos;
}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll_winch(struct mx_ll_winch *winch, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "    Winchester type: (%i) ", winch->type);
	switch (winch->type) {
		case MX_WINCH_BASF:
			pos += sprintf(b+pos, "BASF");
			break;
		case MX_WINCH_NEC:
			pos += sprintf(b+pos, "NEC");
			break;
		default:
			pos += sprintf(b+pos, "unknown");
			break;
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "    Formatting: %s\n", winch->format_protect ? "disallowed" : "allowed");
	return pos;
}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll_floppy(struct mx_ll_floppy *floppy, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "    Floppy type: (%i) ", floppy->type);
	switch (floppy->type) {
		case 0:
			pos += sprintf(b+pos, "40 cyl. (SD)");
			break;
		case 1:
			pos += sprintf(b+pos, "80 cyl. (DD)");
			break;
		case 2:
			pos += sprintf(b+pos, "80 cyl. (DD/HD)");
			break;
		default:
			pos += sprintf(b+pos, "unknown");
			break;
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "    Formatting: %s\n", floppy->format_protect ? "disallowed" : "allowed");
	return pos;

}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll(struct mx_cf_sc_ll *ll, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "  Physical line id: %i\n", ll->pl_id);
	pos += sprintf(b+pos, "  Protocol: (%i) ", ll->proto);
	switch (ll->proto) {
		case MX_PROTO_PUNCH_READER:
			pos += sprintf(b+pos, "punched tape reader\n");
			break;
		case MX_PROTO_PUNCHER:
			pos += sprintf(b+pos, "tape puncher\n");
			break;
		case MX_PROTO_TERMINAL:
			pos += sprintf(b+pos, "terminal\n");
			break;
		case MX_PROTO_SOM_PUNCH_READER:
			pos += sprintf(b+pos, "SOM punched tape reader\n");
			break;
		case MX_PROTO_SOM_PUNCHER:
			pos += sprintf(b+pos, "SOM tape puncher\n");
			break;
		case MX_PROTO_SOM_TERMINAL:
			pos += sprintf(b+pos, "SOM terminal\n");
			break;
		case MX_PROTO_WINCHESTER:
			pos += sprintf(b+pos, "winchester\n");
			if (ll->winch) {
				pos += decode_mxpsuk_ll_winch(ll->winch, b+pos);
			} else {
				pos += sprintf(b+pos, "Missing Winchester logical line description\n");
			}
			break;
		case MX_PROTO_MTAPE:
			pos += sprintf(b+pos, "magnetic tape\n");
			pos += sprintf(b+pos, "  Formatter: %i\n", ll->formatter);
			break;
		case MX_PROTO_FLOPPY:
			pos += sprintf(b+pos, "floppy disk\n");
			if (ll->floppy) {
				pos += decode_mxpsuk_ll_floppy(ll->floppy, b+pos);
			} else {
				pos += sprintf(b+pos, "Missing floppy logical line description\n");
			}
			break;
		case MX_PROTO_TTY_ITWL:
			pos += sprintf(b+pos, "TTY ITWL\n");
			break;
		default:
			pos += sprintf(b+pos, "unknown\n");
			break;
	}
	return pos;
}

// -----------------------------------------------------------------------
char * decode_mxpsuk(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;
	int i;

	if (!buf) {
		return NULL;
	}
	struct mx_cf_sc *uk = calloc(1, sizeof(struct mx_cf_sc));
	if (!uk) {
		pos += sprintf(b+pos, "Cannot allocate memory");
		return buf;
	}

	int res = mx_decode_cf_sc(addr, uk);

	if (res != E_OK) {
		pos += sprintf(b+pos, "ERROR DECODING CONFIGURATION FIELD, RESULTS ARE PROBABLY WRONG!\n");
		pos += sprintf(b+pos, "Error %i for line %i: ", uk->err_code, uk->err_line);
		switch (uk->err_code) {
			case MX_SC_E_CONFSET:			pos += sprintf(b+pos, "configuration already set\n"); break;
			case MX_SC_E_NUMLINES:			pos += sprintf(b+pos, "wrong number of physical or logical lines\n"); break;
			case MX_SC_E_DEVTYPE:			pos += sprintf(b+pos, "unknown device type in physical line description\n"); break;
			case MX_SC_E_DIR:				pos += sprintf(b+pos, "unknown transmission direction\n"); break;
			case MX_SC_E_PHY_INCOMPLETE:	pos += sprintf(b+pos, "incomplete physical line description\n"); break;
			case MX_SC_E_PROTO_MISSING:		pos += sprintf(b+pos, "missing protocol\n"); break;
			case MX_SC_E_PHY_UNUSED:		pos += sprintf(b+pos, "physical line is not used\n"); break;
			case MX_SC_E_DIR_MISMATCH:		pos += sprintf(b+pos, "device vs. protocol transmission dricetion mismatch\n"); break;
			case MX_SC_E_PHY_BUSY:			pos += sprintf(b+pos, "physical line is busy\n"); break;
			case MX_SC_E_NOMEM:				pos += sprintf(b+pos, "memory exhausted\n"); break;
			case MX_SC_E_PROTO_MISMATCH:	pos += sprintf(b+pos, "protocol vs. physical line type mismatch\n"); break;
			case MX_SC_E_PROTO_PARAMS:		pos += sprintf(b+pos, "wrong protocol parameters\n"); break;
			default:						pos += sprintf(b+pos, "unknown\b"); break;
		}
	} else {
		pos += sprintf(b+pos, "Field decoded correctly\n");
	}

	pos += sprintf(b+pos, "Line descriptions: physical = %i, logical = %i\n", uk->pl_desc_count, uk->ll_desc_count);
	pos += sprintf(b+pos, "-------------------------------------\n");

	if (!uk->pl) {
		pos += sprintf(b+pos, "Missing physical lines configuration.");
		return buf;
	}

	if (!uk->pl) {
		pos += sprintf(b+pos, "Missing logical lines configuration.");
		return buf;
	}

	int start_ll = 0;
	for (i=0 ; i<uk->pl_desc_count ; i++) {
		pos += sprintf(b+pos, "PHYSICAL LINES %i - %i (count: %i):\n", start_ll, start_ll+uk->pl[i].count-1, uk->pl[i].count);
		pos += decode_mxpsuk_pl((uk->pl)+i, b+pos);
		start_ll += uk->pl[i].count;
	}
	pos += sprintf(b+pos, "-------------------------------------\n");
	for (i=0 ; i<uk->ll_desc_count ; i++) {
		pos += sprintf(b+pos, "LOGICAL LINE %i\n", i);
		pos += decode_mxpsuk_ll((uk->ll)+i, b+pos);
	}

	mx_free_cf_sc(uk);

	return buf;
}

// -----------------------------------------------------------------------
char * decode_mxpsdl(int nb, uint16_t addr, int arg)
{
	return NULL;
}

// -----------------------------------------------------------------------
int decode_mxpst_transmit_winch(struct mx_winch_cf_transmit *t, char *b)
{
	int pos = 0;

	pos += sprintf(b+pos, "Starting logical sector: %i\n", t->sector);
	pos += sprintf(b+pos, "Destination: CPU=%i, NB=%i, ADDR=0x%04x, length=%i\n", t->cpu, t->nb, t->addr, t->len);
	pos += sprintf(b+pos, "Ignore CRC=%s, Fill last sector=%s, Watch EOF=%s\n", t->ign_crc?"y":"n", t->sector_fill?"y":"n", t->watch_eof?"y":"n");

	return pos;
}

// -----------------------------------------------------------------------
char * decode_mxpst_winch(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct mx_winch_cf_t *t = mx_winch_cf_t_decode(addr);
	if (!t) {
		pos += sprintf(b+pos, "Error decoding field");
		return buf;
	}

	pos += sprintf(b+pos, "Operation: (%i) ", t->oper);
	switch (t->oper) {
		case MX_WINCH_FORMAT_SPARE:
			pos += sprintf(b+pos, "Format spare area\n(no additional arguments)\n)");
			break;
		case MX_WINCH_FORMAT:
			pos += sprintf(b+pos, "Format track (optionally move sectors to spare area)\n");
			char *map = int2binf("................", t->format->sector_map, 16);
			pos += sprintf(b+pos, "Sector relocation map: %s\n", map);
			pos += sprintf(b+pos, "Starting sector: %i\n", t->format->start_sector);
			free(map);
			break;
		case MX_WINCH_READ:
			pos += sprintf(b+pos, "Read\n");
			pos += decode_mxpst_transmit_winch(t->transmit, buf+pos);
			break;
		case MX_WINCH_WRITE:
			pos += sprintf(b+pos, "Write\n");
			pos += decode_mxpst_transmit_winch(t->transmit, buf+pos);
			break;
		case MX_WINCH_PARK:
			pos += sprintf(b+pos, "Park\n");
			pos += sprintf(b+pos, "Cylinder: %i\n", t->park->cylinder);
			break;
		default:
			pos += sprintf(b+pos, "unknown\n");
			break;
	}

	mx_winch_cf_t_free(t);
	return buf;
}

// -----------------------------------------------------------------------
char * decode_mxpst_term(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct mx_term_cf_transmit_t *t = mx_term_cf_transmit_decode(addr);
	if (!t) {
		pos += sprintf(b+pos, "Error decoding field");
		return buf;
	}

	pos += sprintf(b+pos, "Send (%s%s%s): Len: %i, buf addr: %i : 0x%04x, eot char: #%i, start byte pos: %i\n",
		(t->opts & MX_TERM_TX_SEND_BY_SIZE) ? "by_size " : "x ",
		(t->opts & MX_TERM_TX_SEND_BY_EOT_EXCL) ? "by_eot- " : "x ",
		(t->opts & MX_TERM_TX_SEND_BY_EOT_INCL) ? "by_eot+" : "x",
		t->send_len,
		t->send_nb,
		t->send_buf_addr,
		t->send_eot_char,
		t->send_start_byte);

	pos += sprintf(b+pos, "Recv (%s%s%s): Len: %i, buf addr: %i : 0x%04x, eot char: #%i, eot char2: #%i, start byte pos: %i\n",
		(t->opts & MX_TERM_TX_RECV_BY_SIZE) ? "by_size " : "x ",
		(t->opts & MX_TERM_TX_RECV_BY_EOT_EXCL) ? "by_eot- " : "x ",
		(t->opts & MX_TERM_TX_RECV_BY_EOT_INCL) ? "by_eot+" : "x",
		t->recv_len,
		t->recv_nb,
		t->recv_buf_addr,
		t->recv_eot_char,
		t->recv_eot_char2,
		t->send_start_byte);

	pos += sprintf(b+pos, "Timeout: %i, Prompt: \"%s\", Flags: %s%s\n",
		t->timeout,
		t->prompt_text,
		(t->opts & MX_TERM_TX_ECHO) ? "echo " : "",
		(t->opts & MX_TERM_TX_PROMPT) ? "prompt " : "");

	mx_term_cf_transmit_free(t);
	return buf;
}

// -----------------------------------------------------------------------
char * decode_cmempst(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct cmem_m9425_cf_t *t = calloc(1, sizeof(struct cmem_m9425_cf_t));

	if (!t) {
		free(buf);
		return NULL;
	}

	int ret = cmem_m9425_decode_cf_t(addr, t);
	if (ret != E_OK) {
		pos += sprintf(b+pos, "Error decoding: %s", get_error(ret));
		return buf;
	}

	pos += sprintf(b+pos, "CF length: %i\n", t->cf_len);

	pos += sprintf(b+pos, "Operation: ");
	if (t->oper & 2) {
		pos += sprintf(b+pos, "write");
	} else {
		pos += sprintf(b+pos, "read");
	}
	if (t->oper & 1) {
		pos += sprintf(b+pos, " address");
	} else {
		pos += sprintf(b+pos, " data");
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "PLATTER: %i, HEAD: %i, CYL: %i, SECTOR: %i\n", t->platter, t->head, t->cyl, t->sector);
	pos += sprintf(b+pos, "Destination: CPU=%i, NB=%i, ADDR=%i, length=%i\n", t->cpu, t->nb, t->addr, t->len);
	pos += sprintf(b+pos, "Ignore: wprotect=%s, defects=%s, key=%s, eof=%s\n", t->ign_wrprotect?"y":"n", t->ign_defects?"y":"n", t->ign_key?"y":"n", t->ign_eof?"y":"n");
	pos += sprintf(b+pos, "Key: %i\n", t->key);

	free(t);

	return buf;
}

// vim: tabstop=4 shiftwidth=4 autoindent
