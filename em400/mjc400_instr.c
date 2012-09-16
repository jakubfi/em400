//  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include "mjc400_instr.h"
#include "mjc400_mem.h"
#include "mjc400_regs.h"
#include "mjc400.h"

int mjc400_op_illegal()
{
	return OP_ILLEGAL;
}


// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_lw()
{
	R[IR_A] = mjc400_get_eff_arg();
	printf("LW %i\n", IR_A);
	return OP_OK;
}

int mjc400_op_tw()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = MEMNB(N);
	return OP_OK;
}

int mjc400_op_ls()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = (R[IR_A] & ~R[7]) | (N & R[7]);
	return OP_OK;
}

int mjc400_op_ri()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(R[IR_A], N);
	R[IR_A]++;
	return OP_OK;
}

int mjc400_op_rw()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, R[IR_A]);
	printf("RW %i %i %i\n", N, IR_A, R[IR_A]);
	return OP_OK;
}

int mjc400_op_pw()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[IR_A]);
	return OP_OK;
}

int mjc400_op_rj()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = IC;
	IC = N;
	return OP_OK;
}

int mjc400_op_is()
{
	int16_t N = mjc400_get_eff_arg();
	if ((MEMNB(N) & R[IR_A]) == R[IR_A]) {
		P = 1;
		MEMNBw(N, (MEMNB(N) | R[IR_A]));
	}
	return OP_OK;
}

int mjc400_op_bb()
{
	int16_t N = mjc400_get_eff_arg();
	if ((MEMNB(N) & R[IR_A]) == R[IR_A]) P = 1;
	return OP_OK;
}

int mjc400_op_bm()
{
	return OP_OK;
}

int mjc400_op_bs()
{
	int16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & R[7]) == (N & R[7])) P = 1;
	return OP_OK;
}

int mjc400_op_bc()
{
	int16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & N) == N) P = 1;
	return OP_OK;
}

int mjc400_op_bn()
{
	int16_t N = mjc400_get_eff_arg();
	if ((R[IR_A] & N) == 0) P = 1;
	return OP_OK;
}

int mjc400_op_ou()
{
	return OP_OK;
}

int mjc400_op_in()
{
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------
int mjc400_op_37()
{
	return OP_OK;
}

int mjc400_op_37_ad()
{
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();

	// get exp and fraction
	int8_t a1e = F_EXP(R[1], R[2], R[3]);
	int64_t a1f = F_FRAC(R[1], R[2], R[3]);
	int8_t a2e = F_EXP(MEM(N), MEM(N+1), MEM(N+2));
	int64_t a2f = F_FRAC(MEM(N), MEM(N+1), MEM(N+2));

	// offset
	int8_t ediff;
	if (a1e<a2e) {
		ediff = a2e-a1e;
		a1e += ediff;
		a1f /= 2^ediff;
	} else if (a1e>a2e) {
		ediff = a1e-a2e;
		a1e += ediff;
		a1f /= 2^ediff;
	}

	int8_t e = a1e;

	// add
	int64_t f = a1f + a2f;
	R0_Ms64(f);
	R0_Zs64(f);
	R0_Cs64(f);

	//normalize down
	while (abs(f) >= 2) {
		f /= 2;
		e -= 1;
	}

	// normalize up
	while (abs(f) < 1) {
		f *= 2;
		e += 1;
	}

	R[1] = (f&0b1111111111111111000000000000000000000000) >> (16+8);
	R[2] =                 (f&0b111111111111111100000000) >> 8;
	R[3] =                            e | ((f&0b11111111) << 8);

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
	R0_Ms16(res);
	R0_Zs16(res);
	R0_Cs16(res);
	R0_Vs16(N, tmp, res);
	R[IR_A] = (int16_t) res;
	printf("AW %i\n", IR_A);
	return OP_OK;
}

int mjc400_op_ac()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t tmp = R[IR_A];
	int32_t res = N + tmp + R0_C;
	R0_Ms16(res);
	R0_Zs16(res);
	R0_Cs16(res);
	R0_Vs16(N, tmp, res);
	R[IR_A] = (int16_t) res;
	return OP_OK;
}

int mjc400_op_sw()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t tmp = R[IR_A];
	int32_t res = N - tmp;
	R0_Ms16(res);
	R0_Zs16(res);
	R0_Cs16(res);
	R0_Vs16(N, -tmp, res);
	R[IR_A] = (int16_t) res;
	return OP_OK;
}

