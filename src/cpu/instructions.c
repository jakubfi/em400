//  Copyright (c) 2012-2022 Jakub Filipowicz <jakubf@gmail.com>
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

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "cpu/alu.h"
#include "io/defs.h"
#include "io/io.h"

#include "log.h"
#include "log_crk.h"

#include "ectl.h" // for global constants

// Instructions are implemented to mimic CPU FSM behavior.
// CPU states are indicated in (usually commented out) labels.

// -----------------------------------------------------------------------
void step_point()
{

}

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_lw()
{
// WP:
	w = ac;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_tw()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_ls()
{
	static bool wls;
WE:
	if (wls) {
		step_point();
		at = ~r[7] & ac;
		w = at;
		ac = w;
		goto WS;
	} else {
		step_point();
		at =  r[7] & ac;
		w = at;
		ar = w;
		goto WA;
	}
WA:
	w = r[IR_A];
	step_point();
	ac = w;
	wls = true;
	goto WE;
WS:
	wls = false;
	step_point();
	at = ac | ar;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
	return;
}

// -----------------------------------------------------------------------
void op_ri()
{
// WA:
	w = r[IR_A];
	ar = w;
	step_point();
// WW:
	w = ac;
	step_point();
	cpu_mem_write_1(q, ar, w);
// W&:
	step_point();
	at = r[IR_A] + 1;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_rw()
{
// WW:
	w = r[IR_A];
	step_point();
	cpu_mem_write_1(q, ar, w);
}

// -----------------------------------------------------------------------
void op_pw()
{
// WW:
	w = r[IR_A];
	step_point();
	cpu_mem_write_1(true, ar, w);
}

// -----------------------------------------------------------------------
void op_rj()
{
// WA:
	w = ic;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
// WP:
	w = ac;
	step_point();
	ic = w;
}

// -----------------------------------------------------------------------
void op_is()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// W&:
	step_point();
	at = r[IR_A] & ~ac;
	w = at;
	if (at == 0) {
		p = true;
		return;
	}
// WZ:
	step_point();
	at = r[IR_A] | ac;
	w = at;
// WW:
	cpu_mem_write_1(true, ar, w);
}

// -----------------------------------------------------------------------
void op_bb()
{
// W&:
	step_point();
	at = ~r[IR_A] & ac;
	w = at;
	p = (at == 0);
}

// -----------------------------------------------------------------------
void op_bm()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// W&:
	step_point();
	at = r[IR_A] & ~ac;
	w = at;
	p = (at == 0);
}

// -----------------------------------------------------------------------
void op_bs()
{
// WE:
	step_point();
	at = r[IR_A] ^ ac;
	w = at;
	ac = w;
// W&:
	step_point();
	at = r[7] & ac;
	w = at;
	p = (at == 0);
}

// -----------------------------------------------------------------------
void op_bc()
{
// W&:
	step_point();
	at = r[IR_A] | ~ac;
	w = at;
	p = (at != 0xffff);
}

// -----------------------------------------------------------------------
void op_bn()
{
// WS:
	step_point();
	at = r[IR_A] & ac;
	w = at;
	p = (at == 0);
}

// -----------------------------------------------------------------------
void op_ou()
{
	// TODO: FSM
	ic += io_dispatch(IO_OU, ar, r+IR_A);
	cpu_mem_read_1(q, ic, &ic);
}

