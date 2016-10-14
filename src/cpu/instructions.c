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
#include "cpu/interrupts.h"
#include "cpu/alu.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "io/defs.h"
#include "io/io.h"

#include "cfg.h"
#include "utils.h"
#include "log.h"
#include "log_crk.h"

#define USER_ILLEGAL \
	if (Q) { \
		LOGCPU(L_CPU, 2, "    (ineffective: user-illegal instruction)"); \
		int_set(INT_ILLEGAL_INSTRUCTION); \
		return; \
	}

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_lw()
{
	reg_restrict_write(IR_A, N);
}

// -----------------------------------------------------------------------
void op_tw()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		reg_restrict_write(IR_A, data);
	}
}

// -----------------------------------------------------------------------
void op_ls()
{
	reg_restrict_write(IR_A, (regs[IR_A] & ~regs[7]) | (N & regs[7]));
}

// -----------------------------------------------------------------------
void op_ri()
{
	if (cpu_mem_put(QNB, regs[IR_A], N)) {
		reg_restrict_write(IR_A, regs[IR_A]+1);
	}
}

// -----------------------------------------------------------------------
void op_rw()
{
	cpu_mem_put(QNB, N, regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_pw()
{
	cpu_mem_put(NB, N, regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_rj()
{
	reg_restrict_write(IR_A, rIC);
	rIC = N;
}

// -----------------------------------------------------------------------
void op_is()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		if ((data & regs[IR_A]) == regs[IR_A]) {
			P = 1;
		} else {
			cpu_mem_put(NB, N, data | regs[IR_A]);
		}
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
	if (cpu_mem_get(NB, N, &data)) {
		if ((data & regs[IR_A]) == regs[IR_A]) {
			P = 1;
		}
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
	if (cpu_user_io_illegal) {
		USER_ILLEGAL;
	}

	uint16_t data;
	int io_result = io_dispatch(IO_OU, N, regs+IR_A);
	if (cpu_mem_get(QNB, rIC + io_result, &data)) {
		rIC = data;
	}
}

// -----------------------------------------------------------------------
void op_in()
{
	if (cpu_user_io_illegal) {
		USER_ILLEGAL;
	}

	uint16_t data;
	int io_result = io_dispatch(IO_IN, N, regs+IR_A);
	if (cpu_mem_get(QNB, rIC + io_result, &data)) {
		rIC = data;
	}
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_37_ad()
{
	awp_dispatch(AWP_AD, N);
}

// -----------------------------------------------------------------------
void op_37_sd()
{
	awp_dispatch(AWP_SD, N);
}

// -----------------------------------------------------------------------
void op_37_mw()
{
	awp_dispatch(AWP_MW, N);
}

// -----------------------------------------------------------------------
void op_37_dw()
{
	awp_dispatch(AWP_DW, N);
}

// -----------------------------------------------------------------------
void op_37_af()
{
	awp_dispatch(AWP_AF, N);
}

// -----------------------------------------------------------------------
void op_37_sf()
{
	awp_dispatch(AWP_SF, N);
}

// -----------------------------------------------------------------------
void op_37_mf()
{
	awp_dispatch(AWP_MF, N);
}

// -----------------------------------------------------------------------
void op_37_df()
{
	awp_dispatch(AWP_DF, N);
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
	reg_restrict_write(IR_A, regs[IR_A] | N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_om()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data |= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_nr()
{
	reg_restrict_write(IR_A, regs[IR_A] & N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_nm()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data &= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_er()
{
	reg_restrict_write(IR_A, regs[IR_A] & ~N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_em()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data &= ~regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_xr()
{
	reg_restrict_write(IR_A, regs[IR_A] ^ N);
	alu_16_set_Z(regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_xm()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data ^= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z(data);
		}
	}
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
	if (cpu_mem_get_byte(NB, N, &data)) {
		reg_restrict_write(IR_A, (regs[IR_A] & 0b1111111100000000) | data);
	}
}

// -----------------------------------------------------------------------
void op_rb()
{
	cpu_mem_put_byte(NB, N, regs[IR_A]);
}

// -----------------------------------------------------------------------
void op_cb()
{
	uint8_t data;
	if (cpu_mem_get_byte(NB, N, &data)) {
		alu_16_set_LEG((uint8_t) regs[IR_A], data);
	}
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
	reg_restrict_write(IR_A, regs[IR_A] + N);
	if (regs[IR_A] == 0) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_irb()
{
	reg_restrict_write(IR_A, regs[IR_A]+1);
	if (regs[IR_A]) rIC += N;
}

// -----------------------------------------------------------------------
void op_drb()
{
	reg_restrict_write(IR_A, regs[IR_A]-1);
	if (regs[IR_A] != 0) rIC += N;
}

// -----------------------------------------------------------------------
void op_cwt()
{
	alu_16_set_LEG((int16_t) regs[IR_A], (int16_t) N);
}

// -----------------------------------------------------------------------
void op_lwt()
{
	reg_restrict_write(IR_A, N);
}

// -----------------------------------------------------------------------
void op_lws()
{
	uint16_t data;
	if (cpu_mem_get(QNB, rIC + N, &data)) {
		reg_restrict_write(IR_A, data);
	}
}

// -----------------------------------------------------------------------
void op_rws()
{
	cpu_mem_put(QNB, rIC + N, regs[IR_A]);
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_70_ujs()
{
	rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jls()
{
	if (Fget(FL_L)) rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jes()
{
	if (Fget(FL_E)) rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jgs()
{
	if (Fget(FL_G)) rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
	if (Fget(FL_V)) {
		rIC += N;
		Fclr(FL_V);
	}
}

// -----------------------------------------------------------------------
void op_70_jxs()
{
	if (Fget(FL_X)) rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jys()
{
	if (Fget(FL_Y)) rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jcs()
{
	if (Fget(FL_C)) rIC += N;
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

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

	if (LOG_ENABLED) {
		if (log_wants(L_OP, 2)) {
			log_log_cpu(L_OP, 2, "EXL: %i (r4: 0x%04x)", IR_b, regs[4]);
		}
		if (log_wants(L_CRK5, 2)) {
			log_handle_syscall(L_CRK5, 2, IR_b, QNB, rIC, regs[4]);
		}
	}

	if (cpu_mem_get(0, 96, &data)) {
		cpu_ctx_switch(IR_b, data, 0b1111111110011111);
	}
}

// -----------------------------------------------------------------------
void op_71_brc()
{
	uint16_t b = IR_b + rMOD;
	if ((regs[0] & b) != b) {
		P = 1;
	}
}

// -----------------------------------------------------------------------
void op_71_nrf()
{
	int nrf_op = IR_A & 0b011;
	awp_dispatch(nrf_op, 0);
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_72_ric()
{
	reg_restrict_write(IR_A, rIC);
}

// -----------------------------------------------------------------------
void op_72_zlb()
{
	reg_restrict_write(IR_A, regs[IR_A] & 0b0000000011111111);
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
	reg_restrict_write(IR_A, regs[IR_A]<<1);
}

// -----------------------------------------------------------------------
void op_72_sly()
{
	uint16_t ir_a = regs[IR_A];
	reg_restrict_write(IR_A, (regs[IR_A]<<1) | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
}

// -----------------------------------------------------------------------
void op_72_slx()
{
	if (regs[IR_A] & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_restrict_write(IR_A, (regs[IR_A]<<1) | Fget(FL_X));
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	uint16_t ir_a = regs[IR_A];
	reg_restrict_write(IR_A, (regs[IR_A]>>1) | Fget(FL_Y)<<15);
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
	reg_restrict_write(IR_A, regs[0]);
}

// -----------------------------------------------------------------------
void op_72_shc()
{
	if (!IR_t) return;

	uint16_t falling = (regs[IR_A] & ((1<<IR_t)-1)) << (16-IR_t);

	reg_restrict_write(IR_A, (regs[IR_A] >> IR_t) | falling);
}

// -----------------------------------------------------------------------
void op_72_rky()
{
	reg_restrict_write(IR_A, rKB);
}

// -----------------------------------------------------------------------
void op_72_zrb()
{
	reg_restrict_write(IR_A, regs[IR_A] & 0b1111111100000000);
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
	reg_restrict_write(IR_A, regs[IR_A]<<1);
}

// -----------------------------------------------------------------------
void op_72_svy()
{
	uint16_t ir_a = regs[IR_A];
	reg_restrict_write(IR_A, regs[IR_A]<<1 | Fget(FL_Y));
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
	reg_restrict_write(IR_A, regs[IR_A]<<1 | Fget(FL_X));
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
	reg_restrict_write(IR_A, (regs[IR_A]>>1) | Fget(FL_X)<<15);
}

// -----------------------------------------------------------------------
void op_72_srz()
{
	if (regs[IR_A] & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_restrict_write(IR_A, (regs[IR_A]>>1) & 0b0111111111111111);
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
void op_73_hlt()
{
	USER_ILLEGAL;

	LOGCPU(L_OP, 1, "HALT 0%02o (alarm: %i)", N, regs[6]&255);

	// handle hlt>=040 as "exit emulation" if user wants to
	if (exit_on_hlt && (N >= 040)) {
		cpu_trigger_quit();
		return;
	// otherwise, enter halt state
	} else {
		cpu_trigger_halt();
	}
}

// -----------------------------------------------------------------------
void op_73_mcl()
{
	USER_ILLEGAL;
	cpu_trigger_clear(STATE_CLM);
}

// -----------------------------------------------------------------------
void op_73_softint()
{
	USER_ILLEGAL;

	// SIT, SIL, SIU, CIT
	if ((IR_C & 3) == 0) {
		int_clear(INT_SOFT_U);
		int_clear(INT_SOFT_L);
	} else {
		if ((IR_C & 1)) int_set(INT_SOFT_L);
		if ((IR_C & 2)) int_set(INT_SOFT_U);
	}

	// SINT, SIND
	if (cpu_mod_present && (IR_C & 4)) int_set(INT_TIMER);
}

// -----------------------------------------------------------------------
void op_73_giu()
{
	USER_ILLEGAL;

	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_gil()
{
	USER_ILLEGAL;

	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_lip()
{
	USER_ILLEGAL;

	cpu_ctx_restore();

	if (LOG_ENABLED) {
		log_update_process();
		if (log_wants(L_CRK5, 2)) {
			log_handle_syscall_ret(L_CRK5, 2, rIC, rSR, regs[4]);
		}
		if (log_wants(L_CRK5, 2)) {
			log_log_process(L_CRK5, 2);
		}
		log_intlevel_dec();
	}
}

// -----------------------------------------------------------------------
void op_73_cron()
{
	USER_ILLEGAL;

	if (cpu_mod_present) {
		cpu_mod_on();
	}
	// CRON is an illegal instruction anyway
	op_illegal();
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_74_uj()
{
	rIC = N;
}

// -----------------------------------------------------------------------
void op_74_jl()
{
	if (Fget(FL_L)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_je()
{
	if (Fget(FL_E)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_jg()
{
	if (Fget(FL_G)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_jz()
{
	if (Fget(FL_Z)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_jm()
{
	if (Fget(FL_M)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_jn()
{
	if (!Fget(FL_E)) rIC = N;
}

// -----------------------------------------------------------------------
void op_74_lj()
{
	if (cpu_mem_put(QNB, N, rIC)) {
		rIC = N+1;
	}
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_75_ld()
{
	cpu_mem_mget(QNB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_75_lf()
{
	cpu_mem_mget(QNB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_75_la()
{
	cpu_mem_mget(QNB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_75_ll()
{
	cpu_mem_mget(QNB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
void op_75_td()
{
	cpu_mem_mget(NB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_75_tf()
{
	cpu_mem_mget(NB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_75_ta()
{
	cpu_mem_mget(NB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_75_tl()
{
	cpu_mem_mget(NB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_76_rd()
{
	cpu_mem_mput(QNB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_76_rf()
{
	cpu_mem_mput(QNB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_76_ra()
{
	cpu_mem_mput(QNB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_76_rl()
{
	cpu_mem_mput(QNB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
void op_76_pd()
{
	cpu_mem_mput(NB, N, regs+1, 2);
}

// -----------------------------------------------------------------------
void op_76_pf()
{
	cpu_mem_mput(NB, N, regs+1, 3);
}

// -----------------------------------------------------------------------
void op_76_pa()
{
	cpu_mem_mput(NB, N, regs+1, 7);
}

// -----------------------------------------------------------------------
void op_76_pl()
{
	cpu_mem_mput(NB, N, regs+5, 3);
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_77_mb()
{
	USER_ILLEGAL;

	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		rSR = (rSR & 0b111111111000000) | (data & 0b0000000000111111);
	}
}

// -----------------------------------------------------------------------
void op_77_im()
{
	USER_ILLEGAL;

	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		rSR = (rSR & 0b000000000111111) | (data & 0b1111111111000000);
		int_update_mask(rSR);
	}
}

// -----------------------------------------------------------------------
void op_77_ki()
{
	USER_ILLEGAL;

	uint16_t data = int_get_nchan();
	cpu_mem_put(QNB, N, data);
}

// -----------------------------------------------------------------------
void op_77_fi()
{
	USER_ILLEGAL;

	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		int_put_nchan(data);
	}
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	USER_ILLEGAL;

	uint16_t data[3];
	if (cpu_mem_mget(NB, N, data, 3) != 3) return;

	rIC = data[0];
	regs[0] = data[1];
	rSR = data[2];

	int_update_mask(rSR);

	if (LOG_ENABLED) {
		log_update_process();
		log_intlevel_reset();
		if (log_wants(L_OP, 1)) {
			log_log_cpu(L_OP, 1, "SP: context @ 0x%04x", N);
		}
		if (log_wants(L_CRK5, 2)) {
			log_handle_syscall_ret(L_CRK5, 2, rIC, rSR, regs[4]);
		}
		if (log_wants(L_CRK5, 2)) {
			log_log_process(L_CRK5, 2);
		}
	}
}

// -----------------------------------------------------------------------
void op_77_md()
{
	if (rMODc >= 3) {
		LOGCPU(L_CPU, 2, "    (ineffective: 4th MD)");
		int_set(INT_ILLEGAL_INSTRUCTION);
		rMOD = 0;
		rMODc = 0;
		return;
	}
	rMOD = N;
	rMODc++;
}

// -----------------------------------------------------------------------
void op_77_rz()
{
	cpu_mem_put(QNB, N, 0);
}

// -----------------------------------------------------------------------
void op_77_ib()
{
	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		if (cpu_mem_put(QNB, N, ++data)) {
			if (data == 0) {
				P = 1;
			}
		}
	}
}

// -----------------------------------------------------------------------
void op_illegal()
{
	char *opcode = int2binf("... ... . ... ... ...", rIR, 16);
	LOGCPU(L_CPU, 2, "    (ineffective: illegal instruction) opcode: %s (0x%04x)", opcode, rIR);
	free(opcode);
	int_set(INT_ILLEGAL_INSTRUCTION);
}

// vim: tabstop=4 shiftwidth=4 autoindent
