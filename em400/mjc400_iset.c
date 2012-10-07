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
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "mjc400_iset.h"
#include "mjc400_instr.h"
#include "mjc400_dasm.h"
#include "mjc400_trans.h"

struct mjc400_opdef mjc400_iset[] = {
	{ 000, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 001, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 002, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 003, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 004, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 005, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 006, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 007, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 010, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 011, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 012, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 013, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 014, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 015, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 016, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 017, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	
	{ 020, "LW", false, true, mjc400_op_lw, NULL, NULL, D_2ARGN, NULL },
	{ 021, "TW", false, true, mjc400_op_tw, NULL, NULL, D_2ARGN, NULL },
	{ 022, "LS", false, true, mjc400_op_ls, NULL, NULL, D_2ARGN, NULL },
	{ 023, "RI", false, true, mjc400_op_ri, NULL, NULL, D_2ARGN, NULL },
	{ 024, "RW", false, true, mjc400_op_rw, NULL, NULL, D_2ARGN, NULL },
	{ 025, "PW", false, true, mjc400_op_pw, NULL, NULL, D_2ARGN, NULL },
	{ 026, "RJ", false, true, mjc400_op_rj, NULL, NULL, D_2ARGN, NULL },
	{ 027, "IS", false, true, mjc400_op_is, NULL, NULL, D_2ARGN, NULL },
	{ 030, "BB", false, true, mjc400_op_bb, NULL, NULL, D_2ARGN, NULL },
	{ 031, "BM", false, true, mjc400_op_bm, NULL, NULL, D_2ARGN, NULL },
	{ 032, "BS", false, true, mjc400_op_bs, NULL, NULL, D_2ARGN, NULL },
	{ 033, "BC", false, true, mjc400_op_bc, NULL, NULL, D_2ARGN, NULL },
	{ 034, "BN", false, true, mjc400_op_bn, NULL, NULL, D_2ARGN, NULL },
	{ 035, "OU", false, true, mjc400_op_ou, NULL, NULL, D_2ARGN, NULL },
	{ 036, "IN", false, true, mjc400_op_in, NULL, NULL, D_2ARGN, NULL },

	{ 037, NULL, false, false, mjc400_op_37, mjc400_dasm_e37, mjc400_iset_37, NULL },

	{ 040, "AW", false, true, mjc400_op_aw, NULL, NULL, D_2ARGN, NULL },
	{ 041, "AC", false, true, mjc400_op_ac, NULL, NULL, D_2ARGN, NULL },
	{ 042, "SW", false, true, mjc400_op_sw, NULL, NULL, D_2ARGN, NULL },
	{ 043, "CW", false, true, mjc400_op_cw, NULL, NULL, D_2ARGN, NULL },
	{ 044, "OR", false, true, mjc400_op_or, NULL, NULL, D_2ARGN, NULL },
	{ 045, "OM", false, true, mjc400_op_om, NULL, NULL, D_2ARGN, NULL },
	{ 046, "NR", false, true, mjc400_op_nr, NULL, NULL, D_2ARGN, NULL },
	{ 047, "NM", false, true, mjc400_op_nm, NULL, NULL, D_2ARGN, NULL },
	{ 050, "ER", false, true, mjc400_op_er, NULL, NULL, D_2ARGN, NULL },
	{ 051, "EM", false, true, mjc400_op_em, NULL, NULL, D_2ARGN, NULL },
	{ 052, "XR", false, true, mjc400_op_xr, NULL, NULL, D_2ARGN, NULL },
	{ 053, "XM", false, true, mjc400_op_xm, NULL, NULL, D_2ARGN, NULL },
	{ 054, "CL", false, true, mjc400_op_cl, NULL, NULL, D_2ARGN, NULL },
	{ 055, "LB", false, true, mjc400_op_lb, NULL, NULL, D_2ARGN, NULL },
	{ 056, "RB", false, true, mjc400_op_rb, NULL, NULL, D_2ARGN, NULL },
	{ 057, "CB", false, true, mjc400_op_cb, NULL, NULL, D_2ARGN, NULL },