int mjc400_op_cw()
{
	int16_t N = mjc400_get_eff_arg();
	R0_LEG(R[IR_A], N);
	return OP_OK;
}

int mjc400_op_or()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] | N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_om()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) | R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_nr()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] & N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_nm()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) & R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_er()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] & ~N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_em()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) & ~R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_xr()
{
	int16_t N = mjc400_get_eff_arg();
	R[IR_A] = R[IR_A] ^ N;
	R0_Zs16(R[IR_A]);
	return OP_OK;
}

int mjc400_op_xm()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, MEMNB(N) ^ R[IR_A]);
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

int mjc400_op_cl()
{
	int16_t N = mjc400_get_eff_arg();
	R0_LEG(R[IR_A], N);
	return OP_OK;
}

int mjc400_op_lb()
{
	int16_t N = mjc400_get_eff_arg();

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
	int16_t N = mjc400_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;

	int16_t oval = MEMNB(N) & (0b1111111100000000 >> shift);
	int16_t val = (R[IR_A] & 0b0000000011111111) << shift;
	MEMNBw(N, oval | val);

	return OP_OK;
}

int mjc400_op_cb()
{
	int16_t N = mjc400_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;
	R0_LEG((R[IR_A]&0b0000000011111111), ((MEMNB(N) >> shift) & 0b0000000011111111));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_awt()
{
	int16_t tmp = R[IR_A];
	int32_t res = IR_T + tmp;
	R0_Ms16(res);
	R0_Zs16(res);
	R0_Cs16(res);
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
	if (!R[IR_A]) IC += IR_T;
	return OP_OK;
}

int mjc400_op_drb()
{
	R[IR_A] -= 1;
	if (!R[IR_A]) IC += IR_T;
	return OP_OK;
}

int mjc400_op_cwt()
{
	R0_LEG(R[IR_A], IR_T);
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
	return OP_OK;
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
	return OP_OK;
}

int mjc400_op_71_blc()
{
	if ((((R[0] & 0b1111111100000000) >> 8) & IR_b) != IR_b) P = 1;
	return OP_OK;
}

int mjc400_op_71_exl()
{
	uint16_t SP = mjc400_os_mem[97];
	mjc400_os_mem[SP] = IC;
	mjc400_os_mem[SP+1] = R[0];
	mjc400_os_mem[SP+2] = SR;
	mjc400_os_mem[SP+3] = IR_b;
	IC = mjc400_os_mem[96];
	R[0] = 0;
	mjc400_os_mem[97] = SP + 4;
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
	return OP_OK;
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
	if (!IR_T) return OP_OK;

	uint16_t falling = (R[IR_A] & (IR_T-1)) << (16-IR_T);
	
	R[IR_A] = (R[IR_A] >> IR_T) | falling;
	return OP_OK;
}

int mjc400_op_72_rky()
{
	R[IR_A] = KB;
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
	return OP_OK;
}

int mjc400_op_73_hlt()
{
	// TODO
	while (!SR_RM);
	return OP_ILLEGAL;
}

int mjc400_op_73_mcl()
{
	RZ = 0;
	SR = 0;
	R[0] = 0;
	// zeruj kanały
	// zeruj urządzenia
	// zeruj rejestry podziału PAO (jeśli będziemy dzielić)
	return OP_OK;
}

int mjc400_op_73_cit()
{
	RZ_3031cb;
	return OP_OK;
}

int mjc400_op_73_sil()
{
	RZ_31sb;
	return OP_OK;
}

int mjc400_op_73_siu()
{
	RZ_30sb;
	return OP_OK;
}

int mjc400_op_73_sit()
{
	RZ_3031sb;
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
	uint16_t SP = mjc400_os_mem[97];
	IC = mjc400_os_mem[SP-4];
	R[0] = mjc400_os_mem[SP-3];
	SR = mjc400_os_mem[SP-2];
	mjc400_os_mem[97] = SP - 4;
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_74()
{
	return OP_OK;
}

int mjc400_op_74_uj()
{
	int16_t N = mjc400_get_eff_arg();
	IC = N;
	return OP_OK;
}

int mjc400_op_74_jl()
{
	int16_t N = mjc400_get_eff_arg();
	if (R0_L) IC = N;
	return OP_OK;
}

int mjc400_op_74_je()
{
	int16_t N = mjc400_get_eff_arg();
	if (R0_E) IC = N;
	return OP_OK;
}

int mjc400_op_74_jg()
{
	int16_t N = mjc400_get_eff_arg();
	if (R0_G) IC = N;
	return OP_OK;
}

int mjc400_op_74_jz()
{
	int16_t N = mjc400_get_eff_arg();
	if (R0_Z) IC = N;
	return OP_OK;
}

int mjc400_op_74_jm()
{
	int16_t N = mjc400_get_eff_arg();
	if (R0_M) IC = N;
	return OP_OK;
}

int mjc400_op_74_jn()
{
	int16_t N = mjc400_get_eff_arg();
	if (!R0_E) IC = N;
	return OP_OK;
}

int mjc400_op_74_lj()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, IC);
	IC = N+1;
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

int mjc400_op_75()
{
	return OP_OK;
}

int mjc400_op_75_ld()
{
	int16_t N = mjc400_get_eff_arg();
	R[1] = MEM(N);
	R[2] = MEM(N+1);
	return OP_OK;
}

int mjc400_op_75_lf()
{
	int16_t N = mjc400_get_eff_arg();
	R[1] = MEM(N);
	R[2] = MEM(N+1);
	R[3] = MEM(N+2);
	return OP_OK;
}

int mjc400_op_75_la()
{
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
	R[5] = MEM(N);
	R[6] = MEM(N+1);
	R[7] = MEM(N+2);
	return OP_OK;
}

int mjc400_op_75_td()
{
	int16_t N = mjc400_get_eff_arg();
	R[1] = MEMNB(N);
	R[2] = MEMNB(N+1);
	return OP_OK;
}

int mjc400_op_75_tf()
{
	int16_t N = mjc400_get_eff_arg();
	R[1] = MEMNB(N);
	R[2] = MEMNB(N+1);
	R[3] = MEMNB(N+2);
	return OP_OK;
}

int mjc400_op_75_ta()
{
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
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
	return OP_OK;
}

int mjc400_op_76_rd()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, R[1]);
	MEMw(N+1, R[2]);
	return OP_OK;
}

