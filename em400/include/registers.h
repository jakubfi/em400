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

#ifndef REGISTERS_H
#define REGISTERS_H

#include <inttypes.h>

// -----------------------------------------------------------------------
// registers and "registers"
// -----------------------------------------------------------------------

enum _registers {
	R_R0 = 0,
	R_R1 = 1,
	R_R2 = 2,
	R_R3 = 3,
	R_R4 = 4,
	R_R5 = 5,
	R_R6 = 6,
	R_R7 = 7,
	R_IC = 8,
	R_SR = 9,
	R_IR = 10,
	R_KB = 11,
	R_MOD = 12,
	R_MODc = 13,
	R_P = 14,
	R_ZC17 = 15,
	R_MAX = 16
};

extern uint16_t regs[];

extern uint32_t RZ;

uint16_t reg_read(unsigned short int r, int trace);
void reg_write(unsigned short int r, uint16_t x, int trace);

#define R(x)		reg_read(x, 1)
#define nR(x)		reg_read(x, 0)
#define Rw(r, x)	reg_write(r, x, 1)
#define nRw(r, x)	reg_write(r, x, 0)

#define Rinc(r)		reg_write(r, R(r)+1, 1)
#define nRinc(r)	reg_write(r, nR(r)+1, 0)
#define Rdec(r)		reg_write(r, R(r)-1, 1)
#define nRdec(r)	reg_write(r, nR(r)-1, 0)
#define Radd(r, x)	reg_write(r, R(r)+x, 1)
#define nRadd(r, x)	reg_write(r, nR(r)+x, 0)

// -----------------------------------------------------------------------
// macros to access bit fields
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// SR
// -----------------------------------------------------------------------
#define SR_RM	((R(R_SR) & 0b1111111111000000) >> 6)
#define SR_RM0	((R(R_SR) & 0b1000000000000000) >> 15)
#define SR_RM1	((R(R_SR) & 0b0100000000000000) >> 14)
#define SR_RM2	((R(R_SR) & 0b0010000000000000) >> 13)
#define SR_RM3	((R(R_SR) & 0b0001000000000000) >> 12)
#define SR_RM4	((R(R_SR) & 0b0000100000000000) >> 11)
#define SR_RM5	((R(R_SR) & 0b0000010000000000) >> 10)
#define SR_RM6	((R(R_SR) & 0b0000001000000000) >> 9)
#define SR_RM7	((R(R_SR) & 0b0000000100000000) >> 8)
#define SR_RM8	((R(R_SR) & 0b0000000010000000) >> 7)
#define SR_RM9	((R(R_SR) & 0b0000000001000000) >> 6)
#define SR_Q	((R(R_SR) & 0b0000000000100000) >> 5)
#define SR_BS	((R(R_SR) & 0b0000000000010000) >> 4)
#define SR_NB	((R(R_SR) & 0b0000000000001111) >> 0)

#define SR_RM9cb	Rw(R_SR, R(R_SR) & 0b1111111110111111)
#define SR_Qcb		Rw(R_SR, R(R_SR) & 0b1111111111011111)

#define	SR_SET_QNB(x)	Rw(R_SR, (R(R_SR) & 0b111111111000000) | (x & 0b0000000000111111))
#define	SR_SET_MASK(x)	Rw(R_SR, (R(R_SR) & 0b000000000111111) | (x & 0b1111111111000000))

// -----------------------------------------------------------------------
// IR
// -----------------------------------------------------------------------
#define _OP(x)	((x & 0b1111110000000000) >> 10)
#define _D(x)	((x & 0b0000001000000000) >> 9)
#define _A(x)	((x & 0b0000000111000000) >> 6)
#define _B(x)	((x & 0b0000000000111000) >> 3)
#define _C(x)	((x & 0b0000000000000111) >> 0)
#define _T(x)	(int8_t) ((x & 0b0000000000111111) * (((x & 0b0000001000000000) >> 9) ? -1 : 1))
#define _t(x)	(uint8_t) ((x & 0b0000000000000111) | ((x & 0b0000001000000000) >> 6)) // only SHC uses it
#define _b(x)	(x & 0b0000000011111111)
#define IR_OP	_OP(R(R_IR))
#define IR_D	_D(R(R_IR))
#define IR_A	_A(R(R_IR))
#define IR_B	_B(R(R_IR))
#define IR_C	_C(R(R_IR))
#define IR_T	_T(R(R_IR))
#define IR_t	_t(R(R_IR))
#define IR_b	_b(R(R_IR))

