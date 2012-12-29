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

#include <stdlib.h>

#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "io.h"
#include "iset.h"
#include "instructions.h"
#include "utils.h"

// -----------------------------------------------------------------------
int op_illegal()
{
	return OP_ILLEGAL;
}

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_lw()
{
	Rw(IR_A, (uint16_t) cpu_get_eff_arg());
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_tw()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ls()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, (R(IR_A) & ~R(7)) | (N & R(7)));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ri()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(R(IR_A), N);
	Rinc(IR_A);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rw()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_pw()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rj()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, R(R_IC));
	Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_is()
{
	uint16_t N = cpu_get_eff_arg();
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) {
		Rw(R_P, 1);
		MEMNBw(N, (MEMNB(N) | R(IR_A)));
	}
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_bb()
{
	uint16_t N = cpu_get_eff_arg();
	if ((R(IR_A) & N) == N) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_bm()
{
	uint16_t N = cpu_get_eff_arg();
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_bs()
{
	uint16_t N = cpu_get_eff_arg();
	if ((R(IR_A) & R(7)) == (N & R(7))) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_bc()
{
	uint16_t N = cpu_get_eff_arg();
	if ((R(IR_A) & N) == N) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_bn()
{
	uint16_t N = cpu_get_eff_arg();
	if ((R(IR_A) & N) == 0) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ou()
{
	uint16_t N = cpu_get_eff_arg();
	int io_result = io_dispatch(IO_OU, N, IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_in()
{
	uint16_t N = cpu_get_eff_arg();
	int io_result = io_dispatch(IO_IN, N, IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_37()
{
	return iset_37[EXT_OP_37(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_37_ad()
{
	// TODO: AWP
	uint16_t N = cpu_get_eff_arg();
	int32_t a1 = DWORD(R(1), R(2));
	int32_t a2 = DWORD(MEM(N), MEM(N+1));
	int64_t res = a1 + a2;
	Rw(1, DWORDl(res));
	Rw(2, DWORDr(res));
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Cs32(res);
	R0_Vs32(a1, a2, res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_sd()
{
	// TODO: AWP
	uint16_t N = cpu_get_eff_arg();
	int32_t a1 = DWORD(R(1), R(2));
	int32_t a2 = DWORD(MEM(N), MEM(N+1));
	int64_t res = a1 - a2;
	Rw(1, DWORDl(res));
	Rw(2, DWORDr(res));
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Cs32(res);
	R0_Vs32(a1, -a2, res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_mw()
{
	// TODO: AWP
	uint16_t N = cpu_get_eff_arg();
	int16_t m1 = R(2);
	int16_t m2 = MEM(N);
	int64_t res = m1 * m2;
	Rw(1, DWORDl(res));
	Rw(2, DWORDr(res));
	R0_Ms32(res);
	R0_Zs32(res);
	R0_Vs32(m1, m2, res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_dw()
{
	// TODO: AWP
	uint16_t N = cpu_get_eff_arg();
	int32_t d1 = DWORD(R(1), R(2));
	int16_t d2 = MEM(N);
	int32_t res = d1 / d2;
	Rw(2, res);
	Rw(1, d1 % d2);
	R0_Ms32(res);
	R0_Zs32(res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_af()
{
	// TODO: floats
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_sf()
{
	// TODO: floats
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_mf()
{
	// TODO: floats
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_df()
{
	// TODO: floats
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_aw()
{
	int16_t N = cpu_get_eff_arg();
	int16_t tmp = R(IR_A);
	int32_t res = N + tmp;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, tmp, res);
	Rw(IR_A, (uint16_t) res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ac()
{
	int16_t N = cpu_get_eff_arg();
	int16_t tmp = R(IR_A);
	int32_t res = N + tmp + R0_C;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, tmp, res);
	Rw(IR_A, (uint16_t) res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_sw()
{
	int16_t N = cpu_get_eff_arg();
	int16_t tmp = R(IR_A);
	int32_t res = N - tmp;
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(N, -tmp, res);
	Rw(IR_A, (uint16_t) res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cw()
{
	uint16_t N = cpu_get_eff_arg();
	R0_LEG((int16_t)R(IR_A), (int16_t)N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_or()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, R(IR_A) | N);
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_om()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, MEMNB(N) | R(IR_A));
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_nr()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, R(IR_A) & N);
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_nm()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, MEMNB(N) & R(IR_A));
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_er()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, R(IR_A) & ~N);
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_em()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, MEMNB(N) & ~R(IR_A));
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_xr()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(IR_A, R(IR_A) ^ N);
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_xm()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, MEMNB(N) ^ R(IR_A));
	R0_Zs16(MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cl()
{
	uint16_t N = cpu_get_eff_arg();
	R0_LEG((uint16_t)R(IR_A), (uint16_t)N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lb()
{
	uint16_t N = cpu_get_eff_arg();

	// N lsb=1 - right byte
	// N lsb=0 - left byte

	// we shift data 8 bits right if N0 = 0
	int shift = 8 * ~(N&1); // left=8, right=0

	// get the real address
	N >>= 1;

	Rw(IR_A, (nR(IR_A) & 0b1111111100000000) | ((MEMNB(N) >> shift) & 0b0000000011111111));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rb()
{
	uint16_t N = cpu_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;

	int16_t oval = nMEMNB(N) & (0b1111111100000000 >> shift);
	int16_t val = (R(IR_A) & 0b0000000011111111) << shift;
	MEMNBw(N, oval | val);

	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cb()
{
	uint16_t N = cpu_get_eff_arg();
	int shift = 8 * ~(N&1);
	N >>= 1;
	R0_LEG((uint8_t)(R(IR_A)&0b0000000011111111), (uint8_t)((MEMNB(N) >> shift) & 0b0000000011111111));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_awt()
{
	int16_t tmp = R(IR_A);
	int32_t res = IR_T + tmp;
	Rw(IR_A, res);
	R0_Ms16((uint32_t) res);
	R0_Zs16((uint32_t) res);
	R0_Cs16((uint32_t) res);
	R0_Vs16(IR_T, tmp, res);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_trb()
{
	Radd(IR_A, IR_T);
	if (!R(IR_A)) Rw(R_P, 1);
	else Rw(R_P, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_irb()
{
	Rinc(IR_A);
	if (R(IR_A)) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_drb()
{
	Rdec(IR_A);
	if (R(IR_A)) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cwt()
{
	R0_LEG((int16_t)R(IR_A), (int8_t)IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lwt()
{
	Rw(IR_A, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lws()
{
	Rw(IR_A, MEM(R(R_IC) + IR_T));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rws()
{
	MEMw(R(R_IC) + IR_T, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_70()
{
	return iset_70[EXT_OP_70(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_70_ujs()
{
	Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jls()
{
	if (R0_E) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jes()
{
	if (R0_L) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jgs()
{
	if (R0_G) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jvs()
{
	if (R0_V) {
		Radd(R_IC, IR_T);
		R0_Vcb;
	}
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jxs()
{
	if (R0_X) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jys()
{
	if (R0_Y) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jcs()
{
	if (R0_C) Radd(R_IC, IR_T);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_71()
{
	return iset_71[EXT_OP_71(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_71_blc()
{
	if ((((R(0) & 0b1111111100000000) >> 8) & IR_b) != IR_b) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_71_exl()
{
	uint16_t SP = nMEMB(0, 97);
	nMEMBw(0, SP, R(R_IC));
	nMEMBw(0, SP+1, R(0));
	nMEMBw(0, SP+2, R(R_SR));
	nMEMBw(0, SP+3, IR_b);
	Rw(R_IC, nMEMB(0, 96));
	Rw(0, 0);
	nMEMBw(0, 97, SP+4);
	SR_RM9cb;
	SR_Qcb;
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_71_brc()
{
	if (((R(0) & 0b0000000011111111) & IR_b) != IR_b) Rw(R_P, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_71_nrf()
{
	// TODO: floats
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_72()
{
	return iset_72[EXT_OP_72(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_72_ric()
{
	Rw(IR_A, R(R_IC));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_zlb()
{
	Rw(IR_A, nR(IR_A) & 0b0000000011111111);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sxu()
{
	if (R(IR_A) & 0b1000000000000000) R0_Zsb;
	else R0_Zcb;
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_nga()
{
	Rw(IR_A, ~R(IR_A) + 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_slz()
{
	if (R(IR_A) & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)<<1) & 0b1111111111111110);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sly()
{
	if (R(IR_A) & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)<<1) & (0b1111111111111110 | R0_Y));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_slx()
{
	if (R(IR_A) & 0b1000000000000000) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)<<1) & (0b1111111111111110 | R0_X));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sry()
{
	if (R(IR_A) & 1) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)>>1) & (0b0111111111111111 | (R0_Y<<15)));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_ngl()
{
	Rw(IR_A, ~R(IR_A));
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_rpc()
{
	Rw(IR_A, R(0));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_shc()
{
	if (!IR_t) return OP_OK;

	uint16_t falling = (R(IR_A) & (IR_t-1)) << (16-IR_t);
	
	Rw(IR_A, (R(IR_A) >> IR_t) | falling);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_rky()
{
	// TODO: does it work that way?
	if (R(R_KB)) {
		Rw(IR_A, R(R_KB));
	}
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_zrb()
{
	Rw(IR_A, nR(IR_A) & 0b1111111100000000);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sxl()
{
	if (R(IR_A) & 1) R0_Xsb;
	else R0_Xcb;
	R0_Zs16(R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_ngc()
{
	Rw(IR_A, ~R(IR_A) + R0_C);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svz()
{
	if (R(IR_A) & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	Rw(IR_A, (R(IR_A)<<1) & 0b1111111111111110);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svy()
{
	if (R(IR_A) & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	Rw(IR_A, (R(IR_A)<<1) & (0b1111111111111110 | R0_Y));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svx()
{
	if (R(IR_A) & 0b1000000000000000) {
		R0_Ysb;
		R0_Vsb;
	} else {
		R0_Ycb;
		R0_Vcb;
	}
	Rw(IR_A, (R(IR_A)<<1) & (0b1111111111111110 | R0_X));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_srx()
{
	if (R(IR_A) & 1) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)>>1) & (0b0111111111111111 | (R0_X<<15)));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_srz()
{
	if (R(IR_A) & 1) R0_Ysb;
	else R0_Ycb;
	Rw(IR_A, (R(IR_A)>>1) & 0b0111111111111111);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_lpc()
{
	Rw(0, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_73()
{
	// all 73-instructions are illegal in user mode
	if (SR_Q) return OP_ILLEGAL;
	return iset_73[EXT_OP_73(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_73_hlt()
{
	pthread_mutex_lock(&int_mutex);
	while (!RP) {
		pthread_cond_wait(&int_cond, &int_mutex);
	}
	pthread_mutex_unlock(&int_mutex);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_mcl()
{
	int_clear(INT_ALL);
	Rw(R_SR, 0);
	Rw(0, 0);
	// TODO: zeruj kanały
	// TODO: zeruj urządzenia
	mem_remove_maps();
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_cit()
{
	int_clear(INT_SOFT_U | INT_SOFT_L);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_sil()
{
	int_set(INT_SOFT_L);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_siu()
{
	int_set(INT_SOFT_U);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_sit()
{
	int_set(INT_SOFT_U | INT_SOFT_L);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_giu()
{
	// TODO: 2-cpu configuration
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_gil()
{
	// TODO: 2-cpu configuration
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_lip()
{
	uint16_t SP = nMEMB(0, 97);
	Rw(R_IC, nMEMB(0, SP-4));
	Rw(0, nMEMB(0, SP-3));
	Rw(R_SR, nMEMB(0, SP-2));
	nMEMBw(0, 97, SP-4);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_six()
{
	int_set(INT_EXTRA);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_cix()
{
	int_clear(INT_EXTRA);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_74()
{
	return iset_74[EXT_OP_74(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_74_uj()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jl()
{
	uint16_t N = cpu_get_eff_arg();
	if (R0_L) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_je()
{
	uint16_t N = cpu_get_eff_arg();
	if (R0_E) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jg()
{
	uint16_t N = cpu_get_eff_arg();
	if (R0_G) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jz()
{
	uint16_t N = cpu_get_eff_arg();
	if (R0_Z) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jm()
{
	uint16_t N = cpu_get_eff_arg();
	if (R0_M) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jn()
{
	uint16_t N = cpu_get_eff_arg();
	if (!R0_E) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_lj()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(R_IC));
	Rw(R_IC, N+1);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_75()
{
	return iset_75[EXT_OP_75(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_75_ld()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_lf()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	Rw(3, MEM(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_la()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	Rw(3, MEM(N+2));
	Rw(4, MEM(N+3));
	Rw(5, MEM(N+4));
	Rw(6, MEM(N+5));
	Rw(7, MEM(N+6));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_ll()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(5, MEM(N));
	Rw(6, MEM(N+1));
	Rw(7, MEM(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_td()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_tf()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	Rw(3, MEMNB(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_ta()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	Rw(3, MEMNB(N+2));
	Rw(4, MEMNB(N+3));
	Rw(5, MEMNB(N+4));
	Rw(6, MEMNB(N+5));
	Rw(7, MEMNB(N+6));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_tl()
{
	uint16_t N = cpu_get_eff_arg();
	Rw(5, MEMNB(N));
	Rw(6, MEMNB(N+1));
	Rw(7, MEMNB(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_76()
{
	return iset_76[EXT_OP_76(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_76_rd()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_rf()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	MEMw(N+2, R(3));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_ra()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	MEMw(N+2, R(3));
	MEMw(N+3, R(4));
	MEMw(N+4, R(5));
	MEMw(N+5, R(6));
	MEMw(N+6, R(7));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_rl()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, R(5));
	MEMw(N+1, R(6));
	MEMw(N+2, R(7));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pd()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pf()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	MEMNBw(N+2, R(3));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pa()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	MEMNBw(N+2, R(3));
	MEMNBw(N+3, R(4));
	MEMNBw(N+4, R(5));
	MEMNBw(N+5, R(6));
	MEMNBw(N+6, R(7));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pl()
{
	uint16_t N = cpu_get_eff_arg();
	MEMNBw(N, R(5));
	MEMNBw(N+1, R(6));
	MEMNBw(N+2, R(7));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_77()
{
	return iset_77[EXT_OP_77(R(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_77_mb()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = cpu_get_eff_arg();
	SR_SET_QNB(MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_im()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = cpu_get_eff_arg();
	SR_SET_MASK(MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_ki()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, int_get_nchan());
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_fi()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = cpu_get_eff_arg();
	int_put_nchan(MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_sp()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = cpu_get_eff_arg();
	Rw(R_IC, MEMNB(N));
	Rw(0, MEMNB(N+1));
	Rw(R_SR, MEMNB(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_md()
{
	Rinc(R_MODc);
	if (R(R_MODc) >= 4) {
		return OP_ILLEGAL;
	}
	int16_t N = cpu_get_eff_arg();
	Rw(R_MOD, N);
	return OP_MD;
}

// -----------------------------------------------------------------------
int op_77_rz()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_ib()
{
	uint16_t N = cpu_get_eff_arg();
	MEMw(N, MEM(N)+1);
	if (!MEM(N)) Rw(R_P, 1);
	return OP_OK;
}

// vim: tabstop=4