	{ 060, "AWT", false, false, mjc400_op_awt, NULL, NULL, D_KA1, NULL },
	{ 061, "TRB", false, false, mjc400_op_trb, NULL, NULL, D_KA1, NULL },
	{ 062, "IRB", false, false, mjc400_op_irb, NULL, NULL, D_KA1, NULL },
	{ 063, "DRB", false, false, mjc400_op_drb, NULL, NULL, D_KA1, NULL },
	{ 064, "CWT", false, false, mjc400_op_cwt, NULL, NULL, D_KA1, NULL },
	{ 065, "LWT", false, false, mjc400_op_lwt, NULL, NULL, D_KA1, NULL },
	{ 066, "LWS", false, false, mjc400_op_lws, NULL, NULL, D_KA1, NULL },
	{ 067, "RWS", false, false, mjc400_op_rws, NULL, NULL, D_KA1, NULL },

	{ 070, NULL, false, false, mjc400_op_70, mjc400_dasm_e70, mjc400_iset_70, NULL },
	{ 071, NULL, false, false, mjc400_op_71, mjc400_dasm_e71, mjc400_iset_71, NULL },
	{ 072, NULL, false, false, mjc400_op_72, mjc400_dasm_e72, mjc400_iset_72, NULL },
	{ 073, NULL, false, false, mjc400_op_73, mjc400_dasm_e73, mjc400_iset_73, NULL },
	{ 074, NULL, false, false, mjc400_op_74, mjc400_dasm_e74, mjc400_iset_74, NULL },
	{ 075, NULL, false, false, mjc400_op_75, mjc400_dasm_e75, mjc400_iset_75, NULL },
	{ 076, NULL, false, false, mjc400_op_76, mjc400_dasm_e76, mjc400_iset_76, NULL },
	{ 077, NULL, false, false, mjc400_op_77, mjc400_dasm_e77, mjc400_iset_77, NULL }
};

struct mjc400_opdef mjc400_iset_37[] = {
	{ 0, "AD", false, true, mjc400_op_37_ad, NULL, NULL, D_FD, NULL },
	{ 1, "SD", false, true, mjc400_op_37_sd, NULL, NULL, D_FD, NULL },
	{ 2, "MW", false, true, mjc400_op_37_mw, NULL, NULL, D_FD, NULL },
	{ 3, "DW", false, true, mjc400_op_37_dw, NULL, NULL, D_FD, NULL },
	{ 4, "AF", false, true, mjc400_op_37_af, NULL, NULL, D_FD, NULL },
	{ 5, "SF", false, true, mjc400_op_37_sf, NULL, NULL, D_FD, NULL },
	{ 6, "MF", false, true, mjc400_op_37_mf, NULL, NULL, D_FD, NULL },
	{ 7, "DF", false, true, mjc400_op_37_df, NULL, NULL, D_FD, NULL }
};

struct mjc400_opdef mjc400_iset_70[] = {
	{ 0, "UJS", false, false, mjc400_op_70_ujs, NULL, NULL, D_JS, NULL },
	{ 1, "JLS", false, false, mjc400_op_70_jls, NULL, NULL, D_JS, NULL },
	{ 2, "JES", false, false, mjc400_op_70_jes, NULL, NULL, D_JS, NULL },
	{ 3, "JGS", false, false, mjc400_op_70_jgs, NULL, NULL, D_JS, NULL },
	{ 4, "JVS", false, false, mjc400_op_70_jvs, NULL, NULL, D_JS, NULL },
	{ 5, "JXS", false, false, mjc400_op_70_jxs, NULL, NULL, D_JS, NULL },
	{ 6, "JYS", false, false, mjc400_op_70_jys, NULL, NULL, D_JS, NULL },
	{ 7, "JCS", false, false, mjc400_op_70_jcs, NULL, NULL, D_JS, NULL }
};

struct mjc400_opdef mjc400_iset_71[] = {
	{ 0, "BLC", false, false, mjc400_op_71_blc, NULL, NULL, D_KA2, NULL },
	{ 1, "EXL", false, false, mjc400_op_71_exl, NULL, NULL, D_KA2, NULL },
	{ 2, "BRC", false, false, mjc400_op_71_brc, NULL, NULL, D_KA2, NULL },
	{ 3, "NRF", false, false, mjc400_op_71_nrf, NULL, NULL, D_KA2, NULL }
};

struct mjc400_opdef mjc400_iset_72[] = {
	{ 0b000000, "RIC", false, false, mjc400_op_72_ric, NULL, NULL, D_C, NULL },
	{ 0b000001, "ZLB", false, false, mjc400_op_72_zlb, NULL, NULL, D_C, NULL },
	{ 0b000010, "SXU", false, false, mjc400_op_72_sxu, NULL, NULL, D_C, NULL },
	{ 0b000011, "NGA", false, false, mjc400_op_72_nga, NULL, NULL, D_C, NULL },
	{ 0b000100, "SLZ", false, false, mjc400_op_72_slz, NULL, NULL, D_C, NULL },
	{ 0b000101, "SLY", false, false, mjc400_op_72_sly, NULL, NULL, D_C, NULL },
	{ 0b000110, "SLX", false, false, mjc400_op_72_slx, NULL, NULL, D_C, NULL },
	{ 0b000111, "SRY", false, false, mjc400_op_72_sry, NULL, NULL, D_C, NULL },
	{ 0b001000, "NGL", false, false, mjc400_op_72_ngl, NULL, NULL, D_C, NULL },
	{ 0b001001, "RPC", false, false, mjc400_op_72_rpc, NULL, NULL, D_C, NULL },
	{ 0b001010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010000, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010001, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010010, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010011, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010100, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010101, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010110, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b010111, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b011000, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011001, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b100000, "RKY", false, false, mjc400_op_72_rky, NULL, NULL, D_C, NULL },
	{ 0b100001, "ZRB", false, false, mjc400_op_72_zrb, NULL, NULL, D_C, NULL },
	{ 0b100010, "SXL", false, false, mjc400_op_72_sxl, NULL, NULL, D_C, NULL },
	{ 0b100011, "NGC", false, false, mjc400_op_72_ngc, NULL, NULL, D_C, NULL },
	{ 0b100100, "SVZ", false, false, mjc400_op_72_svz, NULL, NULL, D_C, NULL },
	{ 0b100101, "SVY", false, false, mjc400_op_72_svy, NULL, NULL, D_C, NULL },
	{ 0b100110, "SVX", false, false, mjc400_op_72_svx, NULL, NULL, D_C, NULL },
	{ 0b100111, "SRX", false, false, mjc400_op_72_srx, NULL, NULL, D_C, NULL },
	{ 0b101000, "SRZ", false, false, mjc400_op_72_srz, NULL, NULL, D_C, NULL },
	{ 0b101001, "LPC", false, false, mjc400_op_72_lpc, NULL, NULL, D_C, NULL },
	{ 0b101010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110001, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110000, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110010, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110011, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110100, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110101, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110110, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b110111, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, NULL },
	{ 0b111000, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111001, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL }
};

struct mjc400_opdef mjc400_iset_73[] = {
	{ 0b000000, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b000001, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b000010, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b000011, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b000100, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b000101, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b000110, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b000111, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b001000, "CIT", true, false, mjc400_op_73_cit, NULL, NULL, D_S, NULL },
	{ 0b001001, "SIL", true, false, mjc400_op_73_sil, NULL, NULL, D_S, NULL },
	{ 0b001010, "SIU", true, false, mjc400_op_73_siu, NULL, NULL, D_S, NULL },
	{ 0b001011, "SIT", true, false, mjc400_op_73_sit, NULL, NULL, D_S, NULL },
	{ 0b001100, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, NULL },
	{ 0b001101, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, NULL },
	{ 0b001110, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, NULL },
	{ 0b001111, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, NULL },
	{ 0b010000, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b010001, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b010010, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b010011, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b010100, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010101, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010110, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010111, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011000, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011001, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011010, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011011, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011100, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011101, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011110, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011111, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b100000, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b100001, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b100010, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b100011, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, NULL },
	{ 0b100100, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b100101, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b100110, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b100111, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, NULL },
	{ 0b101000, "CIT", true, false, mjc400_op_73_cit, NULL, NULL, D_S, NULL },
	{ 0b101001, "SIL", true, false, mjc400_op_73_sil, NULL, NULL, D_S, NULL },
	{ 0b101010, "SIU", true, false, mjc400_op_73_siu, NULL, NULL, D_S, NULL },
	{ 0b101011, "SIT", true, false, mjc400_op_73_sit, NULL, NULL, D_S, NULL },
	{ 0b101100, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, NULL },
	{ 0b101101, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, NULL },
	{ 0b101110, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, NULL },
	{ 0b101111, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, NULL },
	{ 0b110001, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b110000, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b110010, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b110011, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, NULL },
	{ 0b110100, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110101, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110110, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110111, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111000, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111001, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111010, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111011, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111100, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111101, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111110, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111111, NULL, false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL }
};

struct mjc400_opdef mjc400_iset_74[] = {
	{ 0, "UJ", false, true, mjc400_op_74_uj, NULL, NULL, D_J, NULL },
	{ 1, "JL", false, true, mjc400_op_74_jl, NULL, NULL, D_J, NULL },
	{ 2, "JE", false, true, mjc400_op_74_je, NULL, NULL, D_J, NULL },
	{ 3, "JG", false, true, mjc400_op_74_jg, NULL, NULL, D_J, NULL },
	{ 4, "JZ", false, true, mjc400_op_74_jz, NULL, NULL, D_J, NULL },
	{ 5, "JM", false, true, mjc400_op_74_jm, NULL, NULL, D_J, NULL },
	{ 6, "JN", false, true, mjc400_op_74_jn, NULL, NULL, D_J, NULL },
	{ 7, "LJ", false, true, mjc400_op_74_lj, NULL, NULL, D_J, NULL }
};

struct mjc400_opdef mjc400_iset_75[] = {
	{ 0, "LD", false, true, mjc400_op_75_ld, NULL, NULL, D_L, NULL },
	{ 1, "LF", false, true, mjc400_op_75_lf, NULL, NULL, D_L, NULL },
	{ 2, "LA", false, true, mjc400_op_75_la, NULL, NULL, D_L, NULL },
	{ 3, "LL", false, true, mjc400_op_75_ll, NULL, NULL, D_L, NULL },
	{ 4, "TD", false, true, mjc400_op_75_td, NULL, NULL, D_L, NULL },
	{ 5, "TF", false, true, mjc400_op_75_tf, NULL, NULL, D_L, NULL },
	{ 6, "TA", false, true, mjc400_op_75_ta, NULL, NULL, D_L, NULL },
	{ 7, "TL", false, true, mjc400_op_75_tl, NULL, NULL, D_L, NULL }
};

struct mjc400_opdef mjc400_iset_76[] = {
	{ 0, "RD", false, true, mjc400_op_76_rd, NULL, NULL, D_G, NULL },
	{ 1, "RF", false, true, mjc400_op_76_rf, NULL, NULL, D_G, NULL },
	{ 2, "RA", false, true, mjc400_op_76_ra, NULL, NULL, D_G, NULL },
	{ 3, "RL", false, true, mjc400_op_76_rl, NULL, NULL, D_G, NULL },
	{ 4, "PD", false, true, mjc400_op_76_pd, NULL, NULL, D_G, NULL },
	{ 5, "PF", false, true, mjc400_op_76_pf, NULL, NULL, D_G, NULL },
	{ 6, "PA", false, true, mjc400_op_76_pa, NULL, NULL, D_G, NULL },
	{ 7, "PL", false, true, mjc400_op_76_pl, NULL, NULL, D_G, NULL }
};

struct mjc400_opdef mjc400_iset_77[] = {
	{ 0, "MB", true,  true, mjc400_op_77_mb, NULL, NULL, D_BN, NULL },
	{ 1, "IM", true,  true, mjc400_op_77_im, NULL, NULL, D_BN, NULL },
	{ 2, "KI", true,  true, mjc400_op_77_ki, NULL, NULL, D_BN, NULL },
	{ 3, "FI", true,  true, mjc400_op_77_fi, NULL, NULL, D_BN, NULL },
	{ 4, "SP", true,  true, mjc400_op_77_sp, NULL, NULL, D_BN, NULL },
	{ 5, "MD", false, true, mjc400_op_77_md, NULL, NULL, D_BN, NULL },
	{ 6, "RZ", false, true, mjc400_op_77_rz, NULL, NULL, D_BN, NULL },
	{ 7, "IB", false, true, mjc400_op_77_ib, NULL, NULL, D_BN, NULL }
};

// vim: tabstop=4
