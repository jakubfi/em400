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
#include <inttypes.h>

#include "log.h"

#include "cpu/flags.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"

// varmasks tell which bits contain variable instruction elements
enum em400_var_masks {
	VARMASK_ALL		= 0b1111111111111111,
	VARMASK_DABC	= 0b0000001111111111,
	VARMASK_DAC		= 0b0000001111000111,
	VARMASK_DBC		= 0b0000001000111111,
	VARMASK_BYTE	= 0b0000000011111111,
	VARMASK_BC		= 0b0000000000111111,
	VARMASK_DB1C	= 0b0000001000111100,
	VARMASK_A		= 0b0000000111000000,
	VARMASK_B		= 0b0000000000111000,
};

#define O(x) ((x) << 10)

// base MERA-400 instruction list
struct iset_instruction em400_ilist[] = {
	{ 0, VARMASK_ALL, { 0, 0, op_illegal, 0, 0 } }, // illegal instructions

	{ O(020), VARMASK_DABC, { 1, 0, op_lw, 0, 0 } },
	{ O(021), VARMASK_DABC, { 1, 0, op_tw, 0, 0 } },
	{ O(022), VARMASK_DABC, { 1, 0, op_ls, 0, 0 } },
	{ O(023), VARMASK_DABC, { 1, 0, op_ri, 0, 0 } },
	{ O(024), VARMASK_DABC, { 1, 0, op_rw, 0, 0 } },
	{ O(025), VARMASK_DABC, { 1, 0, op_pw, 0, 0 } },
	{ O(026), VARMASK_DABC, { 1, 0, op_rj, 0, 0 } },
	{ O(027), VARMASK_DABC, { 1, 0, op_is, 0, 0 } },
	{ O(030), VARMASK_DABC, { 1, 0, op_bb, 0, 0 } },
	{ O(031), VARMASK_DABC, { 1, 0, op_bm, 0, 0 } },
	{ O(032), VARMASK_DABC, { 1, 0, op_bs, 0, 0 } },
	{ O(033), VARMASK_DABC, { 1, 0, op_bc, 0, 0 } },
	{ O(034), VARMASK_DABC, { 1, 0, op_bn, 0, 0 } },
	{ O(035), VARMASK_DABC, { 1, 0, op_ou, 0, 0 } },
	{ O(036), VARMASK_DABC, { 1, 0, op_in, 0, 0 } },

	{ O(037)+0000, VARMASK_DBC, { 1, 0, op_37_ad, 0, 0 } },
	{ O(037)+0100, VARMASK_DBC, { 1, 0, op_37_sd, 0, 0 } },
	{ O(037)+0200, VARMASK_DBC, { 1, 0, op_37_mw, 0, 0 } },
	{ O(037)+0300, VARMASK_DBC, { 1, 0, op_37_dw, 0, 0 } },
	{ O(037)+0400, VARMASK_DBC, { 1, 0, op_37_af, 0, 0 } },
	{ O(037)+0500, VARMASK_DBC, { 1, 0, op_37_sf, 0, 0 } },
	{ O(037)+0600, VARMASK_DBC, { 1, 0, op_37_mf, 0, 0 } },
	{ O(037)+0700, VARMASK_DBC, { 1, 0, op_37_df, 0, 0 } },

	{ O(040), VARMASK_DABC, { 1, 0, op_aw, 0, 0 } },
	{ O(041), VARMASK_DABC, { 1, 0, op_ac, 0, 0 } },
	{ O(042), VARMASK_DABC, { 1, 0, op_sw, 0, 0 } },
	{ O(043), VARMASK_DABC, { 1, 0, op_cw, 0, 0 } },
	{ O(044), VARMASK_DABC, { 1, 0, op_or, 0, 0 } },
	{ O(045), VARMASK_DABC, { 1, 0, op_om, 0, 0 } },
	{ O(046), VARMASK_DABC, { 1, 0, op_nr, 0, 0 } },
	{ O(047), VARMASK_DABC, { 1, 0, op_nm, 0, 0 } },
	{ O(050), VARMASK_DABC, { 1, 0, op_er, 0, 0 } },
	{ O(051), VARMASK_DABC, { 1, 0, op_em, 0, 0 } },
	{ O(052), VARMASK_DABC, { 1, 0, op_xr, 0, 0 } },
	{ O(053), VARMASK_DABC, { 1, 0, op_xm, 0, 0 } },
	{ O(054), VARMASK_DABC, { 1, 0, op_cl, 0, 0 } },
	{ O(055), VARMASK_DABC, { 1, 0, op_lb, 0, 0 } },
	{ O(056), VARMASK_DABC, { 1, 0, op_rb, 0, 0 } },
	{ O(057), VARMASK_DABC, { 1, 0, op_cb, 0, 0 } },

