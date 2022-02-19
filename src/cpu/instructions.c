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
	REG_RESTRICT_WRITE(IR_A, ac);
}

// -----------------------------------------------------------------------
void op_tw()
{
	uint16_t data;
	if (cpu_mem_read(true, ar, &data)) {
		REG_RESTRICT_WRITE(IR_A, data);
	}
}

// -----------------------------------------------------------------------
void op_ls()
{
	ar = ac & r[7];
	ac = r[IR_A] & ~r[7];
	REG_RESTRICT_WRITE(IR_A, ac | ar);
}

// -----------------------------------------------------------------------
void op_ri()
{
	ar = r[IR_A];
	if (cpu_mem_write(q, ar, ac)) {
		REG_RESTRICT_WRITE(IR_A, r[IR_A] + 1);
	}
}

// -----------------------------------------------------------------------
void op_rw()
{
	cpu_mem_write(q, ar, r[IR_A]);
}

// -----------------------------------------------------------------------
void op_pw()
{
	cpu_mem_write(true, ar, r[IR_A]);
}

// -----------------------------------------------------------------------
void op_rj()
{
	REG_RESTRICT_WRITE(IR_A, ic);
	ic = ac;
}

// -----------------------------------------------------------------------
void op_is()
{
	if (!cpu_mem_read(true, ar, &ac)) return;

	if ((ac & r[IR_A]) == r[IR_A]) {
		p = true;
	} else {
		cpu_mem_write(true, ar, ac | r[IR_A]);
	}
}

// -----------------------------------------------------------------------
void op_bb()
{
	p = (r[IR_A] & ac) == ac;
}

// -----------------------------------------------------------------------
void op_bm()
{
	if (cpu_mem_read(true, ar, &ac)) {
		p = (ac & r[IR_A]) == r[IR_A];
	}
}

// -----------------------------------------------------------------------
void op_bs()
{
	ac ^= r[IR_A];
	p = !(ac & r[7]);
}

// -----------------------------------------------------------------------
void op_bc()
{
	p = (r[IR_A] & ac) != ac;
}

// -----------------------------------------------------------------------
void op_bn()
{
	p = (r[IR_A] & ac) == 0;
}

// -----------------------------------------------------------------------
void op_ou()
{
	ic += io_dispatch(IO_OU, ar, r+IR_A);
	cpu_mem_read(q, ic, &ic);
}

