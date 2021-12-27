//  Copyright (c) 2012-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#include "utils/utils.h"
#include "log.h"
#include "log_crk.h"

#include "ectl.h" // for global constants

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
	uint16_t data;
	int io_result = io_dispatch(IO_OU, N, regs+IR_A);
	if (cpu_mem_get(QNB, rIC + io_result, &data)) {
		rIC = data;
	}
}

// -----------------------------------------------------------------------
void op_in()
{
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
	alu_16_add(regs[IR_A], N, 0);
}

// -----------------------------------------------------------------------
void op_ac()
{
	alu_16_add(regs[IR_A], N, Fget(FL_C));
}

// -----------------------------------------------------------------------
void op_sw()
{
	alu_16_sub(regs[IR_A], N);
}

// -----------------------------------------------------------------------
void op_cw()
{
	alu_16_set_LEG((int16_t) regs[IR_A], (int16_t) N);
}

// -----------------------------------------------------------------------
void op_or()
{
	uint16_t result = regs[IR_A] | N;
	alu_16_set_Z_bool(result);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_om()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data |= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z_bool(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_nr()
{
	uint16_t result = regs[IR_A] & N;
	alu_16_set_Z_bool(result);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_nm()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data &= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z_bool(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_er()
{
	uint16_t result = regs[IR_A] & ~N;
	alu_16_set_Z_bool(result);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_em()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data &= ~regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z_bool(data);
		}
	}
}

// -----------------------------------------------------------------------
void op_xr()
{
	uint16_t result = regs[IR_A] ^ N;
	alu_16_set_Z_bool(result);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_xm()
{
	uint16_t data;
	if (cpu_mem_get(NB, N, &data)) {
		data ^= regs[IR_A];
		if (cpu_mem_put(NB, N, data)) {
			alu_16_set_Z_bool(data);
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
		reg_restrict_write(IR_A, (regs[IR_A] & 0xff00) | data);
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
	alu_16_add(regs[IR_A], N, 0);
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
void op_70_jump()
{
	rIC += N;
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
	rIC += N;
	Fclr(FL_V);
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
		if (LOG_WANTS(L_OP)) {
			log_log_cpu(L_OP, "EXL: %i (r4: 0x%04x)", IR_b, regs[4]);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall(L_CRK5, IR_b, QNB, rIC, regs[4]);
		}
	}

	if (cpu_mem_get(0, 96, &data)) {
		cpu_ctx_switch(IR_b, data, MASK_9);
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
	awp_dispatch(nrf_op, IR_b);
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
	reg_restrict_write(IR_A, regs[IR_A] & 0x00ff);
}

// -----------------------------------------------------------------------
void op_72_sxu()
{
	if (regs[IR_A] & 0x8000) {
		Fset(FL_X);
	} else {
		Fclr(FL_X);
	}
}

// -----------------------------------------------------------------------
void op_72_nga()
{
	alu_16_add(~regs[IR_A], 0, 1);
}

// -----------------------------------------------------------------------
void shift_left(uint16_t shift_in, int check_v)
{
	uint16_t result = regs[IR_A]<<1 | shift_in;
	if (check_v && ((regs[IR_A] ^ result) & 0x8000)) {
		Fset(FL_V);
	}
	if (regs[IR_A] & 0x8000) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_72_slz()
{
	shift_left(0, 0);
}

// -----------------------------------------------------------------------
void op_72_sly()
{
	shift_left(Fget(FL_Y), 0);
}

// -----------------------------------------------------------------------
void op_72_slx()
{
	shift_left(Fget(FL_X), 0);
}

// -----------------------------------------------------------------------
void op_72_svz()
{
	shift_left(0, 1);
}

// -----------------------------------------------------------------------
void op_72_svy()
{
	shift_left(Fget(FL_Y), 1);
}

// -----------------------------------------------------------------------
void op_72_svx()
{
	shift_left(Fget(FL_X), 1);
}

// -----------------------------------------------------------------------
void shift_right(uint16_t shift_in)
{
	uint16_t result = (regs[IR_A]>>1) | shift_in;
	if (regs[IR_A] & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	reg_restrict_write(IR_A, result);
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	shift_right(Fget(FL_Y)<<15);
}

// -----------------------------------------------------------------------
void op_72_srx()
{
	shift_right(Fget(FL_X)<<15);
}

// -----------------------------------------------------------------------
void op_72_srz()
{
	shift_right(0);
}

// -----------------------------------------------------------------------
void op_72_ngl()
{
	uint16_t result = ~regs[IR_A];
	alu_16_set_Z_bool(result);
	regs[IR_A] = result;
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
	reg_restrict_write(IR_A, regs[IR_A] & 0xff00);
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
	alu_16_add(~regs[IR_A], 0, Fget(FL_C));
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
	LOGCPU(L_OP, "HALT 0%02o (alarm: %i)", N, regs[6]&255);
	cpu_state_change(ECTL_STATE_WAIT, ECTL_STATE_RUN);
}

// -----------------------------------------------------------------------
void op_73_mcl()
{
	cpu_state_change(ECTL_STATE_CLM, ECTL_STATE_RUN);
}

// -----------------------------------------------------------------------
void op_73_softint()
{
	// SIT, SIL, SIU, CIT
	if ((IR_C & 3) == 0) {
		int_clear(INT_SOFT_U);
		int_clear(INT_SOFT_L);
	} else {
		if ((IR_C & 1)) int_set(INT_SOFT_L);
		if ((IR_C & 2)) int_set(INT_SOFT_U);
	}

	// SINT, SIND
	if (cpu_mod_present && (IR_C & 4)) int_set(INT_CLOCK);
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

	if (LOG_ENABLED) {
		log_update_process();
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall_ret(L_CRK5, rIC, SR_read(), regs[4]);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_log_process(L_CRK5);
		}
		log_intlevel_dec();
	}
}

// -----------------------------------------------------------------------
void op_73_cron()
{
	if (cpu_mod_present) {
		cpu_mod_on();
	}
	// CRON is an illegal instruction anyway
	int_set(INT_ILLEGAL_INSTRUCTION);
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_74_jump()
{
	rIC = N;
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
	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		Q = (data >> 5) & 1;
		BS = (data >> 4) & 1;
		NB = data & 0b1111;
	}
}

// -----------------------------------------------------------------------
void op_77_im()
{
	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		RM = (data >> 6) & 0b1111111111;
		int_update_mask(RM);
	}
}

// -----------------------------------------------------------------------
void op_77_ki()
{
	uint16_t data = int_get_nchan();
	cpu_mem_put(QNB, N, data);
}

// -----------------------------------------------------------------------
void op_77_fi()
{
	uint16_t data;
	if (cpu_mem_get(QNB, N, &data)) {
		int_put_nchan(data);
	}
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	uint16_t data[3];
	if (cpu_mem_mget(NB, N, data, 3) != 3) return;

	rIC = data[0];
	regs[0] = data[1];
	SR_write(data[2]);

	int_update_mask(RM);

	if (LOG_ENABLED) {
		log_update_process();
		log_intlevel_reset();
		if (LOG_WANTS(L_OP)) {
			log_log_cpu(L_OP, "SP: context @ 0x%04x", N);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall_ret(L_CRK5, rIC, SR_read(), regs[4]);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_log_process(L_CRK5);
		}
	}
}

// -----------------------------------------------------------------------
void op_77_md()
{
	if (rMODc >= 3) {
		LOGCPU(L_CPU, "    (ineffective: 4th MD)");
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

// vim: tabstop=4 shiftwidth=4 autoindent
