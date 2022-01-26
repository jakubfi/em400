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

#include "cpu/flags.h"
#include "cfg.h"

// -----------------------------------------------------------------------
// SR access macros
// -----------------------------------------------------------------------
#define SR_READ() (rm<<6 | q<<5 | bs<<4 | nb)
#define SR_WRITE(sr) \
	rm = (sr >> 6) & 0b1111111111; \
	q = (sr >> 5) & 1; \
	bs = (sr >> 4) & 1; \
	nb = sr & 0b1111

#define QNB	(q*nb)

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
#define IR_OP	_OP(ir)
#define IR_D	_D(ir)
#define IR_A	_A(ir)
#define IR_B	_B(ir)
#define IR_C	_C(ir)
#define IR_T	_T(ir)
#define IR_t	_t(ir)
#define IR_b	_b(ir)

#define REG_RESTRICT_WRITE(i, v) r[i] = ((i)|!q) ? (v) : (r[i] & 0b1111111100000000) | ((v) & 0b0000000011111111)

extern uint16_t r[8];
extern uint16_t ic, kb, ir, ar, ac;
extern int rALARM;
extern uint16_t rMOD;
extern int mc;
extern unsigned rm, q, bs, nb;
extern int p;

extern uint32_t N;

extern int cpu_mod_present;
extern int cpu_mod_active;
extern int cpu_user_io_illegal;
extern int awp_enabled;

int cpu_mem_get(int nb, uint16_t addr, uint16_t *data);
int cpu_mem_put(int nb, uint16_t addr, uint16_t data);
int cpu_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int cpu_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int cpu_mem_get_byte(int nb, uint32_t addr, uint8_t *data);
int cpu_mem_put_byte(int nb, uint32_t addr, uint8_t data);

int cpu_init(em400_cfg *cfg);
void cpu_shutdown();

int cpu_mod_on();
int cpu_mod_off();

int cpu_ctx_switch(uint16_t arg, uint16_t new_ic, uint16_t int_mask);
void cpu_ctx_restore();

void cpu_loop();

int cpu_state_change(int to, int from);
int cpu_state_get();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
