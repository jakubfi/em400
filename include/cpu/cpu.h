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
	STATE_START =	0,
	STATE_STOP =	1 << 0,
	STATE_HALT =	1 << 1,
	STATE_CLM =		1 << 2,
	STATE_CLO =		1 << 3,
	STATE_QUIT =	1 << 10,
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
#define Q	((rSR & 0b0000000000100000) >> 5)
#define BS	((rSR & 0b0000000000010000) >> 4)
#define NB	((rSR & 0b0000000000001111) >> 0)
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
#define IR_OP	_OP(rIR)
#define IR_D	_D(rIR)
#define IR_A	_A(rIR)
#define IR_B	_B(rIR)
#define IR_C	_C(rIR)
#define IR_T	_T(rIR)
#define IR_t	_t(rIR)
#define IR_b	_b(rIR)

#define reg_restrict_write(r, x) regs[r] = ((r)|!Q) ? (x) : (regs[r] & 0b1111111100000000) | ((x) & 0b0000000011111111)

extern uint16_t regs[];
extern uint16_t rIC;
extern uint16_t rKB;
extern int rALARM;
extern uint16_t rMOD;
extern int rMODc;
extern uint16_t rIR;
extern uint16_t rSR;

extern int P;
extern uint32_t N;
extern int cpu_mod_active;

extern int cpu_mod_present;
extern int cpu_user_io_illegal;
extern int hlt_hack;

extern struct awp *awp;

int cpu_mem_get(int nb, uint16_t addr, uint16_t *data);
int cpu_mem_put(int nb, uint16_t addr, uint16_t data);
int cpu_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int cpu_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int cpu_mem_get_byte(int nb, uint32_t addr, uint8_t *data);
int cpu_mem_put_byte(int nb, uint32_t addr, uint8_t data);

int cpu_init(struct cfg_em400 *cfg, int new_ui);
void cpu_shutdown();

int cpu_mod_on();
int cpu_mod_off();

int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t sr_mask);
int cpu_ctx_restore();

unsigned cpu_loop(int autotest, int new_ui);

void cpu_trigger_quit();
void cpu_trigger_halt();
void cpu_trigger_stop();
void cpu_trigger_start();
void cpu_trigger_unhalt();
void cpu_trigger_clear(int scope);
int cpu_state_get();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
