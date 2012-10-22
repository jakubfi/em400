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

#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "io.h"
#include "iset.h"
#include "instructions.h"
#include "utils.h"

int mjc400_op_illegal()
{
	return OP_ILLEGAL;
}

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_lw()
{
	R[IR_A] = (uint16_t) mjc400_get_eff_arg();
	return OP_OK;
}

int mjc400_op_tw()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = MEMNB(N);
	return OP_OK;
}

int mjc400_op_ls()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = (R[IR_A] & ~R[7]) | (N & R[7]);
	return OP_OK;
}

int mjc400_op_ri()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(R[IR_A], N);
	R[IR_A]++;
	return OP_OK;
}

int mjc400_op_rw()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, R[IR_A]);
	return OP_OK;
}

int mjc400_op_pw()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[IR_A]);
	return OP_OK;
}

int mjc400_op_rj()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = IC;
	IC = N;
	return OP_OK;
}

int mjc400_op_is()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((MEMNB(N) & R[IR_A]) == R[IR_A]) {
		P = 1;
		MEMNBw(N, (MEMNB(N) | R[IR_A]));
	}
	return OP_OK;
}

int mjc400_op_bb()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & N) == N) P = 1;
	return OP_OK;
}

int mjc400_op_bm()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((MEMNB(N) & R[IR_A]) == R[IR_A]) P = 1;
	return OP_OK;
}

int mjc400_op_bs()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & R[7]) == (N & R[7])) P = 1;
	return OP_OK;
}

int mjc400_op_bc()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & N) == N) P = 1;
	return OP_OK;
}

int mjc400_op_bn()
{
	uint16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & N) == 0) P = 1;
	return OP_OK;
}

int mjc400_op_ou()
{
	uint16_t N = mjc400_get_eff_arg();

	int is_mem = (N & 0b0000000000000001);
	int chan   = (N & 0b0000000000011110) >> 1;
	int unit   = (N & 0b0000000011100000) >> 5;
	int cmd    = (N & 0b1111111100000000) >> 8;

	int io_result = em400_io_dispatch(IO_OU, is_mem, chan, unit, cmd, R[IR_A]);

	IC = MEM(IC+io_result);

	return OP_OK;
}

int mjc400_op_in()
{
	uint16_t N = mjc400_get_eff_arg();

	int is_mem = (N & 0b0000000000000001);
	int chan   = (N & 0b0000000000011110) >> 1;
	int unit   = (N & 0b0000000011100000) >> 5;
	int cmd    = (N & 0b1111111100000000) >> 8;

	int io_result = em400_io_dispatch(IO_IN, is_mem, chan, unit, cmd, R[IR_A]);

	IC = MEM(IC+io_result);

	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------
int mjc400_op_37()
{
	return mjc400_iset_37[EXT_OP_37(IR)].op_fun();
}

int mjc400_op_37_ad()
{
	uint16_t N = mjc400_get_eff_arg();
	int32_t a1 = DWORD(R[1], R[2]);
	int32_t a2 = DWORD(MEM(N), MEM(N+1));
	int64_t res = a1 + a2;
	DWORDw(R[1], R[2], res);
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Cs32(res);
	R0_Vs32(a1, a2, res);
	return OP_OK;
}

int mjc400_op_37_sd()
{
	uint16_t N = mjc400_get_eff_arg();
	int32_t a1 = DWORD(R[1], R[2]);
	int32_t a2 = DWORD(MEM(N), MEM(N+1));
	int64_t res = a1 - a2;
	DWORDw(R[1], R[2], res);
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Cs32(res);
	R0_Vs32(a1, -a2, res);
	return OP_OK;
}

int mjc400_op_37_mw()
{
	uint16_t N = mjc400_get_eff_arg();
	int16_t m1 = R[2];
	int16_t m2 = MEM(N);
	int64_t res = m1 * m2;
	DWORDw(R[1], R[2], res);
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Vs32(m1, m2, res);
	return OP_OK;
}

int mjc400_op_37_dw()
{
	uint16_t N = mjc400_get_eff_arg();
	int32_t d1 = DWORD(R[1], R[2]);
	int16_t d2 = MEM(N);
	int32_t res = d1 / d2;
	R[2] = res;
	R[1] = d1 % d2;
	R0_Ms32(res);
	R0_Zs32(res);
	return OP_OK;
}

int mjc400_op_37_af()
{
	return OP_OK;
}

int mjc400_op_37_sf()
{
	return OP_OK;
}

int mjc400_op_37_mf()
{
	return OP_OK;
}

int mjc400_op_37_df()
{
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_aw()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t tmp = R[IR_A];
	int32_t res = N + tmp;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, tmp, res);
	R[IR_A] = (uint16_t) res;
	return OP_OK;
}

