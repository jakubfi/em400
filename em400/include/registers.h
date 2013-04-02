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

#ifndef REGISTERS_H
#define REGISTERS_H

#include <inttypes.h>

// -----------------------------------------------------------------------
// registers and "registers"
// -----------------------------------------------------------------------

enum _registers {
	R_R0	= 0,
	R_R1	= 1,
	R_R2	= 2,
	R_R3	= 3,
	R_R4	= 4,
	R_R5	= 5,
	R_R6	= 6,
	R_R7	= 7,
	R_IC,
	R_SR,
	R_IR,
	R_KB,
	R_MOD,
	R_MODc,
	R_ZC17,
	R_ALARM,
	R_MAX
};

extern uint16_t regs[];

#ifdef WITH_SPEEDOPT
#define reg_read(r, trace) regs[r]
#define reg_write(r, x, trace, hw) regs[r] = (r|hw) ? (x) : (regs[r] & 0b1111111100000000) | ((x) & 0b0000000011111111)
#else
uint16_t reg_read(int r, int trace);
void reg_write(int r, uint16_t x, int trace, int hw);
#endif

// -----------------------------------------------------------------------
// Register access macros
// -----------------------------------------------------------------------

#define R(x)		reg_read(x, 1)
#define nR(x)		reg_read(x, 0)
#define Rw(r, x)	reg_write(r, x, 1, 0)
#define nRw(r, x)	reg_write(r, x, 0, 0)

#define Rinc(r)		reg_write(r, R(r)+1, 1, 0)
#define nRinc(r)	reg_write(r, nR(r)+1, 0, 0)
#define Rdec(r)		reg_write(r, R(r)-1, 1, 0)
#define nRdec(r)	reg_write(r, nR(r)-1, 0, 0)
#define Radd(r, x)	reg_write(r, R(r)+x, 1, 0)
#define nRadd(r, x)	reg_write(r, nR(r)+x, 0, 0)

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
#define FL_USER	0b0000000001111111

// -----------------------------------------------------------------------
// SR access macros
// -----------------------------------------------------------------------
#define SR_Q	((nR(R_SR) & 0b0000000000100000) >> 5)
#define SR_NB	((nR(R_SR) & 0b0000000000001111) >> 0)

#define SR_RM9cb	Rw(R_SR, R(R_SR) & 0b1111111110111111)
#define SR_Qcb		Rw(R_SR, R(R_SR) & 0b1111111111011111)

#define	SR_SET_QNB(x)	Rw(R_SR, (R(R_SR) & 0b111111111000000) | (x & 0b0000000000111111))
#define	SR_SET_MASK(x)	Rw(R_SR, (R(R_SR) & 0b000000000111111) | (x & 0b1111111111000000))

// -----------------------------------------------------------------------
// IR access macros
// -----------------------------------------------------------------------
#define _OP(x)	((x & 0b1111110000000000) >> 10)
#define _D(x)	((x & 0b0000001000000000) >> 9)
#define _A(x)	((x & 0b0000000111000000) >> 6)
#define _B(x)	((x & 0b0000000000111000) >> 3)
#define _C(x)	((x & 0b0000000000000111) >> 0)
#define _T(x)	(int8_t) ((x & 0b0000000000111111) * ((x & 0b0000001000000000) ? -1 : 1))
#define _t(x)	(uint8_t) ((x & 0b0000000000000111) | ((x & 0b0000001000000000) >> 6)) // only SHC uses it
#define _b(x)	(x & 0b0000000011111111)
#define IR_OP	_OP(nR(R_IR))
#define IR_D	_D(nR(R_IR))
#define IR_A	_A(nR(R_IR))
#define IR_B	_B(nR(R_IR))
#define IR_C	_C(nR(R_IR))
#define IR_T	_T(nR(R_IR))
#define IR_t	_t(nR(R_IR))
#define IR_b	_b(nR(R_IR))

#endif

// vim: tabstop=4
