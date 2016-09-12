//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CPU_H
#define CPU_H

#include <inttypes.h>

#include <emawp.h>

#include "cfg.h"

enum cpu_states {
	STATE_QUIT = 0,
	STATE_START = 1,
	STATE_STOP = 2,
	STATE_HALT = 3,
};

enum cpu_registers {
	R_R0	= 0,
	R_R1	= 1,
	R_R2	= 2,
	R_R3	= 3,
	R_R4	= 4,
	R_R5	= 5,
	R_R6	= 6,
	R_R7	= 7,
	R_SR,
	R_IR,
	R_MAX
};

// -----------------------------------------------------------------------
// Flags in R0
// -----------------------------------------------------------------------

#define FL_Z	0b1000000000000000
#define FL_M	0b0100000000000000
#define FL_V	0b0010000000000000
#define FL_C	0b0001000000000000
#define FL_L	0b0000100000000000
#define FL_E	0b0000010000000000
#define FL_G	0b0000001000000000
#define FL_Y	0b0000000100000000
#define FL_X	0b0000000010000000

// -----------------------------------------------------------------------
// SR access macros
// -----------------------------------------------------------------------
#define Q	((regs[R_SR] & 0b0000000000100000) >> 5)
#define BS	((regs[R_SR] & 0b0000000000010000) >> 4)
#define NB	((regs[R_SR] & 0b0000000000001111) >> 0)
#define QNB	(Q*NB)

// -----------------------------------------------------------------------
// IR access macros
// -----------------------------------------------------------------------
#define _OP(x)	(((x) & 0b1111110000000000) >> 10)
#define _D(x)	(((x) & 0b0000001000000000) >> 9)
#define _A(x)	(((x) & 0b0000000111000000) >> 6)
#define _B(x)	(((x) & 0b0000000000111000) >> 3)
#define _C(x)	(((x) & 0b0000000000000111) >> 0)
#define _T(x)	(int8_t) (((x) & 0b0000000000111111) * (((x) & 0b0000001000000000) ? -1 : 1))
#define _t(x)	(uint8_t) (((x) & 0b0000000000000111) | (((x) & 0b0000001000000000) >> 6)) // only SHC uses it
#define _b(x)	((x) & 0b0000000011111111)
#define IR_OP	_OP(regs[R_IR])
#define IR_D	_D(regs[R_IR])
#define IR_A	_A(regs[R_IR])
#define IR_B	_B(regs[R_IR])
#define IR_C	_C(regs[R_IR])
#define IR_T	_T(regs[R_IR])
#define IR_t	_t(regs[R_IR])
#define IR_b	_b(regs[R_IR])

extern int cpu_state;
extern uint16_t regs[];
extern uint16_t rIC;
extern uint16_t rKB;
extern int rALARM;
extern uint16_t rMOD;
extern int rMODc;
extern int P;
extern uint32_t N;
extern int cpu_mod_active;

extern int cpu_mod_present;
extern int cpu_user_io_illegal;
extern int exit_on_hlt;
extern int cpu_awp;

extern struct awp *awp;

#define reg_restrict_write(r, x) regs[r] = ((r)|!Q) ? (x) : (regs[r] & 0b1111111100000000) | ((x) & 0b0000000011111111)
int cpu_init(struct cfg_em400 *cfg);
void cpu_shutdown();
void cpu_mod_enable();
int cpu_mod_on();
int cpu_mod_off();
void cpu_reset(int hw);
int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t sr_mask);
int cpu_ctx_restore();
unsigned int cpu_loop(int autotest);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
