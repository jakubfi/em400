//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include <readline/readline.h>
#include <readline/history.h>
#include "utils.h"
#include "em400_debuger.h"
#include "mjc400_regs.h"
#include "mjc400.h"
#include "mjc400_dasm.h"
#include "mjc400_trans.h"
#include "mjc400_mem.h"

extern int em400_quit;

// -----------------------------------------------------------------------
cmd_s em400_debuger_commands[] = {
	{ "quit",	em400_debuger_c_quit,	"Quit the emulator" },
	{ "step",	em400_debuger_c_step,	"Execute instruction at IC" },
	{ "help",	em400_debuger_c_help,	"Print help" },
	{ "?",		em400_debuger_c_help,	"Print help" },
	{ "regs",	em400_debuger_c_regs,	"Show registers" },
	{ "reset",	em400_debuger_c_reset,	"Reset the emulator" },
	{ "dasm",	em400_debuger_c_dasm,	"Disassembly instruction at IC" },
	{ "trans",	em400_debuger_c_trans,	"Translate instruction at IC" },
	{ "mem",	em400_debuger_c_mem,	"Show memory contents (any block)" },
	{ "mem",	em400_debuger_c_memq,	"Show memory contents (block by Q)" },
	{ "mem",	em400_debuger_c_memnb,	"Show memory contents (block by NB)" },
	{ "clmem",	em400_debuger_c_clmem,	"Clear memory contents" },
	{ NULL,		NULL,			NULL }
};

// -----------------------------------------------------------------------
int em400_debuger_c_quit(char* args)
{
	em400_quit = 1;
	return 1;
}

// -----------------------------------------------------------------------
int em400_debuger_c_step(char* args)
{
	return 1;
}

// -----------------------------------------------------------------------
int em400_debuger_c_help(char* args)
{
	cmd_s *c = em400_debuger_commands;
	while (c->cmd) {
		printf("%10s %s\n", c->cmd, c->doc);
		c++;
	}
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_reset(char* args)
{
	mjc400_reset();
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_clmem(char* args)
{
	mjc400_clear_mem();
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_dasm(char* args)
{
	char *buf;
	int len = mjc400_dasm(MEMptr(IC), &buf);
	printf("0x%04x: (%i) %s\n", IC, len, buf);
	free(buf);
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_trans(char* args)
{
	char *buf;
	int len = mjc400_trans(MEMptr(IC), &buf);
	printf("0x%04x: (%i) %s\n", IC, len, buf);
	free(buf);
	return 0;
}

// -----------------------------------------------------------------------
void __em400_debuger_dump_mem(uint16_t *blockptr, uint16_t start, uint16_t count)
{
	uint16_t addr = start;
	char *text = malloc(MEMDUMP_COLS*2+1);
	char *tptr = text;
	char c1, c2;

	// TODO: not #define-able
	printf("        +0x0 +0x1 +0x2 +0x3 +0x4 +0x5 +0x6 +0x7 +0x8 +0x9 +0xa +0xb +0xc +0xd +0xe +0xf  |-text-dump--------------------|\n");

	while (addr < (start+count)) {
		// row header
		if ((addr-start)%MEMDUMP_COLS == 0) {
			printf("0x%04x: ", addr); 
		}
		// hex contents
		printf("%4x ", *(blockptr+addr));

		// store text representation
		c1 = (char) (((*(blockptr+addr))&0b111111110000000)>>8);
		if ((c1<32)||((c1>126)&&(c1<160))) c1 = '.';
		c2 = (char) ((*(blockptr+addr))&0b0000000011111111);
		if ((c2<32)||((c2>126)&&(c2<160))) c2 = '.';
		tptr += sprintf(tptr, "%c%c", c1, c2);

		// row footer - text representation
		if ((addr-start)%MEMDUMP_COLS == (MEMDUMP_COLS-1)) {
			printf(" %s\n", text);
			tptr = text;
		}

		addr++;
	}

	if ((addr-start-1)%MEMDUMP_COLS != (MEMDUMP_COLS-1)) {
		while ((addr-start)%MEMDUMP_COLS !=0) {
			printf("     ");
			addr++;
		}
		printf(" %s\n", text);
	}

	free(text);
}

// -----------------------------------------------------------------------
int em400_debuger_c_memq(char* args)
{
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_memnb(char* args)
{
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_mem(char* args)
{
	int m_block = -1;
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i %i", &m_block, &m_start, &m_end);

	// parse error
	if ((n<=1) || (n>3)) {
		printf("Syntax error. Use: mem block start [end]\n");
		return 0;
	}

	// wrong range
	if ((m_end >= 0) && (m_start > m_end)) {
		printf("Error: end>=start. Use: mem block start [end]\n");
		return 0;
	}

	// wrong block
	if ((m_block < 0) || (m_block > 15)) {
		printf("Error: 0 <= block <= 15. Use: mem block start [end]\n");
		return 0;
	}

	// only start position given, adjust end position
	if (n == 2) {
		m_end = m_start;
	}

	// system block
	if (m_block == 0) {
		__em400_debuger_dump_mem(mjc400_os_mem, m_start, 1+m_end-m_start);
	// user block
	} else {
		__em400_debuger_dump_mem(mjc400_user_mem[m_block], m_start, 1+m_end-m_start);
	}

	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_regs(char* args)
{
	char *ir = int2bin(IR, 16);
	char *sr = int2bin(SR, 16);
	char *rz = int2bin(RZ, 32);
	char *r0 = int2bin(R[0], 16);

	printf("           iiiiiiDAAABBBCCC             RM________QBNB__\n");
	printf("IR: 0x%04x %s  SR: 0x%04x %s\n", IR, ir, SR, sr);
	printf("IC: 0x%04x P: %i\n", IC, P);
	printf("\n");
	printf("    01234567012345670123456701234567      ZMVCLEGYX.......\n");
	printf("RZ: %s  R0: %s\n", rz, r0);
	printf("\n");
	free(ir);
	free(sr);
	free(rz);
	free(r0);
	printf("     hex... dec...  bin.....|.......       hex... dec...  bin.....|.......\n");
	for (int i=0 ; i<8 ; i++) {
		char *r1 = int2bin(R[2*i], 16);
		char *r2 = int2bin(R[2*i+1], 16);
		printf("R%02i: 0x%04x  %5i  %s", 2*i, R[2*i], R[2*i], r1);
		printf("  R%02i: 0x%04x  %5i  %s\n", 2*i+1, R[2*i+1], R[2*i+1], r2);
		free(r1);
		free(r2);
	}
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_execute(char* line)
{
	char cmd[10+1] = {0};
	char args[100+1] = {0};
	int res;

	res = sscanf(line, "%s %100c", cmd, args);
	if (res <= 0) {
		return 0;
	}

	cmd_s* c = em400_debuger_commands;

	while (c->cmd) {
		if (!strcmp(cmd, c->cmd)) {
			return c->fun(args);
		}
		c++;
	}
	printf("Unknown command: %s\n", cmd);
	return 0;
}
 
// -----------------------------------------------------------------------
void em400_debuger_step()
{
	char *buf;
	int done = 0;
 
	rl_bind_key('\t', rl_abort); // disable auto-complete

	while (!done)  {
		buf = readline("em400> ");
		if (!buf) {
			printf("\n");
			continue;
		} else {
			if (*buf != 0) {
				add_history(buf);
				if (em400_debuger_execute(buf)) {
					done = 1;
				}
			}
		}
		free(buf);
	}
}