int mjc400_op_ac()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t tmp = R[IR_A];
	int32_t res = N + tmp + R0_C;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, tmp, res);
	R[IR_A] = (uint16_t) res;
	return OP_OK;
}

int mjc400_op_sw()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t tmp = R[IR_A];
	int32_t res = N - tmp;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, -tmp, res);
	R[IR_A] = (uint16_t) res;
	return OP_OK;
}

int mjc400_op_cw()
{
	uint16_t N = mjc400_get_eff_arg();
	R0_LEG((int16_t)R[IR_A], (int16_t)N);
	return OP_OK;
}

int mjc400_op_or()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] | N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_om()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) | R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_nr()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] & N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_nm()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) & R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_er()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] & ~N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_em()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) & ~R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_xr()
{
	uint16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] ^ N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_xm()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) ^ R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_cl()
{
	uint16_t N = mjc400_get_eff_arg();
	R0_LEG((uint16_t)R[IR_A], (uint16_t)N);
	return OP_OK;
}

int mjc400_op_lb()
{
	uint16_t N = mjc400_get_eff_arg();

	// N lsb=1 - right byte
	// N lsb=0 - left byte

	// we shift data 8 bits right if N0 = 0
	int shift = 8 * ~(N&1); // left=8, right=0

	// get the real address
	N >>= 1;

	R[IR_A] = (R[IR_A] & 0b1111111100000000) | ((MEMNB(N) >> shift) & 0b0000000011111111);
	return OP_OK;
}

int mjc400_op_rb()
{
	uint16_t N = mjc400_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;

	int16_t oval = MEMNB(N) & (0b1111111100000000 >> shift);
	int16_t val = (R[IR_A] & 0b0000000011111111) << shift;
	MEMNBw(N, oval | val);

	return OP_OK;
}

