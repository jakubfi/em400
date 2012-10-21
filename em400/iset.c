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

#include "iset.h"
#include "instructions.h"
#include "dasm.h"
#include "dasm_formats.h"

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
	
	{ 020, "LW", false, true, mjc400_op_lw, NULL, NULL, D_2ARGN, T_LW },
	{ 021, "TW", false, true, mjc400_op_tw, NULL, NULL, D_2ARGN, T_TW },
	{ 022, "LS", false, true, mjc400_op_ls, NULL, NULL, D_2ARGN, T_LS },
	{ 023, "RI", false, true, mjc400_op_ri, NULL, NULL, D_2ARGN, T_RI },
	{ 024, "RW", false, true, mjc400_op_rw, NULL, NULL, D_2ARGN, T_RW },
	{ 025, "PW", false, true, mjc400_op_pw, NULL, NULL, D_2ARGN, T_PW },
	{ 026, "RJ", false, true, mjc400_op_rj, NULL, NULL, D_2ARGN, T_RJ },
	{ 027, "IS", false, true, mjc400_op_is, NULL, NULL, D_2ARGN, T_IS },
	{ 030, "BB", false, true, mjc400_op_bb, NULL, NULL, D_2ARGN, T_BB },
	{ 031, "BM", false, true, mjc400_op_bm, NULL, NULL, D_2ARGN, T_BM },
	{ 032, "BS", false, true, mjc400_op_bs, NULL, NULL, D_2ARGN, T_BS },
	{ 033, "BC", false, true, mjc400_op_bc, NULL, NULL, D_2ARGN, T_BC },
	{ 034, "BN", false, true, mjc400_op_bn, NULL, NULL, D_2ARGN, T_BN },
	{ 035, "OU", false, true, mjc400_op_ou, NULL, NULL, D_2ARGN, T_OU },
	{ 036, "IN", false, true, mjc400_op_in, NULL, NULL, D_2ARGN, T_IN },

	{ 037, NULL, false, false, mjc400_op_37, mjc400_dt_e37, mjc400_iset_37, NULL },

	{ 040, "AW", false, true, mjc400_op_aw, NULL, NULL, D_2ARGN, T_AW },
	{ 041, "AC", false, true, mjc400_op_ac, NULL, NULL, D_2ARGN, T_AC },
	{ 042, "SW", false, true, mjc400_op_sw, NULL, NULL, D_2ARGN, T_SW },
	{ 043, "CW", false, true, mjc400_op_cw, NULL, NULL, D_2ARGN, T_CW },
	{ 044, "OR", false, true, mjc400_op_or, NULL, NULL, D_2ARGN, T_OR },
	{ 045, "OM", false, true, mjc400_op_om, NULL, NULL, D_2ARGN, T_OM },
	{ 046, "NR", false, true, mjc400_op_nr, NULL, NULL, D_2ARGN, T_NR },
	{ 047, "NM", false, true, mjc400_op_nm, NULL, NULL, D_2ARGN, T_NM },
	{ 050, "ER", false, true, mjc400_op_er, NULL, NULL, D_2ARGN, T_ER },
	{ 051, "EM", false, true, mjc400_op_em, NULL, NULL, D_2ARGN, T_EM },
	{ 052, "XR", false, true, mjc400_op_xr, NULL, NULL, D_2ARGN, T_XR },
	{ 053, "XM", false, true, mjc400_op_xm, NULL, NULL, D_2ARGN, T_XM },
	{ 054, "CL", false, true, mjc400_op_cl, NULL, NULL, D_2ARGN, T_CL },
	{ 055, "LB", false, true, mjc400_op_lb, NULL, NULL, D_2ARGN, T_LB },
	{ 056, "RB", false, true, mjc400_op_rb, NULL, NULL, D_2ARGN, T_RB },
	{ 057, "CB", false, true, mjc400_op_cb, NULL, NULL, D_2ARGN, T_CB },

	{ 060, "AWT", false, false, mjc400_op_awt, NULL, NULL, D_KA1, T_AWT },
	{ 061, "TRB", false, false, mjc400_op_trb, NULL, NULL, D_KA1, T_TRB },
	{ 062, "IRB", false, false, mjc400_op_irb, NULL, NULL, D_KA1, T_IRB },
	{ 063, "DRB", false, false, mjc400_op_drb, NULL, NULL, D_KA1, T_DRB },
	{ 064, "CWT", false, false, mjc400_op_cwt, NULL, NULL, D_KA1, T_CWT },
	{ 065, "LWT", false, false, mjc400_op_lwt, NULL, NULL, D_KA1, T_LWT },
	{ 066, "LWS", false, false, mjc400_op_lws, NULL, NULL, D_KA1, T_LWS },
	{ 067, "RWS", false, false, mjc400_op_rws, NULL, NULL, D_KA1, T_RWS },

	{ 070, NULL, false, false, mjc400_op_70, mjc400_dt_e70, mjc400_iset_70, NULL },
	{ 071, NULL, false, false, mjc400_op_71, mjc400_dt_e71, mjc400_iset_71, NULL },
	{ 072, NULL, false, false, mjc400_op_72, mjc400_dt_e72, mjc400_iset_72, NULL },
	{ 073, NULL, false, false, mjc400_op_73, mjc400_dt_e73, mjc400_iset_73, NULL },
	{ 074, NULL, false, false, mjc400_op_74, mjc400_dt_e74, mjc400_iset_74, NULL },
	{ 075, NULL, false, false, mjc400_op_75, mjc400_dt_e75, mjc400_iset_75, NULL },
	{ 076, NULL, false, false, mjc400_op_76, mjc400_dt_e76, mjc400_iset_76, NULL },
	{ 077, NULL, false, false, mjc400_op_77, mjc400_dt_e77, mjc400_iset_77, NULL }
};

