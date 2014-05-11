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

#include "em400.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/reg/ir.h"
#include "cpu/reg/sr.h"
#include "cpu/reg/flags.h"
#include "cpu/regwr.h"
#include "cpu/interrupts.h"
#include "cpu/alu.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "io/io.h"

#include "cfg.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/decode.h"
int exl_was_exl;
int exl_was_nb;
int exl_was_addr;
int exl_was_r4;
#endif
#include "debugger/log.h"

// convenience memory access macros (with "nomem" handling)
#define mem_ret_get(nb, a, dptr)			    if (!mem_cpu_get(nb, a, dptr)) return;
#define mem_ret_put(nb, a, data)			    if (!mem_cpu_put(nb, a, data)) return;
#define mem_ret_mget(nb, saddr, dest, count)    if (!mem_cpu_mget(nb, saddr, dest, count)) return;
#define mem_ret_mput(nb, saddr, src, count) 	if (!mem_cpu_mput(nb, saddr, src, count)) return;

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_lw()
{
	reg_safe_write(IR_A, N);
}

// -----------------------------------------------------------------------
void op_tw()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	reg_safe_write(IR_A, data);
}

// -----------------------------------------------------------------------
void op_ls()
{
	reg_safe_write(IR_A, (regs[IR_A] & ~regs[7]) | (N & regs[7]));
}

// -----------------------------------------------------------------------
void op_ri()
{
	mem_ret_put(QNB, regs[IR_A], N);
	reg_safe_write(IR_A, regs[IR_A]+1);
}

