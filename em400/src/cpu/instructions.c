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

#include <stdlib.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "cpu/alu.h"
#include "cpu/memory.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

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
	uint16_t N = get_arg_norm();
	Rw(IR_A, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_tw()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, MEMNB(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ls()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, (R(IR_A) & ~R(7)) | (N & R(7)));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ri()
{
	uint16_t N = get_arg_norm();
	MEMw(R(IR_A), N);
	Rinc(IR_A);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rw()
{
	uint16_t N = get_arg_norm();
	MEMw(N, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_pw()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rj()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, R(R_IC));
	Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_is()
{
	uint16_t N = get_arg_norm();
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) {
		return OP_P;
	} else {
		MEMNBw(N, (MEMNB(N) | R(IR_A)));
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_bb()
{
	uint16_t N = get_arg_norm();
	if ((R(IR_A) & N) == N) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_bm()
{
	uint16_t N = get_arg_norm();
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_bs()
{
	uint16_t N = get_arg_norm();
	if ((R(IR_A) & R(7)) == (N & R(7))) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_bc()
{
	uint16_t N = get_arg_norm();
	if ((R(IR_A) & N) != N) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_bn()
{
	uint16_t N = get_arg_norm();
	if ((R(IR_A) & N) == 0) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_ou()
{
	uint16_t N = get_arg_norm();
	int io_result = io_dispatch(IO_OU, N, regs+IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_in()
{
	uint16_t N = get_arg_norm();
	int io_result = io_dispatch(IO_IN, N, regs+IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_37()
{
	return iset_37[EXT_OP_37(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_37_ad()
{
	uint16_t N = get_arg_norm();
	alu_add32(1, 2, MEM(N), MEM(N+1), 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_sd()
{
	uint16_t N = get_arg_norm();
	alu_add32(1, 2, MEM(N), MEM(N+1), -1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_mw()
{
	uint16_t N = get_arg_norm();
	alu_mul32(1, 2, MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_37_dw()
{
	uint16_t N = get_arg_norm();
	alu_div32(1, 2, MEM(N));
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
	uint16_t N = get_arg_norm();
	alu_add16(IR_A, N, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_ac()
{
	uint16_t N = get_arg_norm();
	alu_add16(IR_A, N, Fget(FL_C));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_sw()
{
	uint16_t N = - get_arg_norm();
	alu_add16(IR_A, N, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cw()
{
	uint16_t N = get_arg_norm();
	alu_compare((int16_t) R(IR_A), (int16_t) N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_or()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, R(IR_A) | N);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_om()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, MEMNB(N) | R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_nr()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, R(IR_A) & N);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_nm()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, MEMNB(N) & R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_er()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, R(IR_A) & ~N);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_em()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, MEMNB(N) & ~R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_xr()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, R(IR_A) ^ N);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_xm()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, MEMNB(N) ^ R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cl()
{
	uint16_t N = get_arg_norm();
	alu_compare((uint16_t) R(IR_A), (uint16_t) N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lb()
{
	uint16_t N = get_arg_norm();
	Rw(IR_A, (nR(IR_A) & 0b1111111100000000) | (uint16_t) MEMNBb(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rb()
{
	uint16_t N = get_arg_norm();
	MEMNBwb(N, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cb()
{
	uint16_t N = get_arg_norm();
	alu_compare((uint8_t) R(IR_A), MEMNBb(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_awt()
{
	uint16_t T = get_arg_short();
	alu_add16(IR_A, T, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_trb()
{
	int16_t T = get_arg_short();
	Radd(IR_A, T);
	if (!R(IR_A)) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// -----------------------------------------------------------------------
int op_irb()
{
	int16_t T = get_arg_short();
	Rinc(IR_A);
	if (R(IR_A)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_drb()
{
	int16_t T = get_arg_short();
	Rdec(IR_A);
	if (R(IR_A)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_cwt()
{
	int16_t T = get_arg_short();
	alu_compare((int16_t) R(IR_A), (int16_t) T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lwt()
{
	int16_t T = get_arg_short();
	Rw(IR_A, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_lws()
{
	int16_t T = get_arg_short();
	Rw(IR_A, MEM(R(R_IC) + T));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_rws()
{
	int16_t T = get_arg_short();
	MEMw(R(R_IC) + T, R(IR_A));
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_70()
{
	return iset_70[EXT_OP_70(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_70_ujs()
{
	int16_t T = get_arg_short();
	Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jls()
{
	int16_t T = get_arg_short();
	if (Fget(FL_L)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jes()
{
	int16_t T = get_arg_short();
	if (Fget(FL_E)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jgs()
{
	int16_t T = get_arg_short();
	if (Fget(FL_G)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jvs()
{
	int16_t T = get_arg_short();
	if (Fget(FL_V)) {
		Radd(R_IC, T);
		Fclr(FL_V);
	}
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jxs()
{
	int16_t T = get_arg_short();
	if (Fget(FL_X)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jys()
{
	int16_t T = get_arg_short();
	if (Fget(FL_Y)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_70_jcs()
{
	int16_t T = get_arg_short();
	if (Fget(FL_C)) Radd(R_IC, T);
	return OP_OK;
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int op_71()
{
	return iset_71[EXT_OP_71(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_71_blc()
{
	if ((((R(0) & 0b1111111100000000) >> 8) & IR_b) != IR_b) {
		return OP_P;
	} else {
		return OP_OK;
	}
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
	reg_write(0, 0, 1, 1);
	nMEMBw(0, 97, SP+4);
	SR_RM9cb;
	int_update_rp();
	SR_Qcb;
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_71_brc()
{
	if (((R(0) & 0b0000000011111111) & IR_b) != IR_b) {
		return OP_P;
	} else {
		return OP_OK;
	}
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
	return iset_72[EXT_OP_72(nR(R_IR))].op_fun();
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
	if (R(IR_A) & 0b1000000000000000) Fset(FL_X);
	else Fclr(FL_X);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_nga()
{
	alu_negate(IR_A, 1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_slz()
{
	if (R(IR_A) & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, R(IR_A)<<1);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sly()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, (R(IR_A)<<1) | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_slx()
{
	if (R(IR_A) & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)<<1) | Fget(FL_X));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_sry()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, (R(IR_A)>>1) | Fget(FL_Y)<<15);
	if (ir_a & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_ngl()
{
	alu_negate(IR_A, 0);
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

	uint16_t falling = (R(IR_A) & ((1<<IR_t)-1)) << (16-IR_t);
	
	Rw(IR_A, (R(IR_A) >> IR_t) | falling);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_rky()
{
	// TODO: does it work that way?
	Rw(IR_A, R(R_KB));
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
	if (R(IR_A) & 1) Fset(FL_X);
	else Fclr(FL_X);
	alu_set_flag_Z(R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_ngc()
{
	alu_negate(IR_A, Fget(FL_C));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svz()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1);
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);

	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svy()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1 | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_svx()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1 | Fget(FL_X));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_srx()
{
	if (R(IR_A) & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)>>1) | Fget(FL_X)<<15);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_srz()
{
	if (R(IR_A) & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)>>1) & 0b0111111111111111);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_72_lpc()
{
	reg_write(0, R(IR_A), 1, 1);
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
	return iset_73[EXT_OP_73(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_73_hlt()
{
	// handle hlt 077 as "exit emulation" if user wants to
	if (em400_cfg.exit_on_hlt) {
		uint16_t T = get_arg_short();
		if (T == 077) {
			em400_quit = 1;
			return OP_OK;
		}
	}

	pthread_mutex_lock(&int_mutex_rp);
	while (!RP) {
		pthread_cond_wait(&int_cond_rp, &int_mutex_rp);
	}
	pthread_mutex_unlock(&int_mutex_rp);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_mcl()
{
	Rw(R_SR, 0);
	int_clear_all();
	reg_write(0, 0, 1, 1);
	// TODO: zeruj kanały
	// TODO: zeruj urządzenia
	mem_remove_maps();
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_cit()
{
	int_clear(INT_SOFT_U);
	int_clear(INT_SOFT_L);
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
	int_set(INT_SOFT_U);
	int_set(INT_SOFT_L);
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
	reg_write(0, nMEMB(0, SP-3), 1, 1);
	Rw(R_SR, nMEMB(0, SP-2));
	int_update_rp();
	nMEMBw(0, 97, SP-4);
#ifdef WITH_DEBUGGER
	dbg_touch_pop(&touch_int);
#endif
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_sint()
{
	int_set(INT_EXTRA);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_73_sind()
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
	return iset_74[EXT_OP_74(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_74_uj()
{
	uint16_t N = get_arg_norm();
	Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jl()
{
	uint16_t N = get_arg_norm();
	if (Fget(FL_L)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_je()
{
	uint16_t N = get_arg_norm();
	if (Fget(FL_E)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jg()
{
	uint16_t N = get_arg_norm();
	if (Fget(FL_G)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jz()
{
	uint16_t N = get_arg_norm();
	if (Fget(FL_Z)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jm()
{
	uint16_t N = get_arg_norm();
	if (Fget(FL_M)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_jn()
{
	uint16_t N = get_arg_norm();
	if (!Fget(FL_E)) Rw(R_IC, N);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_74_lj()
{
	uint16_t N = get_arg_norm();
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
	return iset_75[EXT_OP_75(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_75_ld()
{
	uint16_t N = get_arg_norm();
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_lf()
{
	uint16_t N = get_arg_norm();
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	Rw(3, MEM(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_la()
{
	uint16_t N = get_arg_norm();
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
	uint16_t N = get_arg_norm();
	Rw(5, MEM(N));
	Rw(6, MEM(N+1));
	Rw(7, MEM(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_td()
{
	uint16_t N = get_arg_norm();
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_tf()
{
	uint16_t N = get_arg_norm();
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	Rw(3, MEMNB(N+2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_75_ta()
{
	uint16_t N = get_arg_norm();
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
	uint16_t N = get_arg_norm();
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
	return iset_76[EXT_OP_76(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_76_rd()
{
	uint16_t N = get_arg_norm();
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_rf()
{
	uint16_t N = get_arg_norm();
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	MEMw(N+2, R(3));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_ra()
{
	uint16_t N = get_arg_norm();
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
	uint16_t N = get_arg_norm();
	MEMw(N, R(5));
	MEMw(N+1, R(6));
	MEMw(N+2, R(7));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pd()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pf()
{
	uint16_t N = get_arg_norm();
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	MEMNBw(N+2, R(3));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_76_pa()
{
	uint16_t N = get_arg_norm();
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
	uint16_t N = get_arg_norm();
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
	return iset_77[EXT_OP_77(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
int op_77_mb()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = get_arg_norm();
	SR_SET_QNB(MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_im()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = get_arg_norm();
	SR_SET_MASK(MEM(N));
	int_update_rp();
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_ki()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = get_arg_norm();
	MEMw(N, int_get_nchan());
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_fi()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = get_arg_norm();
	int_put_nchan(MEM(N));
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_sp()
{
	if (SR_Q) return OP_ILLEGAL;
	uint16_t N = get_arg_norm();
	Rw(R_IC, MEMNB(N));
	reg_write(0, MEMNB(N+1), 1, 1);
	Rw(R_SR, MEMNB(N+2));
	int_update_rp();
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_md()
{
	Rinc(R_MODc);
	if (R(R_MODc) >= 4) {
		return OP_ILLEGAL;
	}
	int16_t N = get_arg_norm();
	Rw(R_MOD, N);
	return OP_MD;
}

// -----------------------------------------------------------------------
int op_77_rz()
{
	uint16_t N = get_arg_norm();
	MEMw(N, 0);
	return OP_OK;
}

// -----------------------------------------------------------------------
int op_77_ib()
{
	uint16_t N = get_arg_norm();
	MEMw(N, MEM(N)+1);
	if (!MEM(N)) {
		return OP_P;
	} else {
		return OP_OK;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