// -----------------------------------------------------------------------
void op_in()
{
	// TODO: FSM
	ic += io_dispatch(IO_IN, ar, r+IR_A);
	cpu_mem_read_1(q, ic, &ic);
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_37_ad()
{
	// TODO: FSM
	awp_dispatch(AWP_AD, ar);
}

// -----------------------------------------------------------------------
void op_37_sd()
{
	// TODO: FSM
	awp_dispatch(AWP_SD, ar);
}

// -----------------------------------------------------------------------
void op_37_mw()
{
	// TODO: FSM
	awp_dispatch(AWP_MW, ar);
}

// -----------------------------------------------------------------------
void op_37_dw()
{
	// TODO: FSM
	awp_dispatch(AWP_DW, ar);
}

// -----------------------------------------------------------------------
void op_37_af()
{
	// TODO: FSM
	awp_dispatch(AWP_AF, ar);
}

// -----------------------------------------------------------------------
void op_37_sf()
{
	// TODO: FSM
	awp_dispatch(AWP_SF, ar);
}

// -----------------------------------------------------------------------
void op_37_mf()
{
	// TODO: FSM
	awp_dispatch(AWP_MF, ar);
}

// -----------------------------------------------------------------------
void op_37_df()
{
	// TODO: FSM
	awp_dispatch(AWP_DF, ar);
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_aw()
{
// W&:
	step_point();
	alu_16_add(r[IR_A], ac, 0);
}

// -----------------------------------------------------------------------
void op_ac()
{
// W&:
	step_point();
	alu_16_add(r[IR_A], ac, FGET(FL_C));
}

// -----------------------------------------------------------------------
void op_sw()
{
// W&:
	step_point();
	alu_16_sub(r[IR_A], ac);
}

// -----------------------------------------------------------------------
void op_cw()
{
// W&:
	step_point();
	alu_16_set_LEG((int16_t) r[IR_A], (int16_t) ac);
}

// -----------------------------------------------------------------------
void op_or()
{
// W&:
	step_point();
	at = r[IR_A] | ac;
	w = at;
	alu_16_set_Z_bool(w);
	// reg writes needs to go after flag setting to cover corner cases
	// where logical operations are performed on r0
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_om()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// WZ:
	step_point();
	at = r[IR_A] | ac;
	alu_16_set_Z_bool(at);
// WW:
	step_point();
	cpu_mem_write_1(true, ar, at);
}

// -----------------------------------------------------------------------
void op_nr()
{
// W&:
	step_point();
	at = r[IR_A] & ac;
	w = at;
	alu_16_set_Z_bool(w);
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_nm()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// WZ:
	step_point();
	at = r[IR_A] & ac;
	alu_16_set_Z_bool(at);
// WW:
	step_point();
	cpu_mem_write_1(true, ar, at);
}

// -----------------------------------------------------------------------
void op_er()
{
// W&:
	step_point();
	at = r[IR_A] & ~ac;
	w = at;
	alu_16_set_Z_bool(w);
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_em()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// WZ:
	step_point();
	at = ~r[IR_A] & ac;
	alu_16_set_Z_bool(at);
// WW:
	step_point();
	cpu_mem_write_1(true, ar, at);
}

// -----------------------------------------------------------------------
void op_xr()
{
// W&:
	step_point();
	at = r[IR_A] ^ ac;
	w = at;
	alu_16_set_Z_bool(w);
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_xm()
{
// WR:
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// WZ:
	step_point();
	at = r[IR_A] ^ ac;
	alu_16_set_Z_bool(at);
// WW:
	step_point();
	cpu_mem_write_1(true, ar, at);
}

// -----------------------------------------------------------------------
void op_cl()
{
// W&:
	step_point();
	alu_16_set_LEG(r[IR_A], ac);
}

// -----------------------------------------------------------------------
void op_lb()
{
	bool pb;

// WZ:
	step_point();
	at = ac;
// WX:
	step_point();
	pb = at & 1;
	at >>= 1;
// WP:
	w = at;
	step_point();
	ar = w;
// WR:
	// fixup address if 17-bit byte addressing is active
	if (cpu_mod_active && !(q && bs)) ar |= zc17 << 15;
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
	if (pb) goto Wand;
// WA:
	w = ac >> 8;
	step_point();
	ac = w;
Wand:
	step_point();
	at = (r[IR_A] & 0xff00) | (ac & 0xff);
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_rb()
{
	bool pb;

// WZ:
	step_point();
	at = ac;
// WX:
	step_point();
	pb = at & 1;
	at >>= 1;
// WP:
	w = at;
	step_point();
	ar = w;
// WR:
	// fixup address if 17-bit byte addressing is active
	if (cpu_mod_active && !(q && bs)) ar |= zc17 << 15;
	cpu_mem_read_1(true, ar, &w);
	step_point();
	ac = w;
// W&:
	step_point();
	if (pb) {
		at = (ac & 0xff00) | (r[IR_A] & 0xff);
	} else {
		at = (r[IR_A] & 0xff) << 8 | (ac & 0x00ff);
	}
	w = at;
// WW:
	step_point();
	cpu_mem_write_1(true, ar, w);
}

// -----------------------------------------------------------------------
void op_cb()
{
	bool pb;

// WZ:
	step_point();
	at = ac;
// WX:
	step_point();
	pb = at & 1;
	at >>= 1;
// WP:
	w = at;
	step_point();
	ar = w;
// WR:
	// fixup address if 17-bit byte addressing is active
	if (cpu_mod_active && !(q && bs)) ar |= zc17 << 15;
	cpu_mem_read_1(true, ar, &w);
	if (pb) w &= 0x00ff;
	step_point();
	ac = w;
	if (pb) goto Wand;
// WA:
	w = ac >> 8;
	step_point();
	ac = w;
Wand:
	step_point();
	alu_16_set_LEG(r[IR_A] & 0x00ff, ac);
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_awt()
{
// W&:
	step_point();
	alu_16_add(r[IR_A], ac, 0);
}

// -----------------------------------------------------------------------
void op_trb()
{
// W&:
	step_point();
	at = r[IR_A] + ac;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
	p = (at == 0);
}

// -----------------------------------------------------------------------
void op_irb()
{
// W&:
	step_point();
	at = r[IR_A] + 1;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
	if (at == 0) return;
// WE:
	step_point();
	at = ic + ac;
	w = at;
	ic = w;
}

// -----------------------------------------------------------------------
void op_drb()
{
// W&:
	step_point();
	at = r[IR_A] - 1;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
	if (at == 0) return;
// WE:
	step_point();
	at = ic + ac;
	w = at;
	ic = w;
}

// -----------------------------------------------------------------------
void op_cwt()
{
// W&:
	step_point();
	alu_16_set_LEG((int16_t) r[IR_A], (int16_t) ac);
}

// -----------------------------------------------------------------------
void op_lwt()
{
// WP:
	w = ac;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_lws()
{
// WE:
	step_point();
	at = ic + ac;
	w = at;
	ar = w;
// WR:
	cpu_mem_read_1(q, ar, &w);
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_rws()
{
// WE:
	step_point();
	at = ic + ac;
	w = at;
	ar = w;
// WW:
	w = r[IR_A];
	step_point();
	cpu_mem_write_1(q, ar, w);
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_70_jump()
{
// WE:
	step_point();
	at = ic + ac;
	w = at;
	ic = w;
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
// WE:
	FCLR(FL_V);
	op_70_jump();
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_71_blc()
{
// W&:
	step_point();
	at = (r[0] >> 8) | ~ac;
	p = (at != 0xffff);
}

// -----------------------------------------------------------------------
void op_71_exl()
{
	uint16_t data;

	if (LOG_ENABLED) {
		log_cpu(L_OP, "EXL: %i (r4: 0x%04x)", ac, r[4]);
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall(L_CRK5, ac, nb, ic, r[4]);
		}
	}

	if (cpu_mem_read_1(false, EXL_VECTOR, &data)) {
		cpu_ctx_switch(ac, data, MASK_9);
	}
}

// -----------------------------------------------------------------------
void op_71_brc()
{
// W&:
	step_point();
	at = (r[0] & 0xff) | ~ac;
	p = (at != 0xffff);
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
// WA:
	w = ic;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_zlb()
{
// WE:
	step_point();
	at = r[IR_A] & 0xff;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_sxu()
{
// WA:
	w = r[IR_A];
	step_point();
	if (r[IR_A] & 0x8000) FSET(FL_X);
	else FCLR(FL_X);
}

// -----------------------------------------------------------------------
void op_72_nga()
{
// WA:
	w = r[IR_A];
	step_point();
	ac = w;
// W&:
	step_point();
	alu_16_sub(0, ac);
}

// -----------------------------------------------------------------------
void shift_left(uint16_t shift_in, int check_v)
{
// W&:
	step_point();
	at = (r[IR_A] << 1) + shift_in;
	if (check_v && ((r[IR_A] ^ at) & 0x8000)) {
		FSET(FL_V);
	}
	if (r[IR_A] & 0x8000) FSET(FL_Y);
	else FCLR(FL_Y);
	REG_RESTRICT_WRITE(IR_A, at);
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
// WZ:
	step_point();
	at = r[IR_A];
// WX:
	step_point();
	if (at & 1) FSET(FL_Y);
	else FCLR(FL_Y);
	at >>= 1;
	at |= shift_in << 15;
// WP:
	w = at;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	shift_right(FGET(FL_Y));
}

// -----------------------------------------------------------------------
void op_72_srx()
{
	shift_right(FGET(FL_X));
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
	unsigned lk = IR_t;
// WZ:
	step_point();
	at = r[IR_A];
WX:
	lk = (lk - 1) & 0b1111;
	step_point();
	bool shift_in = at & 1;
	at >>= 1;
	at |= shift_in << 15;
	REG_RESTRICT_WRITE(IR_A, at);
	if (lk) goto WX;
// WP:
	step_point();
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_rky()
{
// WA:
	w = kb;
	step_point();
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_zrb()
{
// WE:
	step_point();
	at = r[IR_A] & 0xff00;
	w = at;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void op_72_sxl()
{
// WA:
	w = r[IR_A];
	step_point();
	if (r[IR_A] & 1) FSET(FL_X);
	else FCLR(FL_X);
}

// -----------------------------------------------------------------------
void op_72_ngc()
{
// WA:
	w = r[IR_A];
	step_point();
	ac = w;
// W&:
	step_point();
	alu_16_add(~ac, 0, FGET(FL_C)); // IDK how to P16 and alu_16_sub
}

// -----------------------------------------------------------------------
void op_72_lpc()
{
// WA:
	w = r[IR_A];
	step_point();
	r[0] = w;
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_73_hlt()
{
	log_cpu(L_OP, "HALT 0%02o (alarm: %i)", ac, r[6] & 0xff);
// WX:
	step_point();
	cpu_state_change(ECTL_STATE_WAIT, ECTL_STATE_ANY);
}

// -----------------------------------------------------------------------
void op_73_mcl()
{
// WM:
	step_point();
	cpu_do_clear(false);
}

// -----------------------------------------------------------------------
void op_73_softint()
{
// WX:
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
	step_point();
}

// -----------------------------------------------------------------------
void op_73_giu()
{
// WM:
	step_point();
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_gil()
{
// WM:
	step_point();
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_lip()
{
	cpu_sp_rewind();
	cpu_ctx_restore(false);

	LOG(L_CPU, "Loaded process ctx @ 0x%04x: [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x]", ar-2, ic, r[0], SR_READ());

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
// WP:
	w = ac;
	step_point();
	ic = w;
}

// -----------------------------------------------------------------------
void op_74_lj()
{
// WW:
	w = ic;
	step_point();
	cpu_mem_write_1(q, ar, w);
// WE:
	step_point();
	at = ac + 1;
	w = at;
	ic = w;
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static inline void load_multiword(bool barnb, int start, int count)
{
// WX:
	int lg = start;
	int lk = count;
	step_point();
WR:
	cpu_mem_read_1(barnb, ar, &w);
	lk--;
	r[lg] = w;
	step_point();
	lg++;
	ar++;
	if (lk) goto WR;
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
	load_multiword(q, 5, 3);
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
	load_multiword(true, 5, 3);
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static inline void store_multiword(bool barnb, int start, int count)
{
// WX:
	int lg = start;
	int lk = count;
	step_point();
WW:
	w = r[lg];
	lk--;
	cpu_mem_write_1(barnb, ar, w);
	step_point();
	lg++;
	ar++;
	if (lk) goto WW;
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
	store_multiword(q, 5, 3);
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
	store_multiword(true, 5, 3);
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_77_mb()
{
// WR:
	cpu_mem_read_1(q, ar, &w);
	step_point();
	q =  w & 0b100000;
	bs = w & 0b010000;
	nb = w & 0b001111;
}

// -----------------------------------------------------------------------
void op_77_im()
{
// WR:
	cpu_mem_read_1(q, ar, &w);
	step_point();
	rm = (w >> 6) & 0b1111111111;
	int_update_mask(rm);
}

// -----------------------------------------------------------------------
void op_77_ki()
{
// WW:
	w = int_get_nchan();
	step_point();
	cpu_mem_write_1(q, ar, w);
}

// -----------------------------------------------------------------------
void op_77_fi()
{
// WR:
	cpu_mem_read_1(q, ar, &w);
	step_point();
	int_put_nchan(w);
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	cpu_ctx_restore(true);

	if (LOG_ENABLED) {
		log_update_process();
		log_intlevel_reset();
		log_cpu(L_OP, "SP: context @ 0x%04x", ac);
		if (LOG_WANTS(L_CRK5)) {
			log_handle_syscall_ret(L_CRK5, ic, SR_READ(), r[4]);
			log_log_process(L_CRK5);
		}
	}
}

// -----------------------------------------------------------------------
void op_77_md()
{
// WX:
	mc++;
	step_point();
}

// -----------------------------------------------------------------------
void op_77_rz()
{
// WW:
	w = 0;
	step_point();
	cpu_mem_write_1(q, ar, w);
}

// -----------------------------------------------------------------------
void op_77_ib()
{
// WR:
	cpu_mem_read_1(q, ar, &w);
	step_point();
	ac = w;
// W&:
	step_point();
	at = 1 + ac;
	p = (at == 0);
// WW:
	w = at;
	step_point();
	cpu_mem_write_1(q, ar, w);
}

// vim: tabstop=4 shiftwidth=4 autoindent
