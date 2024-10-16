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
#include <inttypes.h>

#include "log.h"

#include "cpu/cpu.h"
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

// Instruction execution times are based on actual measurements
// taken on MERA-400 with Computex DRAM memory module
// with CPU strob times set according to service guidelines.
// That means:
//  - CPU is on the slower, "safer" side
//  - memory is about 170ns faster than standard Elwro magnetic core memory

// MERA-400 instruction list
struct iset_instruction em400_ilist[] = {
	{ 0, VARMASK_ALL, { OP_FL_ILLEGAL, NULL, 0, 0, TIME_P } },// // illegal instructions

	{ O(020), VARMASK_DABC, { OP_FL_ARG_NORM, op_lw, 0, 0, 2510 } },
	{ O(021), VARMASK_DABC, { OP_FL_ARG_NORM, op_tw, 0, 0, 3740 } },
	{ O(022), VARMASK_DABC, { OP_FL_ARG_NORM, op_ls, 0, 0, 3940 } },
	{ O(023), VARMASK_DABC, { OP_FL_ARG_NORM, op_ri, 0, 0, 4295 } },
	{ O(024), VARMASK_DABC, { OP_FL_ARG_NORM, op_rw, 0, 0, 3500 } },
	{ O(025), VARMASK_DABC, { OP_FL_ARG_NORM, op_pw, 0, 0, 3500 } },
	{ O(026), VARMASK_DABC, { OP_FL_ARG_NORM, op_rj, 0, 0, 2820 } },
	{ O(027), VARMASK_DABC, { OP_FL_ARG_NORM, op_is, 0, 0, 4220 } }, // TODO: conditions
	{ O(030), VARMASK_DABC, { OP_FL_ARG_NORM, op_bb, 0, 0, 2660 } },
	{ O(031), VARMASK_DABC, { OP_FL_ARG_NORM, op_bm, 0, 0, 4220 } },
	{ O(032), VARMASK_DABC, { OP_FL_ARG_NORM, op_bs, 0, 0, 3145 } },
	{ O(033), VARMASK_DABC, { OP_FL_ARG_NORM, op_bc, 0, 0, 2660 } },
	{ O(034), VARMASK_DABC, { OP_FL_ARG_NORM, op_bn, 0, 0, 2660 } },
	{ O(035), VARMASK_DABC, { OP_FL_IO | OP_FL_ARG_NORM, op_ou, 0, 0, 2000 } }, // TODO
	{ O(036), VARMASK_DABC, { OP_FL_IO | OP_FL_ARG_NORM, op_in, 0, 0, 2000 } }, // TODO

