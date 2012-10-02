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

char *debuger_prompt;

// -----------------------------------------------------------------------
cmd_s em400_debuger_commands[] = {
	{ "exit",	em400_debuger_c_quit,	"Quit the emulator", "  exit" },
	{ "quit",	em400_debuger_c_quit,	"Quit the emulator", "  quit" },
	{ "step",	em400_debuger_c_step,	"Execute instruction at IC", "  step" },
	{ "help",	em400_debuger_c_help,	"Print help", "  help" },
	{ "?",		em400_debuger_c_help,	"Print help", "  ?" },
	{ "regs",	em400_debuger_c_regs,	"Show registers", "  regs" },
	{ "reset",	em400_debuger_c_reset,	"Reset the emulator", "  reset" },
	{ "dasm",	em400_debuger_c_dasm,	"Disassembler", "  dasm\n  dasm count\n  dasm start count" },
	{ "trans",	em400_debuger_c_trans,	"Translator", "  trans\n  trans count\n  trans start count" },
	{ "mem",	em400_debuger_c_mem,	"Show memory contents (any block)", "  mem block word_addr\n  mem block start_addr end_addr",  },
	{ "memq",	em400_debuger_c_memq,	"Show memory contents (block by Q,NB)", "  memq word_addr\n  memq start_addr end_addr" },
	{ "memnb",	em400_debuger_c_memnb,	"Show memory contents (block by NB)", "  memnb word_addr\n  memnb start_addr end_addr" },
	{ "clmem",	em400_debuger_c_clmem,	"Clear memory contents", "  clmem" },
	{ NULL,		NULL,			NULL, NULL }
};

// -----------------------------------------------------------------------
int em400_debuger_c_quit(char* args)
{
	return DEBUGER_EM400_QUIT;
}

// -----------------------------------------------------------------------
int em400_debuger_c_step(char* args)
{
	return DEBUGER_EM400_STEP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_help(char* args)
{
	cmd_s *c = em400_debuger_commands;
	if (args && *args) {
		while (c->cmd) {
			if (!strcmp(args, c->cmd)) {
				printf("%s : %s\nUsage:\n%s\n", c->cmd, c->doc, c->help);
				return DEBUGER_EM400_SKIP;
			}
			c++;
		}
		return DEBUGER_LOOP_ERR;
	} else {
		while (c->cmd) {
			printf("%-10s : %s\n", c->cmd, c->doc);
			c++;
		}
	}
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_reset(char* args)
{
	mjc400_reset();
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_clmem(char* args)
{
	mjc400_clear_mem();
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int __em400_debuger_c_dasmtrans(char* args, int dasm_fun(uint16_t* memptr, char **buf))
{
	int d_start;
	int d_count;

	int n = sscanf(args, "%i %i", &d_start, &d_count);

	if (n<=0) {
		d_count = 1;
		d_start = IC;
	} else if (n==1) {
		d_count = d_start;
		d_start = IC;
	} else if (n==2) {
	} else {
		printf("Syntax error.\n");
	}

	char *buf;
	int len;
	while (d_count >0) {
		len = dasm_fun(MEMptr(d_start), &buf);
		printf("0x%04x: (%i) %s\n", d_start, len, buf);
		d_start += len;
		d_count--;
		free(buf);
	}
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_dasm(char* args)
{
	return __em400_debuger_c_dasmtrans(args, mjc400_dasm);
}

// -----------------------------------------------------------------------
int em400_debuger_c_trans(char* args)
{
	return __em400_debuger_c_dasmtrans(args, mjc400_trans);
}

// -----------------------------------------------------------------------
int __em400_debuger_dump_mem(int block, int start, int end)
{
	uint16_t addr = start;
	char *text = malloc(MEMDUMP_COLS*2+1);
	char *tptr = text;
	char c1, c2;
	uint16_t *blockptr;

	if (block < 0) {
		return DEBUGER_LOOP_ERR;
	} else if (block == 0) {
		blockptr = mjc400_os_mem;
	} else if (block < 16) {
		blockptr = mjc400_user_mem[block];
	} else {
		return DEBUGER_LOOP_ERR;
	}

	// wrong range
	if ((end >= 0) && (start > end)) {
		return DEBUGER_LOOP_ERR;
	}

	// only start position given, adjust end position
	if (end < 0) {
		end = start;
	}

	// print headers, mind MEMDUMP_COLS
	printf("  addr: ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		printf("+%03x ", i);
	}
	printf("\n");

	while (addr <= end) {
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

	// fill and finish current line
	if ((addr-start-1)%MEMDUMP_COLS != (MEMDUMP_COLS-1)) {
		while ((addr-start)%MEMDUMP_COLS !=0) {
			printf("     ");
			addr++;
		}
		printf(" %s\n", text);
	}

	free(text);

	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_memq(char* args)
{
	int m_block;
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i", &m_start, &m_end);

	// parse error
	if ((n<1) || (n>2)) {
		return DEBUGER_LOOP_ERR;
	}

	// set block
	if (SR_Q == 0) {
		m_block = 0;
	} else {
		m_block = SR_NB;
	}

	return __em400_debuger_dump_mem(m_block, m_start, m_end);
}

// -----------------------------------------------------------------------
int em400_debuger_c_memnb(char* args)
{
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i", &m_start, &m_end);

	// parse error
	if ((n<1) || (n>2)) {
		return DEBUGER_LOOP_ERR;
	}

	return __em400_debuger_dump_mem(SR_NB, m_start, m_end);
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
		return DEBUGER_LOOP_ERR;
	}

	return __em400_debuger_dump_mem(m_block, m_start, m_end);
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
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_execute(char* line)
{
	char cmd[10+1] = {0};
	char args[100+1] = {0};
	cmd_s* c = em400_debuger_commands;

	int n;
	n = sscanf(line, "%s %100c", cmd, args);
	if (n <= 0) {
		return DEBUGER_EM400_SKIP;
	}

	while (c->cmd) {
		if (!strcmp(cmd, c->cmd)) {
			int ret;
			ret = c->fun(args);
			if (ret == DEBUGER_LOOP_ERR) {
				printf("Syntax error. Usage:\n%s\n", c->help);
			}
			return ret;
		}
		c++;
	}

	printf("Unknown command: '%s'. Try 'help'.\n", cmd);
	return DEBUGER_EM400_SKIP;
}
 
// -----------------------------------------------------------------------
int em400_debuger_setup()
{
	debuger_prompt = malloc(32);
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_step()
{
	int ret = DEBUGER_EM400_STEP;
	char *buf;
	sprintf(debuger_prompt, "em400 [%1i %02i 0x%04x]> ", SR_Q, SR_NB, IC);
 
	rl_bind_key('\t', rl_abort); // disable auto-complete

	buf = readline(debuger_prompt);
	if (!buf) {
		printf("\n");
		ret = DEBUGER_EM400_SKIP;
	} else {
		if (*buf != 0) {
			add_history(buf);
			ret = em400_debuger_execute(buf);
		} else {
			ret = DEBUGER_EM400_SKIP;
		}
	}
	free(buf);
	return ret;
}