// -----------------------------------------------------------------------
void op_in()
{
	ic += io_dispatch(IO_IN, ar, r+IR_A);
	cpu_mem_read(q, ic, &ic);
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_37_ad()
{
	awp_dispatch(AWP_AD, ar);
}

// -----------------------------------------------------------------------
void op_37_sd()
{
	awp_dispatch(AWP_SD, ar);
}

// -----------------------------------------------------------------------
void op_37_mw()
{
	awp_dispatch(AWP_MW, ar);
}

// -----------------------------------------------------------------------
void op_37_dw()
{
	awp_dispatch(AWP_DW, ar);
}

// -----------------------------------------------------------------------
void op_37_af()
{
	awp_dispatch(AWP_AF, ar);
}

// -----------------------------------------------------------------------
void op_37_sf()
{
	awp_dispatch(AWP_SF, ar);
}

// -----------------------------------------------------------------------
void op_37_mf()
{
	awp_dispatch(AWP_MF, ar);
}

// -----------------------------------------------------------------------
void op_37_df()
{
	awp_dispatch(AWP_DF, ar);
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_aw()
{
	alu_16_add(r[IR_A], ac, 0);
}

// -----------------------------------------------------------------------
void op_ac()
{
	alu_16_add(r[IR_A], ac, FGET(FL_C));
}

// -----------------------------------------------------------------------
void op_sw()
{
	alu_16_sub(r[IR_A], ac);
}

// -----------------------------------------------------------------------
void op_cw()
{
	alu_16_set_LEG((int16_t) r[IR_A], (int16_t) ac);
}

// -----------------------------------------------------------------------
void op_or()
{
	uint16_t data = r[IR_A] | ac;
	alu_16_set_Z_bool(data);
	// reg writes needs to go after flag setting to cover corner cases
	// where logical operations are performed on r0
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_om()
{
	if (cpu_mem_read(true, ar, &ac)) {
		uint16_t data = ac | r[IR_A];
		alu_16_set_Z_bool(data);
		cpu_mem_write(true, ar, data);
	}
}

// -----------------------------------------------------------------------
void op_nr()
{
	uint16_t data = r[IR_A] & ac;
	alu_16_set_Z_bool(data);
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_nm()
{
	if (cpu_mem_read(true, ar, &ac)) {
		uint16_t data = ac & r[IR_A];
		alu_16_set_Z_bool(data);
		cpu_mem_write(true, ar, data);
	}
}

// -----------------------------------------------------------------------
void op_er()
{
	uint16_t data = r[IR_A] & ~ac;
	alu_16_set_Z_bool(data);
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_em()
{
	if (cpu_mem_read(true, ar, &ac)) {
		uint16_t data = ac & ~r[IR_A];
		alu_16_set_Z_bool(data);
		cpu_mem_write(true, ar, data);
	}
}

// -----------------------------------------------------------------------
void op_xr()
{
	uint16_t data = r[IR_A] ^ ac;
	alu_16_set_Z_bool(data);
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_xm()
{
	if (cpu_mem_read(true, ar, &ac)) {
		uint16_t data = ac ^ r[IR_A];
		alu_16_set_Z_bool(data);
		cpu_mem_write(true, ar, data);
	}
}

// -----------------------------------------------------------------------
void op_cl()
{
	alu_16_set_LEG(r[IR_A], ac);
}

// -----------------------------------------------------------------------
static inline int cpu_byte_addr_fixup()
{
	int shift = 8 * (~ar & 1);
	ar >>= 1;

	// fixup address if 17-bit byte addressing is active
	if (cpu_mod_active && !(q & bs)) {
		ar |= zc17 << 15;
	}

	return shift;
}

// -----------------------------------------------------------------------
void op_lb()
{
	int shift = cpu_byte_addr_fixup();

	if (!cpu_mem_read(true, ar, &ac)) return;
	ac >>= shift;

	REG_RESTRICT_WRITE(IR_A, (r[IR_A] & 0xff00) | (ac & 0xff));
}

// -----------------------------------------------------------------------
void op_rb()
{
	int shift = cpu_byte_addr_fixup();

	if (!cpu_mem_read(true, ar, &ac)) return;
	cpu_mem_write(true, ar, (ac & (0xff00 >> shift)) | ((r[IR_A] & 0xff) << shift));
}

// -----------------------------------------------------------------------
void op_cb()
{
	int shift = cpu_byte_addr_fixup();

	if (!cpu_mem_read(true, ar, &ac)) return;
	ac >>= shift;
	alu_16_set_LEG((uint8_t) r[IR_A], ac & 0xff);
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_awt()
{
	alu_16_add(r[IR_A], ac, 0);
}

// -----------------------------------------------------------------------
void op_trb()
{
	REG_RESTRICT_WRITE(IR_A, r[IR_A] + ac);
	p = r[IR_A] == 0;
}

// -----------------------------------------------------------------------
void op_irb()
{
	REG_RESTRICT_WRITE(IR_A, r[IR_A] + 1);
	if (r[IR_A]) ic += ac;
}

// -----------------------------------------------------------------------
void op_drb()
{
	REG_RESTRICT_WRITE(IR_A, r[IR_A] - 1);
	if (r[IR_A] != 0) ic += ac;
}

// -----------------------------------------------------------------------
void op_cwt()
{
	alu_16_set_LEG((int16_t) r[IR_A], (int16_t) ac);
}

// -----------------------------------------------------------------------
void op_lwt()
{
	REG_RESTRICT_WRITE(IR_A, ac);
}

// -----------------------------------------------------------------------
void op_lws()
{
	uint16_t data;
	ar = ic + ac;
	if (cpu_mem_read(q, ar, &data)) {
		REG_RESTRICT_WRITE(IR_A, data);
	}
}

// -----------------------------------------------------------------------
void op_rws()
{
	ar = ic + ac;
	cpu_mem_write(q, ar, r[IR_A]);
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_70_jump()
{
	ic += ac;
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
	ic += ac;
	FCLR(FL_V);
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_71_blc()
{
	p = ((r[0] >> 8) & ac) != ac;
}

// -----------------------------------------------------------------------
void op_71_exl()
{
	uint16_t data;

	if (LOG_ENABLED) {
		if (LOG_WANTS(L_OP)) {
			log_log_cpu(L_OP, "EXL: %i (r4: 0x%04x)", ac, r[4]);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall(L_CRK5, ac, QNB, ic, r[4]);
		}
	}

	if (cpu_mem_read(false, EXL_VECTOR, &data)) {
		cpu_ctx_switch(ac, data, MASK_9);
	}
}

// -----------------------------------------------------------------------
void op_71_brc()
{
	p = (r[0] & ac) != ac;
}

// -----------------------------------------------------------------------
void op_71_nrf()
{
	int nrf_op = IR_A & 0b011; // used by soft-awp, apparently (TODO: check in h/w)
	awp_dispatch(nrf_op, ar);
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_72_ric()
{
	REG_RESTRICT_WRITE(IR_A, ic);
}

// -----------------------------------------------------------------------
void op_72_zlb()
{
	REG_RESTRICT_WRITE(IR_A, r[IR_A] & 0xff);
}

// -----------------------------------------------------------------------
void op_72_sxu()
{
	if (r[IR_A] & 0x8000) {
		FSET(FL_X);
	} else {
		FCLR(FL_X);
	}
}

// -----------------------------------------------------------------------
void op_72_nga()
{
	ac = r[IR_A];
	alu_16_add(~ac, 0, 1);
}

// -----------------------------------------------------------------------
void shift_left(uint16_t shift_in, int check_v)
{
	uint16_t data = (r[IR_A] << 1) | shift_in;
	if (check_v && ((r[IR_A] ^ data) & 0x8000)) {
		FSET(FL_V);
	}
	if (r[IR_A] & 0x8000) FSET(FL_Y);
	else FCLR(FL_Y);
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_72_slz()
{
	shift_left(0, 0);
}

// -----------------------------------------------------------------------
void op_72_sly()
{
	shift_left(FGET(FL_Y), 0);
}

// -----------------------------------------------------------------------
void op_72_slx()
{
	shift_left(FGET(FL_X), 0);
}

// -----------------------------------------------------------------------
void op_72_svz()
{
	shift_left(0, 1);
}

// -----------------------------------------------------------------------
void op_72_svy()
{
	shift_left(FGET(FL_Y), 1);
}

// -----------------------------------------------------------------------
void op_72_svx()
{
	shift_left(FGET(FL_X), 1);
}

// -----------------------------------------------------------------------
static inline void shift_right(uint16_t shift_in)
{
	uint16_t data = (r[IR_A] >> 1) | shift_in;
	if (r[IR_A] & 1) FSET(FL_Y);
	else FCLR(FL_Y);
	REG_RESTRICT_WRITE(IR_A, data);
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	shift_right(FGET(FL_Y) << 15);
}

// -----------------------------------------------------------------------
void op_72_srx()
{
	shift_right(FGET(FL_X) << 15);
}

// -----------------------------------------------------------------------
void op_72_srz()
{
	shift_right(0);
}

// -----------------------------------------------------------------------
void op_72_ngl()
{
	uint16_t data = ~r[IR_A];
	alu_16_set_Z_bool(data);
	r[IR_A] = data;
}

// -----------------------------------------------------------------------
void op_72_rpc()
{
	REG_RESTRICT_WRITE(IR_A, r[0]);
}

// -----------------------------------------------------------------------
void op_72_shc()
{
	if (!IR_t) return;

	uint16_t data = (r[IR_A] & ((1 << IR_t) - 1)) << (16 - IR_t);

	REG_RESTRICT_WRITE(IR_A, (r[IR_A] >> IR_t) | data);
}

// -----------------------------------------------------------------------
void op_72_rky()
{
	REG_RESTRICT_WRITE(IR_A, kb);
}

// -----------------------------------------------------------------------
void op_72_zrb()
{
	REG_RESTRICT_WRITE(IR_A, r[IR_A] & 0xff00);
}

// -----------------------------------------------------------------------
void op_72_sxl()
{
	if (r[IR_A] & 1) {
		FSET(FL_X);
	} else {
		FCLR(FL_X);
	}
}

// -----------------------------------------------------------------------
void op_72_ngc()
{
	ac = r[IR_A];
	alu_16_add(~ac, 0, FGET(FL_C));
}

// -----------------------------------------------------------------------
void op_72_lpc()
{
	r[0] = r[IR_A];
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_73_hlt()
{
	LOGCPU(L_OP, "HALT 0%02o (alarm: %i)", ac, r[6] & 0xff);
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
			log_handle_syscall_ret(L_CRK5, ic, SR_READ(), r[4]);
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
	ic = ac;
}

// -----------------------------------------------------------------------
void op_74_lj()
{
	if (cpu_mem_write(q, ar, ic)) {
		ic = ac+1;
	}
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static inline void load_multiword(bool barnb, int start, int end)
{
	for (int i=start ; i<=end ; i++) {
		if (!cpu_mem_read(barnb, ar, r+i)) return;
		ar++;
	}
}

// -----------------------------------------------------------------------
void op_75_ld()
{
	load_multiword(q, 1, 2);
}

// -----------------------------------------------------------------------
void op_75_lf()
{
	load_multiword(q, 1, 3);
}

// -----------------------------------------------------------------------
void op_75_la()
{
	load_multiword(q, 1, 7);
}

// -----------------------------------------------------------------------
void op_75_ll()
{
	load_multiword(q, 5, 7);
}

// -----------------------------------------------------------------------
void op_75_td()
{
	load_multiword(true, 1, 2);
}

// -----------------------------------------------------------------------
void op_75_tf()
{
	load_multiword(true, 1, 3);
}

// -----------------------------------------------------------------------
void op_75_ta()
{
	load_multiword(true, 1, 7);
}

// -----------------------------------------------------------------------
void op_75_tl()
{
	load_multiword(true, 5, 7);
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static inline void store_multiword(bool barnb, int start, int end)
{
	for (int i=start ; i<=end ; i++) {
		if (!cpu_mem_write(barnb, ar, r[i])) return;
		ar++;
	}
}

// -----------------------------------------------------------------------
void op_76_rd()
{
	store_multiword(q, 1, 2);
}

// -----------------------------------------------------------------------
void op_76_rf()
{
	store_multiword(q, 1, 3);
}

// -----------------------------------------------------------------------
void op_76_ra()
{
	store_multiword(q, 1, 7);
}

// -----------------------------------------------------------------------
void op_76_rl()
{
	store_multiword(q, 5, 7);
}

// -----------------------------------------------------------------------
void op_76_pd()
{
	store_multiword(true, 1, 2);
}

// -----------------------------------------------------------------------
void op_76_pf()
{
	store_multiword(true, 1, 3);
}

// -----------------------------------------------------------------------
void op_76_pa()
{
	store_multiword(true, 1, 7);
}

// -----------------------------------------------------------------------
void op_76_pl()
{
	store_multiword(true, 5, 7);
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_77_mb()
{
	uint16_t data;
	if (cpu_mem_read(q, ar, &data)) {
		q =  data & 0b100000;
		bs = data & 0b010000;
		nb = data & 0b001111;

	}
}

// -----------------------------------------------------------------------
void op_77_im()
{
	uint16_t data;
	if (cpu_mem_read(q, ar, &data)) {
		rm = (data >> 6) & 0b1111111111;
		int_update_mask(rm);
	}
}

// -----------------------------------------------------------------------
void op_77_ki()
{
	uint16_t data = int_get_nchan();
	cpu_mem_write(q, ar, data);
}

// -----------------------------------------------------------------------
void op_77_fi()
{
	uint16_t data;
	if (cpu_mem_read(q, ar, &data)) {
		int_put_nchan(data);
	}
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	uint16_t sr;
	if (!cpu_mem_read(true, ar, &ic)) return;
	if (!cpu_mem_read(true, ++ar, r+0)) return;
	if (!cpu_mem_read(true, ++ar, &sr)) return;
	SR_WRITE(sr);
	int_update_mask(rm);

	if (LOG_ENABLED) {
		log_update_process();
		log_intlevel_reset();
		if (LOG_WANTS(L_OP)) {
			log_log_cpu(L_OP, "SP: context @ 0x%04x", ac);
		}
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall_ret(L_CRK5, ic, SR_READ(), r[4]);
			log_log_process(L_CRK5);
		}
	}
}

// -----------------------------------------------------------------------
void op_77_md()
{
	if (mc < 3) {
		mc++;
	} else {
		mc = 0;
		int_set(INT_ILLEGAL_INSTRUCTION);
		LOGCPU(L_CPU, "    (ineffective: 4th MD)");
	}
}

// -----------------------------------------------------------------------
void op_77_rz()
{
	cpu_mem_write(q, ar, 0);
}

// -----------------------------------------------------------------------
void op_77_ib()
{
	if (cpu_mem_read(q, ar, &ac)) {
		ac++;
		p = (ac == 0);
		cpu_mem_write(q, ar, ac);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