	{ O(037)+0000, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_ad, 0, 0, 8780 } },
	{ O(037)+0100, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_sd, 0, 0, 8760 } },
	{ O(037)+0200, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_mw, 0, 0, (3500+11900)/2 } }, // TODO
	{ O(037)+0300, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_dw, 0, 0, (4900+15300)/2 } }, // TODO
	{ O(037)+0400, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_af, 0, 0, (5400+19000)/2 } }, // TODO
	{ O(037)+0500, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_sf, 0, 0, (5400+19000)/2 } }, // TODO
	{ O(037)+0600, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_mf, 0, 0, (6800+29000)/2 } }, // TODO
	{ O(037)+0700, VARMASK_DBC, { OP_FL_ARG_NORM, op_37_df, 0, 0, (6800+30000)/2 } }, // TODO

	{ O(040), VARMASK_DABC, { OP_FL_ARG_NORM, op_aw, 0, 0, 2660 } },
	{ O(041), VARMASK_DABC, { OP_FL_ARG_NORM, op_ac, 0, 0, 2660 } },
	{ O(042), VARMASK_DABC, { OP_FL_ARG_NORM, op_sw, 0, 0, 2660 } },
	{ O(043), VARMASK_DABC, { OP_FL_ARG_NORM, op_cw, 0, 0, 2660 } },
	{ O(044), VARMASK_DABC, { OP_FL_ARG_NORM, op_or, 0, 0, 2660 } },
	{ O(045), VARMASK_DABC, { OP_FL_ARG_NORM, op_om, 0, 0, 5260 } },
	{ O(046), VARMASK_DABC, { OP_FL_ARG_NORM, op_nr, 0, 0, 2660 } },
	{ O(047), VARMASK_DABC, { OP_FL_ARG_NORM, op_nm, 0, 0, 5260 } },
	{ O(050), VARMASK_DABC, { OP_FL_ARG_NORM, op_er, 0, 0, 2660 } },
	{ O(051), VARMASK_DABC, { OP_FL_ARG_NORM, op_em, 0, 0, 5255 } },
	{ O(052), VARMASK_DABC, { OP_FL_ARG_NORM, op_xr, 0, 0, 2660 } },
	{ O(053), VARMASK_DABC, { OP_FL_ARG_NORM, op_xm, 0, 0, 5260 } },
	{ O(054), VARMASK_DABC, { OP_FL_ARG_NORM, op_cl, 0, 0, 2660 } },
	{ O(055), VARMASK_DABC, { OP_FL_ARG_NORM, op_lb, 0, 0, 5480 } }, // TODO: left/right byte
	{ O(056), VARMASK_DABC, { OP_FL_ARG_NORM, op_rb, 0, 0, 6380 } },
	{ O(057), VARMASK_DABC, { OP_FL_ARG_NORM, op_cb, 0, 0, 5480 } }, // TODO: left/right byte,

	{ O(060), VARMASK_DABC, { OP_FL_ARG_SHORT, op_awt, 0, 0, 2660 } }, // TODO: negative argument
	{ O(061), VARMASK_DABC, { OP_FL_ARG_SHORT, op_trb, 0, 0, 2660 } },
	{ O(062), VARMASK_DABC, { OP_FL_ARG_SHORT, op_irb, 0, 0, 2660 } }, // TODO: conditions
	{ O(063), VARMASK_DABC, { OP_FL_ARG_SHORT, op_drb, 0, 0, 2660 } }, // TODO: conditions
	{ O(064), VARMASK_DABC, { OP_FL_ARG_SHORT, op_cwt, 0, 0, 2660 } },
	{ O(065), VARMASK_DABC, { OP_FL_ARG_SHORT, op_lwt, 0, 0, 2510 } },
	{ O(066), VARMASK_DABC, { OP_FL_ARG_SHORT, op_lws, 0, 0, 4210 } },
	{ O(067), VARMASK_DABC, { OP_FL_ARG_SHORT, op_rws, 0, 0, 3980 } },

	{ O(070)+0000, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, 0, 0, 2660 } },			// ujs
	{ O(070)+0100, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_L, FL_L, 2660 } },	// jls
	{ O(070)+0200, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_E, FL_E, 2660 } },	// jes
	{ O(070)+0300, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_G, FL_G, 2660 } },	// jgs
	{ O(070)+0400, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jvs,  FL_V, FL_V, 2660 } },	// jvs
	{ O(070)+0500, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_X, FL_X, 2660 } },	// jxs
	{ O(070)+0600, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_Y, FL_Y, 2660 } },	// jys
	{ O(070)+0700, VARMASK_DBC, { OP_FL_ARG_SHORT, op_70_jump, FL_C, FL_C, 2660 } },	// jcs

	{ O(071)+00000, VARMASK_BYTE, { OP_FL_ARG_BYTE, op_71_blc, 0, 0, 2660 } },
	{ O(071)+00400, VARMASK_BYTE, { OP_FL_ARG_BYTE, op_71_exl, 0, 0, 10000 } }, // TODO
	{ O(071)+01000, VARMASK_BYTE, { OP_FL_ARG_BYTE, op_71_brc, 0, 0, 2660 } },
	{ O(071)+01400, VARMASK_BYTE, { OP_FL_ARG_BYTE, op_71_nrf, 0, 0, (3800+15800)/2 } }, // TODO

	{ O(072)+00000, VARMASK_A,   { OP_FL_NONE, op_72_ric, 0, 0, 2180 } },
	{ O(072)+00001, VARMASK_A,   { OP_FL_NONE, op_72_zlb, 0, 0, 2350 } },
	{ O(072)+00002, VARMASK_A,   { OP_FL_NONE, op_72_sxu, 0, 0, 2180 } },
	{ O(072)+00003, VARMASK_A,   { OP_FL_NONE, op_72_nga, 0, 0, 2660 } },
	{ O(072)+00004, VARMASK_A,   { OP_FL_NONE, op_72_slz, 0, 0, 2355 } },
	{ O(072)+00005, VARMASK_A,   { OP_FL_NONE, op_72_sly, 0, 0, 2350 } },
	{ O(072)+00006, VARMASK_A,   { OP_FL_NONE, op_72_slx, 0, 0, 2350 } },
	{ O(072)+00007, VARMASK_A,   { OP_FL_NONE, op_72_sry, 0, 0, 2840 } },
	{ O(072)+00010, VARMASK_A,   { OP_FL_NONE, op_72_ngl, 0, 0, 2350 } },
	{ O(072)+00011, VARMASK_A,   { OP_FL_NONE, op_72_rpc, 0, 0, 2180 } },
	{ O(072)+00020, VARMASK_DAC, { OP_FL_NONE, op_72_shc, 0, 0, 2835-TIME_SHIFT } },
	{ O(072)+01000, VARMASK_A,   { OP_FL_NONE, op_72_rky, 0, 0, 2180 } },
	{ O(072)+01001, VARMASK_A,   { OP_FL_NONE, op_72_zrb, 0, 0, 2350 } },
	{ O(072)+01002, VARMASK_A,   { OP_FL_NONE, op_72_sxl, 0, 0, 2180 } },
	{ O(072)+01003, VARMASK_A,   { OP_FL_NONE, op_72_ngc, 0, 0, 2660 } },
	{ O(072)+01004, VARMASK_A,   { OP_FL_NONE, op_72_svz, 0, 0, 2350 } },
	{ O(072)+01005, VARMASK_A,   { OP_FL_NONE, op_72_svy, 0, 0, 2350 } },
	{ O(072)+01006, VARMASK_A,   { OP_FL_NONE, op_72_svx, 0, 0, 2350 } },
	{ O(072)+01007, VARMASK_A,   { OP_FL_NONE, op_72_srx, 0, 0, 2830 } },
	{ O(072)+01010, VARMASK_A,   { OP_FL_NONE, op_72_srz, 0, 0, 2830 } },
	{ O(072)+01011, VARMASK_A,   { OP_FL_NONE, op_72_lpc, 0, 0, 2180 } },

	{ O(073)+00000, VARMASK_DBC, { OP_FL_USR_ILLEGAL | OP_FL_ARG_SHORT, op_73_hlt, 0, 0, 1900 } }, // until CPU halts
	{ O(073)+00100, VARMASK_DBC, { OP_FL_USR_ILLEGAL, op_73_mcl, 0, 0, TIME_NOANS_IF } }, // + P1 time, but more-or-less anyway
	{ O(073)+00200, VARMASK_DBC, { OP_FL_USR_ILLEGAL, op_73_softint, 0, 0, 2190 } },
	{ O(073)+00300, VARMASK_BC,  { OP_FL_USR_ILLEGAL, op_73_giu, 0, 0, 2190 } }, // TODO, guess
	{ O(073)+00400, VARMASK_DBC, { OP_FL_USR_ILLEGAL, op_73_lip, 0, 0, 9620 } },
	{ O(073)+01300, VARMASK_BC,  { OP_FL_USR_ILLEGAL, op_73_gil, 0, 0, 2190 } }, // TODO, guess
	{ O(073)+00500, VARMASK_DBC, { OP_FL_USR_ILLEGAL, op_73_cron, 0, 0, 2190 } }, // TODO, guess

	{ O(074)+0000, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, 0, 0, 2505 } },		// uj
	{ O(074)+0100, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_L, FL_L, 2505 } },	// jl
	{ O(074)+0200, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_E, FL_E, 2505 } },	// je
	{ O(074)+0300, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_G, FL_G, 2505 } },	// jg
	{ O(074)+0400, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_Z, FL_Z, 2505 } },	// jz
	{ O(074)+0500, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_M, FL_M, 2505 } },	// jm
	{ O(074)+0600, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_jump, FL_E, 0, 2505 } },	// jn
	{ O(074)+0700, VARMASK_DBC, { OP_FL_ARG_NORM, op_74_lj, 0, 0, 3980 } },

	{ O(075)+0000, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_ld, 0, 0, 5600 } },
	{ O(075)+0100, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_lf, 0, 0, 7160 } },
	{ O(075)+0200, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_la, 0, 0, 13360 } },
	{ O(075)+0300, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_ll, 0, 0, 7160 } },
	{ O(075)+0400, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_td, 0, 0, 5600 } },
	{ O(075)+0500, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_tf, 0, 0, 7160 } },
	{ O(075)+0600, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_ta, 0, 0, 13360 } },
	{ O(075)+0700, VARMASK_DBC, { OP_FL_ARG_NORM, op_75_tl, 0, 0, 7160 } },

	{ O(076)+0000, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_rd, 0, 0, 5140 } },
	{ O(076)+0100, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_rf, 0, 0, 6460 } },
	{ O(076)+0200, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_ra, 0, 0, 11720 } },
	{ O(076)+0300, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_rl, 0, 0, 6460 } },
	{ O(076)+0400, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_pd, 0, 0, 5140 } },
	{ O(076)+0500, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_pf, 0, 0, 6460 } },
	{ O(076)+0600, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_pa, 0, 0, 11720 } },
	{ O(076)+0700, VARMASK_DBC, { OP_FL_ARG_NORM, op_76_pl, 0, 0, 6460 } },

	{ O(077)+0000, VARMASK_DBC, { OP_FL_ARG_NORM | OP_FL_USR_ILLEGAL, op_77_mb, 0, 0, 3720 } },
	{ O(077)+0100, VARMASK_DBC, { OP_FL_ARG_NORM | OP_FL_USR_ILLEGAL, op_77_im, 0, 0, 3720 } },
	{ O(077)+0200, VARMASK_DBC, { OP_FL_ARG_NORM | OP_FL_USR_ILLEGAL, op_77_ki, 0, 0, 3500 } },
	{ O(077)+0300, VARMASK_DBC, { OP_FL_ARG_NORM | OP_FL_USR_ILLEGAL, op_77_fi, 0, 0, 3740 } },
	{ O(077)+0400, VARMASK_DBC, { OP_FL_ARG_NORM | OP_FL_USR_ILLEGAL, op_77_sp, 0, 0, 6940 } },
	{ O(077)+0500, VARMASK_DBC, { OP_FL_ARG_NORM, op_77_md, 0, 0, 2510 } },
	{ O(077)+0600, VARMASK_DBC, { OP_FL_ARG_NORM, op_77_rz, 0, 0, 3500 } },
	{ O(077)+0700, VARMASK_DBC, { OP_FL_ARG_NORM, op_77_ib, 0, 0, 5420 } },

	{ 0, 0, { OP_FL_NONE, NULL, 0, 0 } }
};

// -----------------------------------------------------------------------
static int iset_register_op(struct iset_opcode **op_tab, struct iset_instruction *instr)
{
	int offsets[16];

	// store 1's positions in mask, count 1's
	int one_count = 0;
	for (int i=0 ; i<16 ; i++) {
		if (instr->var_mask & (1 << i)) {
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
		if ((op) && (op->fun != NULL)) {
			return LOGERR("Trying to overwrite non-illegal registered op 0x%04x.", (instr->opcode | result));
		}
		// register the op
		op_tab[instr->opcode | result] = &instr->op;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int iset_build(struct iset_opcode **op_tab, int cpu_user_io_illegal)
{
	struct iset_instruction *instr = em400_ilist;
	while (instr->var_mask) {
		// set IN/OU legalness in user mode
		if ((instr->op.flags & OP_FL_IO)) {
			if (cpu_user_io_illegal) {
				instr->op.flags |= OP_FL_USR_ILLEGAL;
			} else {
				instr->op.flags &= ~OP_FL_USR_ILLEGAL;
			}
		}
		if (iset_register_op(op_tab, instr) != E_OK) {
			return LOGERR("Failed to register op 0x%04x.", instr->opcode);
		}
		instr++;
	}
	return E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