	{ O(060), VARMASK_DABC, { 0, 1, op_awt, 0, 0 } },
	{ O(061), VARMASK_DABC, { 0, 1, op_trb, 0, 0 } },
	{ O(062), VARMASK_DABC, { 0, 1, op_irb, 0, 0 } },
	{ O(063), VARMASK_DABC, { 0, 1, op_drb, 0, 0 } },
	{ O(064), VARMASK_DABC, { 0, 1, op_cwt, 0, 0 } },
	{ O(065), VARMASK_DABC, { 0, 1, op_lwt, 0, 0 } },
	{ O(066), VARMASK_DABC, { 0, 1, op_lws, 0, 0 } },
	{ O(067), VARMASK_DABC, { 0, 1, op_rws, 0, 0 } },

	{ O(070)+0000, VARMASK_DBC, { 0, 1, op_70_jump, 0, 0 } },		// ujs
	{ O(070)+0100, VARMASK_DBC, { 0, 1, op_70_jump, FL_L, FL_L } },	// jls
	{ O(070)+0200, VARMASK_DBC, { 0, 1, op_70_jump, FL_E, FL_E } },	// jes
	{ O(070)+0300, VARMASK_DBC, { 0, 1, op_70_jump, FL_G, FL_G } },	// jgs
	{ O(070)+0400, VARMASK_DBC, { 0, 1, op_70_jvs,  FL_V, FL_V } },	// jvs
	{ O(070)+0500, VARMASK_DBC, { 0, 1, op_70_jump, FL_X, FL_X } },	// jxs
	{ O(070)+0600, VARMASK_DBC, { 0, 1, op_70_jump, FL_Y, FL_Y } },	// jys
	{ O(070)+0700, VARMASK_DBC, { 0, 1, op_70_jump, FL_C, FL_C } },	// jcs

	{ O(071)+00000, VARMASK_BYTE, { 0, 0, op_71_blc, 0, 0 } },
	{ O(071)+00400, VARMASK_BYTE, { 0, 0, op_71_exl, 0, 0 } },
	{ O(071)+01000, VARMASK_BYTE, { 0, 0, op_71_brc, 0, 0 } },
	{ O(071)+01400, VARMASK_BYTE, { 0, 0, op_71_nrf, 0, 0 } },

	{ O(072)+00000, VARMASK_A,   { 0, 0, op_72_ric, 0, 0 } },
	{ O(072)+00001, VARMASK_A,   { 0, 0, op_72_zlb, 0, 0 } },
	{ O(072)+00002, VARMASK_A,   { 0, 0, op_72_sxu, 0, 0 } },
	{ O(072)+00003, VARMASK_A,   { 0, 0, op_72_nga, 0, 0 } },
	{ O(072)+00004, VARMASK_A,   { 0, 0, op_72_slz, 0, 0 } },
	{ O(072)+00005, VARMASK_A,   { 0, 0, op_72_sly, 0, 0 } },
	{ O(072)+00006, VARMASK_A,   { 0, 0, op_72_slx, 0, 0 } },
	{ O(072)+00007, VARMASK_A,   { 0, 0, op_72_sry, 0, 0 } },
	{ O(072)+00010, VARMASK_A,   { 0, 0, op_72_ngl, 0, 0 } },
	{ O(072)+00011, VARMASK_A,   { 0, 0, op_72_rpc, 0, 0 } },
	{ O(072)+00020, VARMASK_DAC, { 0, 0, op_72_shc, 0, 0 } },
	{ O(072)+01000, VARMASK_A,   { 0, 0, op_72_rky, 0, 0 } },
	{ O(072)+01001, VARMASK_A,   { 0, 0, op_72_zrb, 0, 0 } },
	{ O(072)+01002, VARMASK_A,   { 0, 0, op_72_sxl, 0, 0 } },
	{ O(072)+01003, VARMASK_A,   { 0, 0, op_72_ngc, 0, 0 } },
	{ O(072)+01004, VARMASK_A,   { 0, 0, op_72_svz, 0, 0 } },
	{ O(072)+01005, VARMASK_A,   { 0, 0, op_72_svy, 0, 0 } },
	{ O(072)+01006, VARMASK_A,   { 0, 0, op_72_svx, 0, 0 } },
	{ O(072)+01007, VARMASK_A,   { 0, 0, op_72_srx, 0, 0 } },
	{ O(072)+01010, VARMASK_A,   { 0, 0, op_72_srz, 0, 0 } },
	{ O(072)+01011, VARMASK_A,   { 0, 0, op_72_lpc, 0, 0 } },

	{ O(073)+00000, VARMASK_DBC, { 0, 1, op_73_hlt, 0, 0 } },
	{ O(073)+00100, VARMASK_DBC, { 0, 0, op_73_mcl, 0, 0 } },
	{ O(073)+00200, VARMASK_DBC, { 0, 0, op_73_softint, 0, 0 } },
	{ O(073)+00300, VARMASK_BC,  { 0, 0, op_73_giu, 0, 0 } },
	{ O(073)+00400, VARMASK_DBC, { 0, 0, op_73_lip, 0, 0 } },
	{ O(073)+01300, VARMASK_BC,  { 0, 0, op_73_gil, 0, 0 } },
	{ O(073)+00500, VARMASK_DBC, { 0, 0, op_73_cron, 0, 0 } },