int mjc400_op_76_rf()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, R[1]);
	MEMw(N+1, R[2]);
	MEMw(N+2, R[3]);
	return OP_OK;
}

int mjc400_op_76_ra()
{
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, R[5]);
	MEMw(N+1, R[6]);
	MEMw(N+2, R[7]);
	return OP_OK;
}

int mjc400_op_76_pd()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[1]);
	MEMNBw(N+1, R[2]);
	return OP_OK;
}

int mjc400_op_76_pf()
{
	int16_t N = mjc400_get_eff_arg();
	MEMNBw(N, R[1]);
	MEMNBw(N+1, R[2]);
	MEMNBw(N+2, R[3]);
	return OP_OK;
}

int mjc400_op_76_pa()
{
	int16_t N = mjc400_get_eff_arg();
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
	int16_t N = mjc400_get_eff_arg();
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
	return OP_OK;
}

int mjc400_op_77_mb()
{
	int16_t N = mjc400_get_eff_arg();
	SR_MBw(MEM(N));
	return OP_OK;
}

int mjc400_op_77_im()
{
	int16_t N = mjc400_get_eff_arg();
	SR_RMw(MEM(N));
	return OP_OK;
}

int mjc400_op_77_ki()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, ((RZ & 0b11111111111100000000000000000000) >> 16) | (RZ & 0b00000000000000000000000000001111));
	return OP_OK;
}

int mjc400_op_77_fi()
{
	int16_t N = mjc400_get_eff_arg();
	int16_t RZM = MEM(N);
	RZ = RZ | (RZM & 0b0000000000001111);
	RZ = RZ | ((RZM & 0b1111111111110000) << 16);
	return OP_OK;
}

int mjc400_op_77_sp()
{
	int16_t N = mjc400_get_eff_arg();
	IC = MEMNB(N);
	R[0] = MEMNB(N+1);
	SR = MEMNB(N+2);
	return OP_OK;
}

int mjc400_op_77_md()
{
	int16_t N = mjc400_get_eff_arg();
	MOD = N;
	return OP_OK;
}

int mjc400_op_77_rz()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, 0);
	return OP_OK;
}

int mjc400_op_77_ib()
{
	int16_t N = mjc400_get_eff_arg();
	MEMw(N, MEM(N)+1);
	if (!MEM(N)) P = 1;
	return OP_OK;
}