struct mjc400_opdef mjc400_iset_37[] = {
	{ 0, "AD", false, true, mjc400_op_37_ad, NULL, NULL, D_FD, T_AD },
	{ 1, "SD", false, true, mjc400_op_37_sd, NULL, NULL, D_FD, T_SD },
	{ 2, "MW", false, true, mjc400_op_37_mw, NULL, NULL, D_FD, T_MW },
	{ 3, "DW", false, true, mjc400_op_37_dw, NULL, NULL, D_FD, T_DW },
	{ 4, "AF", false, true, mjc400_op_37_af, NULL, NULL, D_FD, T_AF },
	{ 5, "SF", false, true, mjc400_op_37_sf, NULL, NULL, D_FD, T_SF },
	{ 6, "MF", false, true, mjc400_op_37_mf, NULL, NULL, D_FD, T_MF },
	{ 7, "DF", false, true, mjc400_op_37_df, NULL, NULL, D_FD, T_DF }
};

struct mjc400_opdef mjc400_iset_70[] = {
	{ 0, "UJS", false, false, mjc400_op_70_ujs, NULL, NULL, D_JS, T_UJS },
	{ 1, "JLS", false, false, mjc400_op_70_jls, NULL, NULL, D_JS, T_JLS },
	{ 2, "JES", false, false, mjc400_op_70_jes, NULL, NULL, D_JS, T_JES },
	{ 3, "JGS", false, false, mjc400_op_70_jgs, NULL, NULL, D_JS, T_JGS },
	{ 4, "JVS", false, false, mjc400_op_70_jvs, NULL, NULL, D_JS, T_JVS },
	{ 5, "JXS", false, false, mjc400_op_70_jxs, NULL, NULL, D_JS, T_JXS },
	{ 6, "JYS", false, false, mjc400_op_70_jys, NULL, NULL, D_JS, T_JYS },
	{ 7, "JCS", false, false, mjc400_op_70_jcs, NULL, NULL, D_JS, T_JCS }
};

struct mjc400_opdef mjc400_iset_71[] = {
	{ 0, "BLC", false, false, mjc400_op_71_blc, NULL, NULL, D_KA2, T_BLC },
	{ 1, "EXL", false, false, mjc400_op_71_exl, NULL, NULL, D_KA2, T_EXL },
	{ 2, "BRC", false, false, mjc400_op_71_brc, NULL, NULL, D_KA2, T_BRC },
	{ 3, "NRF", false, false, mjc400_op_71_nrf, NULL, NULL, D_KA2, T_NRF }
};