	{ O(074)+0000, VARMASK_DBC, { 1, 0, op_74_jump, 0, 0 } },		// uj
	{ O(074)+0100, VARMASK_DBC, { 1, 0, op_74_jump, FL_L, FL_L } },	// jl
	{ O(074)+0200, VARMASK_DBC, { 1, 0, op_74_jump, FL_E, FL_E } },	// je
	{ O(074)+0300, VARMASK_DBC, { 1, 0, op_74_jump, FL_G, FL_G } },	// jg
	{ O(074)+0400, VARMASK_DBC, { 1, 0, op_74_jump, FL_Z, FL_Z } },	// jz
	{ O(074)+0500, VARMASK_DBC, { 1, 0, op_74_jump, FL_M, FL_M } },	// jm
	{ O(074)+0600, VARMASK_DBC, { 1, 0, op_74_jump, FL_E, 0 } },	// jn
	{ O(074)+0700, VARMASK_DBC, { 1, 0, op_74_lj, 0, 0 } },

	{ O(075)+0000, VARMASK_DBC, { 1, 0, op_75_ld, 0, 0 } },
	{ O(075)+0100, VARMASK_DBC, { 1, 0, op_75_lf, 0, 0 } },
	{ O(075)+0200, VARMASK_DBC, { 1, 0, op_75_la, 0, 0 } },
	{ O(075)+0300, VARMASK_DBC, { 1, 0, op_75_ll, 0, 0 } },
	{ O(075)+0400, VARMASK_DBC, { 1, 0, op_75_td, 0, 0 } },
	{ O(075)+0500, VARMASK_DBC, { 1, 0, op_75_tf, 0, 0 } },
	{ O(075)+0600, VARMASK_DBC, { 1, 0, op_75_ta, 0, 0 } },
	{ O(075)+0700, VARMASK_DBC, { 1, 0, op_75_tl, 0, 0 } },

	{ O(076)+0000, VARMASK_DBC, { 1, 0, op_76_rd, 0, 0 } },
	{ O(076)+0100, VARMASK_DBC, { 1, 0, op_76_rf, 0, 0 } },
	{ O(076)+0200, VARMASK_DBC, { 1, 0, op_76_ra, 0, 0 } },
	{ O(076)+0300, VARMASK_DBC, { 1, 0, op_76_rl, 0, 0 } },
	{ O(076)+0400, VARMASK_DBC, { 1, 0, op_76_pd, 0, 0 } },
	{ O(076)+0500, VARMASK_DBC, { 1, 0, op_76_pf, 0, 0 } },
	{ O(076)+0600, VARMASK_DBC, { 1, 0, op_76_pa, 0, 0 } },
	{ O(076)+0700, VARMASK_DBC, { 1, 0, op_76_pl, 0, 0 } },

	{ O(077)+0000, VARMASK_DBC, { 1, 0, op_77_mb, 0, 0 } },
	{ O(077)+0100, VARMASK_DBC, { 1, 0, op_77_im, 0, 0 } },
	{ O(077)+0200, VARMASK_DBC, { 1, 0, op_77_ki, 0, 0 } },
	{ O(077)+0300, VARMASK_DBC, { 1, 0, op_77_fi, 0, 0 } },
	{ O(077)+0400, VARMASK_DBC, { 1, 0, op_77_sp, 0, 0 } },
	{ O(077)+0500, VARMASK_DBC, { 1, 0, op_77_md, 0, 0 } },
	{ O(077)+0600, VARMASK_DBC, { 1, 0, op_77_rz, 0, 0 } },
	{ O(077)+0700, VARMASK_DBC, { 1, 0, op_77_ib, 0, 0 } },

	{ 0, 0, { 0, 0, NULL } }
};

// -----------------------------------------------------------------------
static int iset_register_op(struct iset_opcode **op_tab, struct iset_instruction *instr)
{
	int offsets[16];

	// store 1's positions in mask, count 1's
	int one_count = 0;
	for (int i=0 ; i<16 ; i++) {
		if (instr->var_mask & (1<<i)) {
			offsets[one_count] = i;
			one_count++;
		}
	}

	int max = (1 << one_count) - 1;

	// iterate over all variants (as indicated by the mask)
	for (int i=0 ; i<=max ; i++) {
		uint16_t result = 0;
		// shift 1's into positions
		for (int pos=one_count-1 ; pos>=0 ; pos--) {
			result |= ((i >> pos) & 1) << offsets[pos];
		}

		struct iset_opcode *op = op_tab[instr->opcode | result];

		// sanity check: we don't want to overwrite non-illegal registered ops
		if ((op) && (op->fun != op_illegal)) {
			return log_err("Trying to overwrite non-illegal registered op 0x%04x.", (instr->opcode | result));
		}
		// register the op
		op_tab[instr->opcode | result] = &instr->op;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int iset_build(struct iset_opcode **op_tab)
{
	int res;

	struct iset_instruction *instr = em400_ilist;
	while (instr->var_mask) {
		res = iset_register_op(op_tab, instr);
		if (res != E_OK) {
			return log_err("Failed to register op 0x%04x.", instr->opcode);
		}
		instr++;
	}
	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