// -----------------------------------------------------------------------
void op_rw()
{
	mem_ret_put(QNB, N, regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_pw()
{
	mem_ret_put(NB, N, regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_rj()
{
	reg_safe_write(IR_A, regs[R_IC]);
	regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_is()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	if ((data & regs[IR_A]) == regs[IR_A]) {
		P = 1;
	} else {
		mem_ret_put(NB, N, data | regs[IR_A]);
	}
}

// -----------------------------------------------------------------------
void op_bb()
{
	if ((regs[IR_A] & (uint16_t) N) == (uint16_t) N) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_bm()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	if ((data & regs[IR_A]) == regs[IR_A]) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_bs()
{
	if ((regs[IR_A] & regs[7]) == ((uint16_t) N & regs[7])) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_bc()
{
	if ((regs[IR_A] & (uint16_t) N) != (uint16_t) N) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_bn()
{
	if ((regs[IR_A] & (uint16_t) N) == 0) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_ou()
{
	uint16_t data;
	int io_result = io_dispatch(IO_OU, N, regs+IR_A);
	mem_ret_get(QNB, regs[R_IC] + io_result, &data);
	regs[R_IC] = data;
}

// -----------------------------------------------------------------------
void op_in()
{
	uint16_t data;
	int io_result = io_dispatch(IO_IN, N, regs+IR_A);
	mem_ret_get(QNB, regs[R_IC] + io_result, &data);
	regs[R_IC] = data;
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_37()
{
	return iset_37 + EXT_OP_37(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_37_ad()
{
	alu_awp_dispatch(AWP_AD, N);
}

// -----------------------------------------------------------------------
void op_37_sd()
{
	alu_awp_dispatch(AWP_SD, N);
}

// -----------------------------------------------------------------------
void op_37_mw()
{
	alu_awp_dispatch(AWP_MW, N);
}

// -----------------------------------------------------------------------
void op_37_dw()
{
	alu_awp_dispatch(AWP_DW, N);
}

// -----------------------------------------------------------------------
void op_37_af()
{
	alu_awp_dispatch(AWP_AF, N);
}

// -----------------------------------------------------------------------
void op_37_sf()
{
	alu_awp_dispatch(AWP_SF, N);
}

// -----------------------------------------------------------------------
void op_37_mf()
{
	alu_awp_dispatch(AWP_MF, N);
}

// -----------------------------------------------------------------------
void op_37_df()
{
	alu_awp_dispatch(AWP_DF, N);
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_aw()
{
	alu_16_add(IR_A, N, 0, 1);
}

// -----------------------------------------------------------------------
void op_ac()
{
	alu_16_add(IR_A, N, Fget(FL_C), 1);
}

// -----------------------------------------------------------------------
void op_sw()
{
	alu_16_add(IR_A, N, 0, -1);
}

// -----------------------------------------------------------------------
void op_cw()
{
	alu_16_set_LEG((int16_t) regs[IR_A], (int16_t) N);
}

// -----------------------------------------------------------------------
void op_or()
{
	reg_safe_write(IR_A, regs[IR_A] | N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_om()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	data |= regs[IR_A];
	mem_ret_put(NB, N, data);
	alu_16_set_Z(data);
}

// -----------------------------------------------------------------------
void op_nr()
{
	reg_safe_write(IR_A, regs[IR_A] & N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_nm()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	data &= regs[IR_A];
	mem_ret_put(NB, N, data);
	alu_16_set_Z(data);
}

// -----------------------------------------------------------------------
void op_er()
{
	reg_safe_write(IR_A, regs[IR_A] & ~N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_em()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	data &= ~regs[IR_A];
	mem_ret_put(NB, N, data);
	alu_16_set_Z(data);
}

// -----------------------------------------------------------------------
void op_xr()
{
	reg_safe_write(IR_A, regs[IR_A] ^ N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_xm()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	data ^= regs[IR_A];
	mem_ret_put(NB, N, data);
	alu_16_set_Z(data);
}

// -----------------------------------------------------------------------
void op_cl()
{
	alu_16_set_LEG(regs[IR_A], (uint16_t) N);
}

// -----------------------------------------------------------------------
void op_lb()
{
	uint8_t data;
	if (cpu_mod) {
		if (!mem_get_byte(NB, N, &data)) return;
	} else {
		if (!mem_get_byte(NB, (uint16_t) N, &data)) return;
	}
	reg_safe_write(IR_A, (regs[IR_A] & 0b1111111100000000) | data);
}

// -----------------------------------------------------------------------
void op_rb()
{
	if (cpu_mod) {
		if (!mem_put_byte(NB, N, regs[IR_A])) return;
	} else {
		if (!mem_put_byte(NB, (uint16_t) N, regs[IR_A])) return;
	}
}

// -----------------------------------------------------------------------
void op_cb()
{
	uint8_t data;
	if (!mem_get_byte(NB, N, &data)) return;
	alu_16_set_LEG((uint8_t) regs[IR_A], data);
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_awt()
{
	alu_16_add(IR_A, N, 0, 1);
}

// -----------------------------------------------------------------------
void op_trb()
{
	reg_safe_write(IR_A, regs[IR_A] + N);
	if (regs[IR_A] == 0) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_irb()
{
	reg_safe_write(IR_A, regs[IR_A]+1);
	if (regs[IR_A]) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_drb()
{
	reg_safe_write(IR_A, regs[IR_A]-1);
	if (regs[IR_A] != 0) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_cwt()
{
	alu_16_set_LEG((int16_t) regs[IR_A], (int16_t) N);
}

// -----------------------------------------------------------------------
void op_lwt()
{
	reg_safe_write(IR_A, N);
}

// -----------------------------------------------------------------------
void op_lws()
{
	uint16_t data;
	mem_ret_get(QNB, regs[R_IC] + N, &data);
	reg_safe_write(IR_A, data);
}

// -----------------------------------------------------------------------
void op_rws()
{
	mem_ret_put(QNB, regs[R_IC] + N, regs[IR_A]);
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_70()
{
	return iset_70 + EXT_OP_70(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_70_ujs()
{
	regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jls()
{
	if (Fget(FL_L)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jes()
{
	if (Fget(FL_E)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jgs()
{
	if (Fget(FL_G)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
	if (Fget(FL_V)) {
		regs[R_IC] += N;
		Fclr(FL_V);
	}
}

// -----------------------------------------------------------------------
void op_70_jxs()
{
	if (Fget(FL_X)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jys()
{
	if (Fget(FL_Y)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
void op_70_jcs()
{
	if (Fget(FL_C)) regs[R_IC] += N;
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_71()
{
	return iset_71 + EXT_OP_71(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_71_blc()
{
	if (((regs[0] >> 8) & IR_b) != IR_b) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_71_exl()
{
	uint16_t data;
	LOG(L_OP, 10, "EXL: %i (r4: 0x%04x)", IR_b, regs[4]);
#ifdef WITH_DEBUGGER
	char *details = decode_exl(NB, regs[4], IR_b); /* CURRENT_BLOCK_ADDR */
	log_splitlog(L_CRK5, 10, details);
	free(details);
	exl_was_exl = IR_b;
	exl_was_nb = NB; /* CURRENT_BLOCK_ADDR */
	exl_was_addr = regs[R_IC];
	exl_was_r4 = regs[4];
#endif
	mem_ret_get(0, 96, &data);
	if (!cpu_ctx_switch(IR_b, data, 0b1111111110011111)) return;
}

// -----------------------------------------------------------------------
void op_71_brc()
{
	uint16_t b = IR_b + regs[R_MOD];
	if ((regs[0] & b) != b) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_71_nrf()
{
	int nrf_op = IR_A & 0b011;
	alu_awp_dispatch(AWP_NRF0+nrf_op, 0);
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_72()
{
	return iset_72 + EXT_OP_72(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_72_ric()
{
	reg_safe_write(IR_A, regs[R_IC]);
}

// -----------------------------------------------------------------------
void op_72_zlb()
{
	reg_safe_write(IR_A, regs[IR_A] & 0b0000000011111111);
}

// -----------------------------------------------------------------------
void op_72_sxu()
{
	if (regs[IR_A] & 0b1000000000000000) {
		Fset(FL_X);
	} else {
		Fclr(FL_X);
	}
}

// -----------------------------------------------------------------------
void op_72_nga()
{
	alu_16_neg(IR_A, 1);
}

// -----------------------------------------------------------------------
void op_72_slz()
{
	if (regs[IR_A] & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_safe_write(IR_A, regs[IR_A]<<1);
}

// -----------------------------------------------------------------------
void op_72_sly()
{
	uint16_t ir_a = regs[IR_A];
	reg_safe_write(IR_A, (regs[IR_A]<<1) | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
}

// -----------------------------------------------------------------------
void op_72_slx()
{
	if (regs[IR_A] & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_safe_write(IR_A, (regs[IR_A]<<1) | Fget(FL_X));
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	uint16_t ir_a = regs[IR_A];
	reg_safe_write(IR_A, (regs[IR_A]>>1) | Fget(FL_Y)<<15);
	if (ir_a & 1) Fset(FL_Y);
	else Fclr(FL_Y);
}

// -----------------------------------------------------------------------
void op_72_ngl()
{
	regs[IR_A] = ~regs[IR_A];
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_72_rpc()
{
	reg_safe_write(IR_A, regs[0]);
}

// -----------------------------------------------------------------------
void op_72_shc()
{
	if (!IR_t) return;

	uint16_t falling = (regs[IR_A] & ((1<<IR_t)-1)) << (16-IR_t);
	
	reg_safe_write(IR_A, (regs[IR_A] >> IR_t) | falling);
}

// -----------------------------------------------------------------------
void op_72_rky()
{
	reg_safe_write(IR_A, regs[R_KB]);
}

// -----------------------------------------------------------------------
void op_72_zrb()
{
	reg_safe_write(IR_A, regs[IR_A] & 0b1111111100000000);
}

// -----------------------------------------------------------------------
void op_72_sxl()
{
	if (regs[IR_A] & 1) {
		Fset(FL_X);
	} else {
		Fclr(FL_X);
	}
}

// -----------------------------------------------------------------------
void op_72_ngc()
{
	alu_16_neg(IR_A, Fget(FL_C));
}

// -----------------------------------------------------------------------
void op_72_svz()
{
	if (regs[IR_A] & 0b1000000000000000) {
		Fset(FL_Y);
	} else {
		Fclr(FL_Y);
	}
	alu_16_update_V(regs[IR_A], regs[IR_A], regs[IR_A]<<1);
	reg_safe_write(IR_A, regs[IR_A]<<1);
}

// -----------------------------------------------------------------------
void op_72_svy()
{
	uint16_t ir_a = regs[IR_A];
	reg_safe_write(IR_A, regs[IR_A]<<1 | Fget(FL_Y));
	alu_16_update_V(ir_a, ir_a, regs[IR_A]);
	if (ir_a & 0b1000000000000000) {
		Fset(FL_Y);
	} else {
		Fclr(FL_Y);
	}
}

// -----------------------------------------------------------------------
void op_72_svx()
{
	uint16_t ir_a = regs[IR_A];
	reg_safe_write(IR_A, regs[IR_A]<<1 | Fget(FL_X));
	alu_16_update_V(ir_a, ir_a, regs[IR_A]);
	if (ir_a & 0b1000000000000000) {
		Fset(FL_Y);
	} else {
		Fclr(FL_Y);
	}
}

// -----------------------------------------------------------------------
void op_72_srx()
{
	if (regs[IR_A] & 1) {
		Fset(FL_Y);
	} else {
		Fclr(FL_Y);
	}
	reg_safe_write(IR_A, (regs[IR_A]>>1) | Fget(FL_X)<<15);
}

// -----------------------------------------------------------------------
void op_72_srz()
{
	if (regs[IR_A] & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_safe_write(IR_A, (regs[IR_A]>>1) & 0b0111111111111111);
}

// -----------------------------------------------------------------------
void op_72_lpc()
{
	regs[0] = regs[IR_A];
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_73()
{
	return iset_73 + EXT_OP_73(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_73_hlt()
{
	LOG(L_OP, 1, "HALT 0%02o (alarm: %i)", N, regs[6]&255);

	// handle hlt>=040 as "exit emulation" if user wants to
	if ((em400_cfg.exit_on_hlt) && (N >= 040)) {
		em400_state = STATE_QUIT;
		return;
	// otherwise, wait for interrupt
	} else {
		int_wait();
	}
}

// -----------------------------------------------------------------------
void op_73_mcl()
{
	regs[R_SR] = 0;
	int_update_mask();
	int_clear_all();
	regs[0] = 0;
	mem_reset();
	io_reset();
	cpu_mod_off();
}

// -----------------------------------------------------------------------
void op_73_cit()
{
	int_clear(INT_SOFT_U);
	int_clear(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_sil()
{
	int_set(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_siu()
{
	int_set(INT_SOFT_U);
}

// -----------------------------------------------------------------------
void op_73_sit()
{
	int_set(INT_SOFT_U);
	int_set(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_giu()
{
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_gil()
{
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_lip()
{
	cpu_ctx_restore();
#ifdef WITH_DEBUGGER
	log_int_level += 4;
#endif
}

// -----------------------------------------------------------------------
void op_73_sint()
{
	int_set(int_extra);
}

// -----------------------------------------------------------------------
void op_73_sind()
{
	int_set(int_extra);
}

// -----------------------------------------------------------------------
void op_73_cron()
{
	cpu_mod_on();
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_74()
{
	return iset_74 + EXT_OP_74(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_74_uj()
{
	regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_jl()
{
	if (Fget(FL_L)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_je()
{
	if (Fget(FL_E)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_jg()
{
	if (Fget(FL_G)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_jz()
{
	if (Fget(FL_Z)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_jm()
{
	if (Fget(FL_M)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_jn()
{
	if (!Fget(FL_E)) regs[R_IC] = N;
}

// -----------------------------------------------------------------------
void op_74_lj()
{
	mem_ret_put(QNB, N, regs[R_IC]);
	regs[R_IC] = N+1;
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_75()
{
	return iset_75 + EXT_OP_75(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_75_ld()
{
	mem_ret_mget(QNB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_75_lf()
{
	mem_ret_mget(QNB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_75_la()
{
	mem_ret_mget(QNB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_75_ll()
{
	mem_ret_mget(QNB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
void op_75_td()
{
	mem_ret_mget(NB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_75_tf()
{
	mem_ret_mget(NB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_75_ta()
{
	mem_ret_mget(NB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_75_tl()
{
	mem_ret_mget(NB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_76()
{
	return iset_76 + EXT_OP_76(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_76_rd()
{
	mem_ret_mput(QNB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_76_rf()
{
	mem_ret_mput(QNB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_76_ra()
{
	mem_ret_mput(QNB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_76_rl()
{
	mem_ret_mput(QNB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
void op_76_pd()
{
	mem_ret_mput(NB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_76_pf()
{
	mem_ret_mput(NB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_76_pa()
{
	mem_ret_mput(NB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_76_pl()
{
	mem_ret_mput(NB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct opdef * op_77()
{
	return iset_77 + EXT_OP_77(regs[R_IR]);
}

// -----------------------------------------------------------------------
void op_77_mb()
{
	uint16_t data;
	mem_ret_get(QNB, N, &data);
	regs[R_SR] = (regs[R_SR] & 0b111111111000000) | (data & 0b0000000000111111);
}

// -----------------------------------------------------------------------
void op_77_im()
{
	uint16_t data;
	mem_ret_get(QNB, N, &data);
	regs[R_SR] = (regs[R_SR] & 0b000000000111111) | (data & 0b1111111111000000);
	int_update_mask();
}

// -----------------------------------------------------------------------
void op_77_ki()
{
	uint16_t data = int_get_nchan();
	mem_ret_put(QNB, N, data);
}

// -----------------------------------------------------------------------
void op_77_fi()
{
	uint16_t data;
	mem_ret_get(QNB, N, &data);
	int_put_nchan(data);
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	uint16_t data;
	mem_ret_get(NB, N, &data);
	regs[R_IC] = data;

#ifdef WITH_DEBUGGER
	LOG(L_CRK5, 50, "SP: context @ 0x%04x -> IC: 0x%04x", N, data);
	char *ctx = decode_ctx(0, N, 0);
	log_splitlog(L_CRK5, 50, ctx);
	free(ctx);
#endif

	mem_ret_get(NB, N+1, &data);
	regs[0] = data;

	mem_ret_get(NB, N+2, &data);
	regs[R_SR] = data;
	int_update_mask();

#ifdef WITH_DEBUGGER
	log_int_level = LOG_INT_INDENT_MAX;
	if (exl_was_exl && (regs[R_IC] == exl_was_addr) && (NB == exl_was_nb)) {
		char *details = decode_exl(exl_was_nb, exl_was_r4, -exl_was_exl);
		log_splitlog(L_CRK5, 10, details);
		free(details);
		exl_was_exl = 0;
	}
#endif
}

// -----------------------------------------------------------------------
void op_77_md()
{
	regs[R_MOD] = N;
	regs[R_MODc]++;
}

// -----------------------------------------------------------------------
void op_77_rz()
{
	mem_ret_put(QNB, N, 0);
}

// -----------------------------------------------------------------------
void op_77_ib()
{
	uint16_t data;
	mem_ret_get(QNB, N, &data);
	mem_ret_put(QNB, N, ++data);
	if (data == 0) {
		P = 1;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