struct mjc400_opdef mjc400_iset_72[] = {
	{ 0b000000, "RIC", false, false, mjc400_op_72_ric, NULL, NULL, D_C, T_RIC },
	{ 0b000001, "ZLB", false, false, mjc400_op_72_zlb, NULL, NULL, D_C, T_ZLB },
	{ 0b000010, "SXU", false, false, mjc400_op_72_sxu, NULL, NULL, D_C, T_SXU },
	{ 0b000011, "NGA", false, false, mjc400_op_72_nga, NULL, NULL, D_C, T_NGA },
	{ 0b000100, "SLZ", false, false, mjc400_op_72_slz, NULL, NULL, D_C, T_SLZ },
	{ 0b000101, "SLY", false, false, mjc400_op_72_sly, NULL, NULL, D_C, T_SLY },
	{ 0b000110, "SLX", false, false, mjc400_op_72_slx, NULL, NULL, D_C, T_SLX },
	{ 0b000111, "SRY", false, false, mjc400_op_72_sry, NULL, NULL, D_C, T_SRY },
	{ 0b001000, "NGL", false, false, mjc400_op_72_ngl, NULL, NULL, D_C, T_NGL },
	{ 0b001001, "RPC", false, false, mjc400_op_72_rpc, NULL, NULL, D_C, T_RPC },
	{ 0b001010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010000, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010001, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010010, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010011, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010100, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010101, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010110, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b010111, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b011000, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011001, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b100000, "RKY", false, false, mjc400_op_72_rky, NULL, NULL, D_C, T_RKY },
	{ 0b100001, "ZRB", false, false, mjc400_op_72_zrb, NULL, NULL, D_C, T_ZRB },
	{ 0b100010, "SXL", false, false, mjc400_op_72_sxl, NULL, NULL, D_C, T_SXL },
	{ 0b100011, "NGC", false, false, mjc400_op_72_ngc, NULL, NULL, D_C, T_NGC },
	{ 0b100100, "SVZ", false, false, mjc400_op_72_svz, NULL, NULL, D_C, T_SVZ },
	{ 0b100101, "SVY", false, false, mjc400_op_72_svy, NULL, NULL, D_C, T_SVY },
	{ 0b100110, "SVX", false, false, mjc400_op_72_svx, NULL, NULL, D_C, T_SVX },
	{ 0b100111, "SRX", false, false, mjc400_op_72_srx, NULL, NULL, D_C, T_SRX },
	{ 0b101000, "SRZ", false, false, mjc400_op_72_srz, NULL, NULL, D_C, T_SRZ },
	{ 0b101001, "LPC", false, false, mjc400_op_72_lpc, NULL, NULL, D_C, T_LPC },
	{ 0b101010, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101011, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101100, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101101, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101110, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101111, NULL,  false, false, mjc400_op_illegal, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110001, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110000, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110010, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110011, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110100, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110101, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110110, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
	{ 0b110111, "SHC", false, false, mjc400_op_72_shc, NULL, NULL, D_SHC, T_SHC },
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
	{ 0b000000, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b000001, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b000010, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b000011, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b000100, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b000101, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b000110, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b000111, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b001000, "CIT", true, false, mjc400_op_73_cit, NULL, NULL, D_S, T_CIT },
	{ 0b001001, "SIL", true, false, mjc400_op_73_sil, NULL, NULL, D_S, T_SIL },
	{ 0b001010, "SIU", true, false, mjc400_op_73_siu, NULL, NULL, D_S, T_SIU },
	{ 0b001011, "SIT", true, false, mjc400_op_73_sit, NULL, NULL, D_S, T_SIT },
	{ 0b001100, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, T_GIU },
	{ 0b001101, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, T_GIU },
	{ 0b001110, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, T_GIU },
	{ 0b001111, "GIU", true, false, mjc400_op_73_giu, NULL, NULL, D_S, T_GIU },
	{ 0b010000, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b010001, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b010010, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b010011, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
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
	{ 0b100000, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b100001, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b100010, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b100011, "HLT", true, false, mjc400_op_73_hlt, NULL, NULL, D_S, T_HLT },
	{ 0b100100, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b100101, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b100110, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b100111, "MCL", true, false, mjc400_op_73_mcl, NULL, NULL, D_S, T_MCL },
	{ 0b101000, "CIT", true, false, mjc400_op_73_cit, NULL, NULL, D_S, T_CIT },
	{ 0b101001, "SIL", true, false, mjc400_op_73_sil, NULL, NULL, D_S, T_SIL },
	{ 0b101010, "SIU", true, false, mjc400_op_73_siu, NULL, NULL, D_S, T_SIU },
	{ 0b101011, "SIT", true, false, mjc400_op_73_sit, NULL, NULL, D_S, T_SIT },
	{ 0b101100, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, T_GIL },
	{ 0b101101, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, T_GIL },
	{ 0b101110, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, T_GIL },
	{ 0b101111, "GIL", true, false, mjc400_op_73_gil, NULL, NULL, D_S, T_GIL },
	{ 0b110001, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b110000, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b110010, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
	{ 0b110011, "LIP", true, false, mjc400_op_73_lip, NULL, NULL, D_S, T_LIP },
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
	{ 0, "UJ", false, true, mjc400_op_74_uj, NULL, NULL, D_J, T_UJ },
	{ 1, "JL", false, true, mjc400_op_74_jl, NULL, NULL, D_J, T_JL },
	{ 2, "JE", false, true, mjc400_op_74_je, NULL, NULL, D_J, T_JE },
	{ 3, "JG", false, true, mjc400_op_74_jg, NULL, NULL, D_J, T_JG },
	{ 4, "JZ", false, true, mjc400_op_74_jz, NULL, NULL, D_J, T_JZ },
	{ 5, "JM", false, true, mjc400_op_74_jm, NULL, NULL, D_J, T_JM },
	{ 6, "JN", false, true, mjc400_op_74_jn, NULL, NULL, D_J, T_JN },
	{ 7, "LJ", false, true, mjc400_op_74_lj, NULL, NULL, D_J, T_LJ }
};

struct mjc400_opdef mjc400_iset_75[] = {
	{ 0, "LD", false, true, mjc400_op_75_ld, NULL, NULL, D_L, T_LD },
	{ 1, "LF", false, true, mjc400_op_75_lf, NULL, NULL, D_L, T_LF },
	{ 2, "LA", false, true, mjc400_op_75_la, NULL, NULL, D_L, T_LA },
	{ 3, "LL", false, true, mjc400_op_75_ll, NULL, NULL, D_L, T_LL },
	{ 4, "TD", false, true, mjc400_op_75_td, NULL, NULL, D_L, T_TD },
	{ 5, "TF", false, true, mjc400_op_75_tf, NULL, NULL, D_L, T_TF },
	{ 6, "TA", false, true, mjc400_op_75_ta, NULL, NULL, D_L, T_TA },
	{ 7, "TL", false, true, mjc400_op_75_tl, NULL, NULL, D_L, T_TL }
};

struct mjc400_opdef mjc400_iset_76[] = {
	{ 0, "RD", false, true, mjc400_op_76_rd, NULL, NULL, D_G, T_RD },
	{ 1, "RF", false, true, mjc400_op_76_rf, NULL, NULL, D_G, T_RF },
	{ 2, "RA", false, true, mjc400_op_76_ra, NULL, NULL, D_G, T_RA },
	{ 3, "RL", false, true, mjc400_op_76_rl, NULL, NULL, D_G, T_RL },
	{ 4, "PD", false, true, mjc400_op_76_pd, NULL, NULL, D_G, T_PD },
	{ 5, "PF", false, true, mjc400_op_76_pf, NULL, NULL, D_G, T_PF },
	{ 6, "PA", false, true, mjc400_op_76_pa, NULL, NULL, D_G, T_PA },
	{ 7, "PL", false, true, mjc400_op_76_pl, NULL, NULL, D_G, T_PL }
};

struct mjc400_opdef mjc400_iset_77[] = {
	{ 0, "MB", true,  true, mjc400_op_77_mb, NULL, NULL, D_BN, T_MB },
	{ 1, "IM", true,  true, mjc400_op_77_im, NULL, NULL, D_BN, T_IM },
	{ 2, "KI", true,  true, mjc400_op_77_ki, NULL, NULL, D_BN, T_KI },
	{ 3, "FI", true,  true, mjc400_op_77_fi, NULL, NULL, D_BN, T_FI },
	{ 4, "SP", true,  true, mjc400_op_77_sp, NULL, NULL, D_BN, T_SP },
	{ 5, "MD", false, true, mjc400_op_77_md, NULL, NULL, D_BN, T_MD },
	{ 6, "RZ", false, true, mjc400_op_77_rz, NULL, NULL, D_BN, T_RZ },
	{ 7, "IB", false, true, mjc400_op_77_ib, NULL, NULL, D_BN, T_IB }
};

// vim: tabstop=4