int mjc400_op_cb()
{
	uint16_t N = mjc400_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;
	R0_LEG((uint8_t)(R[IR_A]&0b0000000011111111), (uint8_t)((MEMNB(N) >> shift) & 0b0000000011111111));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_awt()
{
	int16_t tmp = R[IR_A];
	int32_t res = IR_T + tmp;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(IR_T, tmp, res);
	return OP_OK;
}

int mjc400_op_trb()
{
	R[IR_A] += IR_T;
	if (!R[IR_A]) P = 1;
	else P = 0;
	return OP_OK;
}

int mjc400_op_irb()
{
	R[IR_A] += 1;
	if (R[IR_A]) IC += IR_T;
	return OP_OK;
}

int mjc400_op_drb()
{
	R[IR_A] -= 1;
	if (R[IR_A]) IC += IR_T;
	return OP_OK;
}

int mjc400_op_cwt()
{
	R0_LEG((int16_t)R[IR_A], (int8_t)IR_T);
	return OP_OK;
}

int mjc400_op_lwt()
{
	R[IR_A] = IR_T;
	return OP_OK;
}

int mjc400_op_lws()
{
	R[IR_A] = MEM(IC+(IR_T));
	return OP_OK;
}

int mjc400_op_rws()
{
	MEMw(IC+IR_T, R[IR_A]);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_70()
{
	return mjc400_iset_70[EXT_OP_70(IR)].op_fun();
}

int mjc400_op_70_ujs()
{
	IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jls()
{
	if (R0_E) IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jes()
{
	if (R0_E) IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jgs()
{
	if (R0_G) IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jvs()
{
	if (R0_V) {
		IC += IR_T;
		R0_Vcb;
	}
	return OP_OK;
}

int mjc400_op_70_jxs()
{
	if (R0_X) IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jys()
{
	if (R0_Y) IC += IR_T;
	return OP_OK;
}

int mjc400_op_70_jcs()
{
	if (R0_C) IC += IR_T;
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_71()
{
	return mjc400_iset_71[EXT_OP_71(IR)].op_fun();
}

int mjc400_op_71_blc()
{
	if ((((R[0] & 0b1111111100000000) >> 8) & IR_b) != IR_b) P = 1;
	return OP_OK;
}

int mjc400_op_71_exl()
{
	uint16_t SP = em400_mem_read(0, 97);
	em400_mem_write(0, SP, IC);
	em400_mem_write(0, SP+1, R[0]);
	em400_mem_write(0, SP+2, SR);
	em400_mem_write(0, SP+3, IR_b);
	IC = em400_mem_read(0, 96);
	R[0] = 0;
	em400_mem_write(0, 97, SP+4);
	SR_RM9cb;
	SR_Qcb;
	return OP_OK;
}

int mjc400_op_71_brc()
{
	if (((R[0] & 0b0000000011111111) & IR_b) != IR_b) P = 1;
	return OP_OK;
}

int mjc400_op_71_nrf()
{
	// TODO
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_72()
{
	return mjc400_iset_72[EXT_OP_72(IR)].op_fun();
}

int mjc400_op_72_ric()
{
	R[IR_A] = IC;
	return OP_OK;
}

int mjc400_op_72_zlb()
{
	R[IR_A] &= 0b0000000011111111;
	return OP_OK;
}

int mjc400_op_72_sxu()
{
	if (R[IR_A] & 0b1000000000000000) R0_Zsb;
	else R0_Zcb;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_72_nga()
{
	R[IR_A] = ~R[IR_A] + 1;
	return OP_OK;
}

int mjc400_op_72_slz()
{
	if (R[IR_A] & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]<<1) & 0b1111111111111110;
	return OP_OK;
}

int mjc400_op_72_sly()
{
	if (R[IR_A] & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]<<1) & (0b1111111111111110 | R0_Y);
	return OP_OK;
}

int mjc400_op_72_slx()
{
	if (R[IR_A] & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]<<1) & (0b1111111111111110 | R0_X);
	return OP_OK;
}

int mjc400_op_72_sry()
{
	if (R[IR_A] & 1) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]>>1) & (0b0111111111111111 | (R0_Y<<15));
	return OP_OK;
}

int mjc400_op_72_ngl()
{
	R[IR_A] = ~ R[IR_A];
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_72_rpc()
{
	R[IR_A] = R[0];
	return OP_OK;
}

int mjc400_op_72_shc()
{
	if (!IR_t) return OP_OK;

	uint16_t falling = (R[IR_A] & (IR_t-1)) << (16-IR_t);
	
	R[IR_A] = (R[IR_A] >> IR_t) | falling;
	return OP_OK;
}

int mjc400_op_72_rky()
{
	// TODO: does it work that way?
	if (KB) {
		R[IR_A] = KB;
	}
	return OP_OK;
}

int mjc400_op_72_zrb()
{
	R[IR_A] &= 0b1111111100000000;
	return OP_OK;
}

int mjc400_op_72_sxl()
{
	if (R[IR_A] & 1) R0_Zsb;
	else R0_Zcb;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_72_ngc()
{
	R[IR_A] = ~R[IR_A] + R0_C;
	return OP_OK;
}

int mjc400_op_72_svz()
{
	if (R[IR_A] & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	R[IR_A] = (R[IR_A]<<1) & 0b1111111111111110;
	return OP_OK;
}

int mjc400_op_72_svy()
{
	if (R[IR_A] & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	R[IR_A] = (R[IR_A]<<1) & (0b1111111111111110 | R0_Y);
	return OP_OK;
}

int mjc400_op_72_svx()
{
	if (R[IR_A] & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	R[IR_A] = (R[IR_A]<<1) & (0b1111111111111110 | R0_X);
	return OP_OK;
}

int mjc400_op_72_srx()
{
	if (R[IR_A] & 1) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]>>1) & (0b0111111111111111 | (R0_X<<15));
	return OP_OK;
}

int mjc400_op_72_srz()
{
	if (R[IR_A] & 1) R0_Ysb;
	else R0_Ycb;
	R[IR_A] = (R[IR_A]>>1) & 0b0111111111111111;
	return OP_OK;
}

int mjc400_op_72_lpc()
{
	R[0] = R[IR_A];
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_73()
{
	// all 73-instructions are illegal in user mode
	if (SR_Q) return OP_ILLEGAL;
	return mjc400_iset_73[EXT_OP_73(IR)].op_fun();
}

int mjc400_op_73_hlt()
{
	// TODO: busy wait
	while (!RZ);
	return OP_OK;
}

int mjc400_op_73_mcl()
{
	RZ = 0;
	SR = 0;
	R[0] = 0;
	// zeruj kanały
	// zeruj urządzenia
	em400_mem_remove_user_maps();
	return OP_OK;
}

int mjc400_op_73_cit()
{
	INT_CLEAR(INT_SOFT_U | INT_SOFT_L);
	return OP_OK;
}

int mjc400_op_73_sil()
{
	INT_SET(INT_SOFT_L);
	return OP_OK;
}

int mjc400_op_73_siu()
{
	INT_SET(INT_SOFT_U);
	return OP_OK;
}

int mjc400_op_73_sit()
{
	INT_SET(INT_SOFT_U | INT_SOFT_L);
	return OP_OK;
}

int mjc400_op_73_giu()
{
	// TODO
	return OP_OK;
}

int mjc400_op_73_gil()
{
	// TODO
	return OP_OK;
}

int mjc400_op_73_lip()
{
	uint16_t SP = em400_mem_read(0, 97);
	IC = em400_mem_read(0, SP-4);
	R[0] = em400_mem_read(0, SP-3);
	SR = em400_mem_read(0, SP-2);
	em400_mem_write(0, 97, SP-4);
	return OP_OK;
}

int mjc400_op_73_six()
{
	INT_SET(INT_EXTRA);
	return OP_OK;
}

int mjc400_op_73_cix()
{
	INT_CLEAR(INT_EXTRA);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_74()
{
	return mjc400_iset_74[EXT_OP_74(IR)].op_fun();
}

int mjc400_op_74_uj()
{
	uint16_t N = mjc400_get_eff_arg();
	IC = N;
	return OP_OK;
}

int mjc400_op_74_jl()
{
	uint16_t N = mjc400_get_eff_arg();
	if (R0_L) IC = N;
	return OP_OK;
}

int mjc400_op_74_je()
{
	uint16_t N = mjc400_get_eff_arg();
	if (R0_E) IC = N;
	return OP_OK;
}

int mjc400_op_74_jg()
{
	uint16_t N = mjc400_get_eff_arg();
	if (R0_G) IC = N;
	return OP_OK;
}

int mjc400_op_74_jz()
{
	uint16_t N = mjc400_get_eff_arg();
	if (R0_Z) IC = N;
	return OP_OK;
}

int mjc400_op_74_jm()
{
	uint16_t N = mjc400_get_eff_arg();
	if (R0_M) IC = N;
	return OP_OK;
}

int mjc400_op_74_jn()
{
	uint16_t N = mjc400_get_eff_arg();
	if (!R0_E) IC = N;
	return OP_OK;
}

int mjc400_op_74_lj()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, IC);
	IC = N+1;
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_75()
{
	return mjc400_iset_75[EXT_OP_75(IR)].op_fun();
}

int mjc400_op_75_ld()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEM(N);
	R[2] = MEM(N+1);
	return OP_OK;
}

int mjc400_op_75_lf()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEM(N);
	R[2] = MEM(N+1);
	R[3] = MEM(N+2);
	return OP_OK;
}

int mjc400_op_75_la()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEM(N);
	R[2] = MEM(N+1);
	R[3] = MEM(N+2);
	R[4] = MEM(N+3);
	R[5] = MEM(N+4);
	R[6] = MEM(N+5);
	R[7] = MEM(N+6);
	return OP_OK;
}

int mjc400_op_75_ll()
{
	uint16_t N = mjc400_get_eff_arg();
	R[5] = MEM(N);
	R[6] = MEM(N+1);
	R[7] = MEM(N+2);
	return OP_OK;
}

int mjc400_op_75_td()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEMNB(N);
	R[2] = MEMNB(N+1);
	return OP_OK;
}

int mjc400_op_75_tf()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEMNB(N);
	R[2] = MEMNB(N+1);
	R[3] = MEMNB(N+2);
	return OP_OK;
}

int mjc400_op_75_ta()
{
	uint16_t N = mjc400_get_eff_arg();
	R[1] = MEMNB(N);
	R[2] = MEMNB(N+1);
	R[3] = MEMNB(N+2);
	R[4] = MEMNB(N+3);
	R[5] = MEMNB(N+4);
	R[6] = MEMNB(N+5);
	R[7] = MEMNB(N+6);
	return OP_OK;
}

int mjc400_op_75_tl()
{
	uint16_t N = mjc400_get_eff_arg();
	R[5] = MEMNB(N);
	R[6] = MEMNB(N+1);
	R[7] = MEMNB(N+2);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_76()
{
	return mjc400_iset_76[EXT_OP_76(IR)].op_fun();
}

int mjc400_op_76_rd()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, R[1]);
	MEMw(N+1, R[2]);
	return OP_OK;
}

int mjc400_op_76_rf()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, R[1]);
	MEMw(N+1, R[2]);
	MEMw(N+2, R[3]);
	return OP_OK;
}

int mjc400_op_76_ra()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, R[1]);
	MEMw(N+1, R[2]);
	MEMw(N+2, R[3]);
	MEMw(N+3, R[4]);
	MEMw(N+4, R[5]);
	MEMw(N+5, R[6]);
	MEMw(N+6, R[7]);
	return OP_OK;
}

