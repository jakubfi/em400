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

#ifndef MJC400_REGS_H
#define MJC400_REGS_H

#include <inttypes.h>

// -----------------------------------------------------------------------
// registers and "registers"
// -----------------------------------------------------------------------
extern uint16_t IC;
extern uint16_t SR;
extern uint16_t R[];
extern uint32_t RZ;
extern uint16_t IR;
extern unsigned short int P;
extern int16_t MOD;
extern unsigned short int MODcnt;
extern uint16_t KB;

// -----------------------------------------------------------------------
// macros to access bit fields
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// SR
// -----------------------------------------------------------------------
#define SR_RM	((SR & 0b1111111111000000) >> 6)
#define SR_RM0	((SR & 0b1000000000000000) >> 15)
#define SR_RM1	((SR & 0b0100000000000000) >> 14)
#define SR_RM2	((SR & 0b0010000000000000) >> 13)
#define SR_RM3	((SR & 0b0001000000000000) >> 12)
#define SR_RM4	((SR & 0b0000100000000000) >> 11)
#define SR_RM5	((SR & 0b0000010000000000) >> 10)
#define SR_RM6	((SR & 0b0000001000000000) >> 9)
#define SR_RM7	((SR & 0b0000000100000000) >> 8)
#define SR_RM8	((SR & 0b0000000010000000) >> 7)
#define SR_RM9	((SR & 0b0000000001000000) >> 6)
#define SR_Q	((SR & 0b0000000000100000) >> 5)
#define SR_BS	((SR & 0b0000000000010000) >> 4)
#define SR_NB	((SR & 0b0000000000001111) >> 0)

#define SR_RM0sb	SR |= 0b1000000000000000
#define SR_RM1sb	SR |= 0b0100000000000000
#define SR_RM2sb	SR |= 0b0010000000000000
#define SR_RM3sb	SR |= 0b0001000000000000
#define SR_RM4sb	SR |= 0b0000100000000000
#define SR_RM5sb	SR |= 0b0000010000000000
#define SR_RM6sb	SR |= 0b0000001000000000
#define SR_RM7sb	SR |= 0b0000000100000000
#define SR_RM8sb	SR |= 0b0000000010000000
#define SR_RM9sb	SR |= 0b0000000001000000
#define SR_Qsb		SR |= 0b0000000000100000

#define SR_RM0cb	SR &= 0b0111111111111111
#define SR_RM1cb	SR &= 0b1011111111111111
#define SR_RM2cb	SR &= 0b1101111111111111
#define SR_RM3cb	SR &= 0b1110111111111111
#define SR_RM4cb	SR &= 0b1111011111111111
#define SR_RM5cb	SR &= 0b1111101111111111
#define SR_RM6cb	SR &= 0b1111110111111111
#define SR_RM7cb	SR &= 0b1111111011111111
#define SR_RM8cb	SR &= 0b1111111101111111
#define SR_RM9cb	SR &= 0b1111111110111111
#define SR_Qcb		SR &= 0b1111111111011111

#define SR_RMw(x)	SR &= (SR & 0b0000000000111111) | (x & 0b1111111111000000)
#define SR_MBw(x)	SR &= (SR & 0b1111111111000000) | (x & 0b0000000000111111)

// -----------------------------------------------------------------------
// IR
// -----------------------------------------------------------------------
#define _OP(x)	((x & 0b1111110000000000) >> 10)
#define _D(x)	((x & 0b0000001000000000) >> 9)
#define _A(x)	((x & 0b0000000111000000) >> 6)
#define _B(x)	((x & 0b0000000000111000) >> 3)
#define _C(x)	((x & 0b0000000000000111) >> 0)
#define _T(x)	((int8_t) (x & 0b0000000000111111) | ((x & 0b0000001000000000) >> 2))
#define _t(x)	((int8_t) (x & 0b0000000000000111) | ((x & 0b0000001000000000) >> 6)) // only SHC uses it
#define _b(x)	(x & 0b0000000011111111)
#define IR_OP	_OP(IR)
#define IR_D	_D(IR)
#define IR_A	_A(IR)
#define IR_B	_B(IR)
#define IR_C	_C(IR)
#define IR_T	_T(IR)
#define IR_t	_t(IR)
#define IR_b	_b(IR)