// -----------------------------------------------------------------------
// R0
// -----------------------------------------------------------------------

// R0 - read
#define R0_Z	((R(0) & 0b1000000000000000) >> 15)
#define R0_M	((R(0) & 0b0100000000000000) >> 14)
#define R0_V	((R(0) & 0b0010000000000000) >> 13)
#define R0_C	((R(0) & 0b0001000000000000) >> 12)
#define R0_L	((R(0) & 0b0000100000000000) >> 11)
#define R0_E	((R(0) & 0b0000010000000000) >> 10)
#define R0_G	((R(0) & 0b0000001000000000) >> 9)
#define R0_Y	((R(0) & 0b0000000100000000) >> 8)
#define R0_X	((R(0) & 0b0000000010000000) >> 7)
#define R0_USER	((R(0) & 0b0000000001111111) >> 0)

// R0 - set
#define R0_Zsb	Rw(0, R(0) | 0b1000000000000000)
#define R0_Msb	Rw(0, R(0) | 0b0100000000000000)
#define R0_Vsb	Rw(0, R(0) | 0b0010000000000000)
#define R0_Csb	Rw(0, R(0) | 0b0001000000000000)
#define R0_Lsb	Rw(0, R(0) | 0b0000100000000000)
#define R0_Esb	Rw(0, R(0) | 0b0000010000000000)
#define R0_Gsb	Rw(0, R(0) | 0b0000001000000000)
#define R0_Ysb	Rw(0, R(0) | 0b0000000100000000)
#define R0_Xsb	Rw(0, R(0) | 0b0000000010000000)

// R0 - clear
#define R0_Zcb	Rw(0, R(0) & 0b0111111111111111)
#define R0_Mcb	Rw(0, R(0) & 0b1011111111111111)
#define R0_Vcb	Rw(0, R(0) & 0b1101111111111111)
#define R0_Ccb	Rw(0, R(0) & 0b1110111111111111)
#define R0_Lcb	Rw(0, R(0) & 0b1111011111111111)
#define R0_Ecb	Rw(0, R(0) & 0b1111101111111111)
#define R0_Gcb	Rw(0, R(0) & 0b1111110111111111)
#define R0_Ycb	Rw(0, R(0) & 0b1111111011111111)
#define R0_Xcb	Rw(0, R(0) & 0b1111111101111111)

// R0 - write
#define R0_USERw(x)	Rw(0, R(0) | x)

// R0 - set flags by x, y, z
#define R0_Cs16(z)	Rw(0, R(0) | (z & (0xffff+1)) >> 4);
#define R0_Cs32(z)	Rw(0, R(0) | (z & (0xffffffff+1)) >> (16+4));
#define R0_Cs64(z)	Rw(0, R(0) | (z & (0xffffffffff+1)) >> (8+16+4));

#define R0_Zs(z)	if (!z) R0_Zsb; else R0_Zcb;
#define R0_Zs16(z)	R0_Zs(z)
#define R0_Zs32(z)	R0_Zs(z)
#define R0_Zs64(z)	R0_Zs(z)

#define R0_Ms16(z)	Rw(0, R(0) | (z & 0x8000) >> 1);
#define R0_Ms32(z)	Rw(0, R(0) | (z & 0x80000000) >> (16+1));
#define R0_Ms64(z)	Rw(0, R(0) | (z & 0x8000000000) >> (8+16+1));

#define R0_Vs(x,y,z)	if (((x<0) && (y<0) && (z>0)) || ((x>0) && (y>0) && (z<0))) R0_Vsb; else R0_Vcb;
#define R0_Vs16(x,y,z)	R0_Vs(x,y,z)
#define R0_Vs32(x,y,z)	R0_Vs(x,y,z)
#define R0_Vs64(x,y,z)	R0_Vs(x,y,z)

#define R0_LEG(x,y) \
	if (x==y) { \
		R0_Esb; \
		R0_Gcb; \
		R0_Lcb; \
	} else { \
		R0_Ecb; \
		if (x<y) { \
			R0_Lsb; \
			R0_Gcb; \
		} else { \
			R0_Lcb; \
			R0_Gsb; \
		} \
	}

#endif

// vim: tabstop=4
