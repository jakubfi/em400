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
	printf("This is help.\n");
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_reset(char* args)
{
	mjc400_reset();
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
int em400_debuger_c_mem(char* args)
{
	int m_block = 0;
	int m_start = 0;
	int m_end = 0;
	int n = sscanf(args, "%i:%i-%i", &m_block, &m_start, &m_end);
	if ((n<=1) || (n>3)) {
		printf("Syntax error. Use: mem block:start_addr[-end_addr]\n");
		return 0;
	} else if (n == 2) {
		int16_t data;
		if (m_block < 0) {
			printf("Syntax error. Use: mem block:start_addr[-end_addr]\n");
			return 0;
		} else if (m_block == 0) {
			data = mjc400_os_mem[m_start];
		} else if (m_block <= 15) {
			data = mjc400_user_mem[SR_NB][m_start];
		} else {
			printf("Syntax error. Use: mem block:start_addr[-end_addr]\n");
			return 0;
		}
		printf("%02i 0x%04x: %i (0x%04x)\n", m_block, m_start, data, data);
	} else if (n == 3) {
		if (m_start>m_end) {
			printf("Syntax error. Use: mem block:start_addr[-end_addr]\n");
			return 0;
		}
		printf("%02i 0x%04x - 0x%04x:\n", m_block, m_start, m_end);
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
cmd_s em400_debuger_commands[] = {
	{ "quit",	em400_debuger_c_quit,	"Quit the emulator" },
	{ "step",	em400_debuger_c_step,	"Execute next instruction" },
	{ "help",	em400_debuger_c_help,	"Print help" },
	{ "?",		em400_debuger_c_help,	"Synonym for 'help'" },
	{ "regs",	em400_debuger_c_regs,	"Print registers" },
	{ "reset",	em400_debuger_c_reset,	"Reset the emulator" },
	{ "dasm",	em400_debuger_c_dasm,	"Disassembly instruction" },
	{ "trans",	em400_debuger_c_trans,	"Translate instruction" },
	{ "mem",	em400_debuger_c_mem,	"Memory contents" },
	{ NULL,		NULL,	NULL }
};

// -----------------------------------------------------------------------
int em400_debuger_execute(char* line)
{
	char cmd[10+1] = {0};
	char args[100+1] = {0};
	int res;

	res = sscanf(line, "%s %100s", cmd, args);

	cmd_s* cmd_pos = em400_debuger_commands;

	while (cmd_pos->cmd) {
		if (!strcmp(cmd, cmd_pos->cmd)) {
			return cmd_pos->fun(args);
		}
		cmd_pos++;
	}
	printf("Unknown command: %s\n", cmd);
	return 0;
}
 
// -----------------------------------------------------------------------
void em400_debuger_step()
{
	char *buf;
	int done = 0;
 
	rl_bind_key('\t', rl_abort); //disable auto-complete

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