int mjc400_op_76_rl()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, R[5]);
	MEMw(N+1, R[6]);
	MEMw(N+2, R[7]);
	return OP_OK;
}

int mjc400_op_76_pd()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[1]);
	MEMNBw(N+1, R[2]);
	return OP_OK;
}

int mjc400_op_76_pf()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[1]);
	MEMNBw(N+1, R[2]);
	MEMNBw(N+2, R[3]);
	return OP_OK;
}

int mjc400_op_76_pa()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[1]);
	MEMNBw(N+1, R[2]);
	MEMNBw(N+2, R[3]);
	MEMNBw(N+3, R[4]);
	MEMNBw(N+4, R[5]);
	MEMNBw(N+5, R[6]);
	MEMNBw(N+6, R[7]);
	return OP_OK;
}

int mjc400_op_76_pl()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[5]);
	MEMNBw(N+1, R[6]);
	MEMNBw(N+2, R[7]);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_77()
{
	return mjc400_iset_77[EXT_OP_77(IR)].op_fun();
}

int mjc400_op_77_mb()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = mjc400_get_eff_arg();
	SR_MBw(MEM(N));
	return OP_OK;
}

int mjc400_op_77_im()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = mjc400_get_eff_arg();
	SR_RMw(MEM(N));
	return OP_OK;
}

int mjc400_op_77_ki()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, ((RZ & 0b11111111111100000000000000000000) >> 16) | (RZ & 0b00000000000000000000000000001111));
	return OP_OK;
}

int mjc400_op_77_fi()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = mjc400_get_eff_arg();
	uint16_t RZM = MEM(N);
	RZ = RZ | (RZM & 0b0000000000001111);
	RZ = RZ | ((RZM & 0b1111111111110000) << 16);
	return OP_OK;
}

int mjc400_op_77_sp()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = mjc400_get_eff_arg();
	IC = MEMNB(N);
	R[0] = MEMNB(N+1);
	SR = MEMNB(N+2);
	return OP_OK;
}

int mjc400_op_77_md()
{
	MODcnt++;
	if (MODcnt >= 4) {
		return OP_ILLEGAL;
	}
	int16_t N = mjc400_get_eff_arg();
	MOD = N;
	return OP_MD;
}

int mjc400_op_77_rz()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, 0);
	return OP_OK;
}

int mjc400_op_77_ib()
{
	uint16_t N = mjc400_get_eff_arg();
	MEMw(N, MEM(N)+1);
	if (!MEM(N)) P = 1;
	return OP_OK;
}

// vim: tabstop=4