// -----------------------------------------------------------------------
// R0
// -----------------------------------------------------------------------

// R0 - read
#define R0_Z	((R[0] & 0b1000000000000000) >> 15)
#define R0_M	((R[0] & 0b0100000000000000) >> 14)
#define R0_V	((R[0] & 0b0010000000000000) >> 13)
#define R0_C	((R[0] & 0b0001000000000000) >> 12)
#define R0_L	((R[0] & 0b0000100000000000) >> 11)
#define R0_E	((R[0] & 0b0000010000000000) >> 10)
#define R0_G	((R[0] & 0b0000001000000000) >> 9)
#define R0_Y	((R[0] & 0b0000000100000000) >> 8)
#define R0_X	((R[0] & 0b0000000010000000) >> 7)
#define R0_USER	((R[0] & 0b0000000001111111) >> 0)

// R0 - set
#define R0_Zsb	R[0] |= 0b1000000000000000
#define R0_Msb	R[0] |= 0b0100000000000000
#define R0_Vsb	R[0] |= 0b0010000000000000
#define R0_Csb	R[0] |= 0b0001000000000000
#define R0_Lsb	R[0] |= 0b0000100000000000
#define R0_Esb	R[0] |= 0b0000010000000000
#define R0_Gsb	R[0] |= 0b0000001000000000
#define R0_Ysb	R[0] |= 0b0000000100000000
#define R0_Xsb	R[0] |= 0b0000000010000000

// R0 - clear
#define R0_Zcb	R[0] &= 0b0111111111111111
#define R0_Mcb	R[0] &= 0b1011111111111111
#define R0_Vcb	R[0] &= 0b1101111111111111
#define R0_Ccb	R[0] &= 0b1110111111111111
#define R0_Lcb	R[0] &= 0b1111011111111111
#define R0_Ecb	R[0] &= 0b1111101111111111
#define R0_Gcb	R[0] &= 0b1111110111111111
#define R0_Ycb	R[0] &= 0b1111111011111111
#define R0_Xcb	R[0] &= 0b1111111101111111

// R0 - write
#define R0_USERw(x)	R[0] |= x

// R0 - set flags by x, y, z
#define R0_Cs16(z)	R[0] |= (z & (0xffff+1)) >> 4;
#define R0_Cs32(z)	R[0] |= (z & (0xffffffff+1)) >> (16+4);
#define R0_Cs64(z)	R[0] |= (z & (0xffffffffff+1)) >> (8+16+4);

#define R0_Zs(z)	if (!z) R0_Zsb; else R0_Zcb;
#define R0_Zs16(z)	R0_Zs(z)
#define R0_Zs32(z)	R0_Zs(z)
#define R0_Zs64(z)	R0_Zs(z)

#define R0_Ms16(z)	R[0] |= (z & 0x8000) >> 1;
#define R0_Ms32(z)	R[0] |= (z & 0x80000000) >> (16+1);
#define R0_Ms64(z)	R[0] |= (z & 0x8000000000) >> (8+16+1);

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

// -----------------------------------------------------------------------
// RZ
// -----------------------------------------------------------------------
#define RZ_2sb		RZ |= 0b00100000000000000000000000000000
#define RZ_5sb		RZ |= 0b00000100000000000000000000000000
#define RZ_6sb		RZ |= 0b00000010000000000000000000000000
#define RZ_30sb		RZ |= 0b00000000000000000000000000000010
#define RZ_31sb		RZ |= 0b00000000000000000000000000000001
#define RZ_3031sb	RZ |= 0b00000000000000000000000000000011
#define RZ_30cb		RZ &= 0b11111111111111111111111111111101
#define RZ_31cb		RZ &= 0b11111111111111111111111111111110
#define RZ_3031cb	RZ &= 0b11111111111111111111111111111100

#endif

// vim: tabstop=4
