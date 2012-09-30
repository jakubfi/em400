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

mjc400_opdef mjc400_iset[] = {
	{ 000, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 001, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 002, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 003, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 004, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 005, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 006, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 007, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 012, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 013, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 014, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 015, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 016, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 017, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	
	{ 020, "LW", "LW", false, mjc400_op_lw, mjc400_trans_lw, mjc400_dasm_2argn },
	{ 021, "TW", "TW", false, mjc400_op_tw, mjc400_trans_tw, mjc400_dasm_2argn },
	{ 022, "LS", "LS", false, mjc400_op_ls, mjc400_trans_ls, mjc400_dasm_2argn },
	{ 023, "RI", "RI", false, mjc400_op_ri, mjc400_trans_ri, mjc400_dasm_2argn },
	{ 024, "RW", "RW", false, mjc400_op_rw, mjc400_trans_rw, mjc400_dasm_2argn },
	{ 025, "PW", "PW", false, mjc400_op_pw, mjc400_trans_pw, mjc400_dasm_2argn },
	{ 026, "RJ", "RJ", false, mjc400_op_rj, mjc400_trans_rj, mjc400_dasm_2argn },
	{ 027, "IS", "IS", false, mjc400_op_is, mjc400_trans_is, mjc400_dasm_2argn },
	{ 030, "BB", "BB", false, mjc400_op_bb, mjc400_trans_bb, mjc400_dasm_2argn },
	{ 031, "BM", "BM", false, mjc400_op_bm, mjc400_trans_bm, mjc400_dasm_2argn },
	{ 032, "BS", "BS", false, mjc400_op_bs, mjc400_trans_bs, mjc400_dasm_2argn },
	{ 033, "BC", "BC", false, mjc400_op_bc, mjc400_trans_bc, mjc400_dasm_2argn },
	{ 034, "BN", "BN", false, mjc400_op_bn, mjc400_trans_bn, mjc400_dasm_2argn },
	{ 035, "OU", "OU", false, mjc400_op_ou, mjc400_trans_ou, mjc400_dasm_2argn },
	{ 036, "IN", "IN", false, mjc400_op_in, mjc400_trans_in, mjc400_dasm_2argn },

	{ 037, "-i-", "-i-", false, mjc400_op_37, mjc400_trans_37, mjc400_dasm_37 },

	{ 040, "AW", "AW", false, mjc400_op_aw, mjc400_trans_aw, mjc400_dasm_2argn },
	{ 041, "AC", "AC", false, mjc400_op_ac, mjc400_trans_ac, mjc400_dasm_2argn },
	{ 042, "SW", "SW", false, mjc400_op_sw, mjc400_trans_sw, mjc400_dasm_2argn },
	{ 043, "CW", "CW", false, mjc400_op_cw, mjc400_trans_cw, mjc400_dasm_2argn },
	{ 044, "OR", "OR", false, mjc400_op_or, mjc400_trans_or, mjc400_dasm_2argn },
	{ 045, "OM", "OM", false, mjc400_op_om, mjc400_trans_om, mjc400_dasm_2argn },
	{ 046, "NR", "NR", false, mjc400_op_nr, mjc400_trans_nr, mjc400_dasm_2argn },
	{ 047, "NM", "NM", false, mjc400_op_nm, mjc400_trans_nm, mjc400_dasm_2argn },
	{ 050, "ER", "ER", false, mjc400_op_er, mjc400_trans_er, mjc400_dasm_2argn },
	{ 051, "EM", "EM", false, mjc400_op_em, mjc400_trans_em, mjc400_dasm_2argn },
	{ 052, "XR", "XR", false, mjc400_op_xr, mjc400_trans_xr, mjc400_dasm_2argn },
	{ 053, "XM", "XM", false, mjc400_op_xm, mjc400_trans_xm, mjc400_dasm_2argn },
	{ 054, "CL", "CL", false, mjc400_op_cl, mjc400_trans_cl, mjc400_dasm_2argn },
	{ 055, "LB", "LB", false, mjc400_op_lb, mjc400_trans_lb, mjc400_dasm_2argn },
	{ 056, "RB", "RB", false, mjc400_op_rb, mjc400_trans_rb, mjc400_dasm_2argn },
	{ 057, "CB", "CB", false, mjc400_op_cb, mjc400_trans_cb, mjc400_dasm_2argn },

	{ 060, "AWT", "AWT", false, mjc400_op_awt, mjc400_trans_awt, mjc400_dasm_ka1 },
	{ 061, "TRB", "TRB", false, mjc400_op_trb, mjc400_trans_trb, mjc400_dasm_ka1 },
	{ 062, "IRB", "IRB", false, mjc400_op_irb, mjc400_trans_irb, mjc400_dasm_ka1 },
	{ 063, "DRB", "DRB", false, mjc400_op_drb, mjc400_trans_drb, mjc400_dasm_ka1 },
	{ 064, "CWT", "CWT", false, mjc400_op_cwt, mjc400_trans_cwt, mjc400_dasm_ka1 },
	{ 065, "LWT", "LWT", false, mjc400_op_lwt, mjc400_trans_lwt, mjc400_dasm_ka1 },
	{ 066, "LWS", "LWS", false, mjc400_op_lws, mjc400_trans_lws, mjc400_dasm_ka1 },
	{ 067, "RWS", "RWS", false, mjc400_op_rws, mjc400_trans_rws, mjc400_dasm_ka1 },

	{ 070, "-i-", "-i-", false, mjc400_op_70, mjc400_trans_70, mjc400_dasm_70 },
	{ 071, "-i-", "-i-", false, mjc400_op_71, mjc400_trans_71, mjc400_dasm_71 },
	{ 072, "-i-", "-i-", false, mjc400_op_72, mjc400_trans_72, mjc400_dasm_72 },
	{ 073, "-i-", "-i-", false, mjc400_op_73, mjc400_trans_73, mjc400_dasm_73 },
	{ 074, "-i-", "-i-", false, mjc400_op_74, mjc400_trans_74, mjc400_dasm_74 },
	{ 075, "-i-", "-i-", false, mjc400_op_75, mjc400_trans_75, mjc400_dasm_75 },
	{ 076, "-i-", "-i-", false, mjc400_op_76, mjc400_trans_76, mjc400_dasm_76 },
	{ 077, "-i-", "-i-", false, mjc400_op_77, mjc400_trans_77, mjc400_dasm_77 }
};

mjc400_opdef mjc400_iset_37[] = {
	{ 0, "AD", "AD", false, mjc400_op_37_ad, mjc400_trans_37_ad, mjc400_dasm_fd },
	{ 1, "SD", "SD", false, mjc400_op_37_sd, mjc400_trans_37_sd, mjc400_dasm_fd },
	{ 2, "MW", "MW", false, mjc400_op_37_mw, mjc400_trans_37_mw, mjc400_dasm_fd },
	{ 3, "DW", "DW", false, mjc400_op_37_dw, mjc400_trans_37_dw, mjc400_dasm_fd },
	{ 4, "AF", "AF", false, mjc400_op_37_af, mjc400_trans_37_af, mjc400_dasm_fd },
	{ 5, "SF", "SF", false, mjc400_op_37_sf, mjc400_trans_37_sf, mjc400_dasm_fd },
	{ 6, "MF", "MF", false, mjc400_op_37_mf, mjc400_trans_37_mf, mjc400_dasm_fd },
	{ 7, "DF", "DF", false, mjc400_op_37_df, mjc400_trans_37_df, mjc400_dasm_fd }
};

mjc400_opdef mjc400_iset_70[] = {
	{ 0, "UJS", "UJS", false, mjc400_op_70_ujs, mjc400_trans_70_ujs, mjc400_dasm_js },
	{ 1, "JLS", "JLS", false, mjc400_op_70_jls, mjc400_trans_70_jls, mjc400_dasm_js },
	{ 2, "JES", "JES", false, mjc400_op_70_jes, mjc400_trans_70_jes, mjc400_dasm_js },
	{ 3, "JGS", "JGS", false, mjc400_op_70_jgs, mjc400_trans_70_jgs, mjc400_dasm_js },
	{ 4, "JVS", "JVS", false, mjc400_op_70_jvs, mjc400_trans_70_jvs, mjc400_dasm_js },
	{ 5, "JXS", "JXS", false, mjc400_op_70_jxs, mjc400_trans_70_jxs, mjc400_dasm_js },
	{ 6, "JYS", "JYS", false, mjc400_op_70_jys, mjc400_trans_70_jys, mjc400_dasm_js },
	{ 7, "JCS", "JCS", false, mjc400_op_70_jcs, mjc400_trans_70_jcs, mjc400_dasm_js }
};

mjc400_opdef mjc400_iset_71[] = {
	{ 0, "BLC", "BLC", false, mjc400_op_71_blc, mjc400_trans_71_blc, mjc400_dasm_ka2 },
	{ 1, "EXL", "EXL", false, mjc400_op_71_exl, mjc400_trans_71_exl, mjc400_dasm_ka2 },
	{ 2, "BRC", "BRC", false, mjc400_op_71_brc, mjc400_trans_71_brc, mjc400_dasm_ka2 },
	{ 3, "NRF", "NRF", false, mjc400_op_71_nrf, mjc400_trans_71_nrf, mjc400_dasm_ka2 }
};

mjc400_opdef mjc400_iset_72[] = {
	{ 0b000000, "RIC", "RIC", false, mjc400_op_72_ric, mjc400_trans_72_ric, mjc400_dasm_c },
	{ 0b000001, "ZLB", "ZLB", false, mjc400_op_72_zlb, mjc400_trans_72_zlb, mjc400_dasm_c },
	{ 0b000010, "SXU", "SXU", false, mjc400_op_72_sxu, mjc400_trans_72_sxu, mjc400_dasm_c },
	{ 0b000011, "NGA", "NGA", false, mjc400_op_72_nga, mjc400_trans_72_nga, mjc400_dasm_c },
	{ 0b000100, "SLZ", "SLZ", false, mjc400_op_72_slz, mjc400_trans_72_slz, mjc400_dasm_c },
	{ 0b000101, "SLY", "SLY", false, mjc400_op_72_sly, mjc400_trans_72_sly, mjc400_dasm_c },
	{ 0b000110, "SLX", "SLX", false, mjc400_op_72_slx, mjc400_trans_72_slx, mjc400_dasm_c },
	{ 0b000111, "SRY", "SRY", false, mjc400_op_72_sry, mjc400_trans_72_sry, mjc400_dasm_c },
	{ 0b001000, "NGL", "NGL", false, mjc400_op_72_ngl, mjc400_trans_72_ngl, mjc400_dasm_c },
	{ 0b001001, "RPC", "RPC", false, mjc400_op_72_rpc, mjc400_trans_72_rpc, mjc400_dasm_c },
	{ 0b001010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b001011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b001100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b001101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b001110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b001111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b010000, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010001, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010010, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010011, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010100, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010101, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010110, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b010111, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b011000, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011001, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b100000, "RKY", "RKY", false, mjc400_op_72_rky, mjc400_trans_72_rky, mjc400_dasm_c },
	{ 0b100001, "ZRB", "ZRB", false, mjc400_op_72_zrb, mjc400_trans_72_zrb, mjc400_dasm_c },
	{ 0b100010, "SXL", "SXL", false, mjc400_op_72_sxl, mjc400_trans_72_sxl, mjc400_dasm_c },
	{ 0b100011, "NGC", "NGC", false, mjc400_op_72_ngc, mjc400_trans_72_ngc, mjc400_dasm_c },
	{ 0b100100, "SVZ", "SVZ", false, mjc400_op_72_svz, mjc400_trans_72_svz, mjc400_dasm_c },
	{ 0b100101, "SVY", "SVY", false, mjc400_op_72_svy, mjc400_trans_72_svy, mjc400_dasm_c },
	{ 0b100110, "SVX", "SVX", false, mjc400_op_72_svx, mjc400_trans_72_svx, mjc400_dasm_c },
	{ 0b100111, "SRX", "SRX", false, mjc400_op_72_srx, mjc400_trans_72_srx, mjc400_dasm_c },
	{ 0b101000, "SRZ", "SRZ", false, mjc400_op_72_srz, mjc400_trans_72_srz, mjc400_dasm_c },
	{ 0b101001, "LPC", "LPC", false, mjc400_op_72_lpc, mjc400_trans_72_lpc, mjc400_dasm_c },
	{ 0b101010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b101011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b101100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b101101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b101110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b101111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b110001, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110000, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110010, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110011, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110100, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110101, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110110, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b110111, "SHC", "SHC", false, mjc400_op_72_shc, mjc400_trans_72_shc, mjc400_dasm_c_shc },
	{ 0b111000, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111001, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal }
};

mjc400_opdef mjc400_iset_73[] = {
	{ 0b000000, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b000001, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b000010, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b000011, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b000100, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b000101, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b000110, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b000111, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b001000, "CIT", "CIT", true, mjc400_op_73_cit, mjc400_trans_73_cit, mjc400_dasm_s },
	{ 0b001001, "SIL", "SIL", true, mjc400_op_73_sil, mjc400_trans_73_sil, mjc400_dasm_s },
	{ 0b001010, "SIU", "SIU", true, mjc400_op_73_siu, mjc400_trans_73_siu, mjc400_dasm_s },
	{ 0b001011, "SIT", "SIT", true, mjc400_op_73_sit, mjc400_trans_73_sit, mjc400_dasm_s },
	{ 0b001100, "GIU", "GIU", true, mjc400_op_73_giu, mjc400_trans_73_giu, mjc400_dasm_s },
	{ 0b001101, "GIU", "GIU", true, mjc400_op_73_giu, mjc400_trans_73_giu, mjc400_dasm_s },
	{ 0b001110, "GIU", "GIU", true, mjc400_op_73_giu, mjc400_trans_73_giu, mjc400_dasm_s },
	{ 0b001111, "GIU", "GIU", true, mjc400_op_73_giu, mjc400_trans_73_giu, mjc400_dasm_s },
	{ 0b010000, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b010001, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b010010, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b010011, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b010100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b010101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b010110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b010111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011000, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011001, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b011111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b100000, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b100001, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b100010, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b100011, "HLT", "HLT", true, mjc400_op_73_hlt, mjc400_trans_73_hlt, mjc400_dasm_s },
	{ 0b100100, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b100101, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b100110, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b100111, "MCL", "MCL", true, mjc400_op_73_mcl, mjc400_trans_73_mcl, mjc400_dasm_s },
	{ 0b101000, "CIT", "CIT", true, mjc400_op_73_cit, mjc400_trans_73_cit, mjc400_dasm_s },
	{ 0b101001, "SIL", "SIL", true, mjc400_op_73_sil, mjc400_trans_73_sil, mjc400_dasm_s },
	{ 0b101010, "SIU", "SIU", true, mjc400_op_73_siu, mjc400_trans_73_siu, mjc400_dasm_s },
	{ 0b101011, "SIT", "SIT", true, mjc400_op_73_sit, mjc400_trans_73_sit, mjc400_dasm_s },
	{ 0b101100, "GIL", "GIL", true, mjc400_op_73_gil, mjc400_trans_73_gil, mjc400_dasm_s },
	{ 0b101101, "GIL", "GIL", true, mjc400_op_73_gil, mjc400_trans_73_gil, mjc400_dasm_s },
	{ 0b101110, "GIL", "GIL", true, mjc400_op_73_gil, mjc400_trans_73_gil, mjc400_dasm_s },
	{ 0b101111, "GIL", "GIL", true, mjc400_op_73_gil, mjc400_trans_73_gil, mjc400_dasm_s },
	{ 0b110001, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b110000, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b110010, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b110011, "LIP", "LIP", true, mjc400_op_73_lip, mjc400_trans_73_lip, mjc400_dasm_s },
	{ 0b110100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b110101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b110110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b110111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111000, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111001, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111010, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111011, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111100, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111101, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111110, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal },
	{ 0b111111, "-i-", "-i-", false, mjc400_op_illegal, mjc400_dasm_illegal, mjc400_dasm_illegal }
};

mjc400_opdef mjc400_iset_74[] = {
	{ 0, "UJ", "UJ", false, mjc400_op_74_uj, mjc400_trans_74_uj, mjc400_dasm_j },
	{ 1, "JL", "JL", false, mjc400_op_74_jl, mjc400_trans_74_jl, mjc400_dasm_j },
	{ 2, "JE", "JE", false, mjc400_op_74_je, mjc400_trans_74_je, mjc400_dasm_j },
	{ 3, "JG", "JG", false, mjc400_op_74_jg, mjc400_trans_74_jg, mjc400_dasm_j },
	{ 4, "JZ", "JZ", false, mjc400_op_74_jz, mjc400_trans_74_jz, mjc400_dasm_j },
	{ 5, "JM", "JM", false, mjc400_op_74_jm, mjc400_trans_74_jm, mjc400_dasm_j },
	{ 6, "JN", "JN", false, mjc400_op_74_jn, mjc400_trans_74_jn, mjc400_dasm_j },
	{ 7, "LJ", "LJ", false, mjc400_op_74_lj, mjc400_trans_74_lj, mjc400_dasm_j }
};

mjc400_opdef mjc400_iset_75[] = {
	{ 0, "LD", "LD", false, mjc400_op_75_ld, mjc400_trans_75_ld, mjc400_dasm_l },
	{ 1, "LF", "LF", false, mjc400_op_75_lf, mjc400_trans_75_lf, mjc400_dasm_l },
	{ 2, "LA", "LA", false, mjc400_op_75_la, mjc400_trans_75_la, mjc400_dasm_l },
	{ 3, "LL", "LL", false, mjc400_op_75_ll, mjc400_trans_75_ll, mjc400_dasm_l },
	{ 4, "TD", "TD", false, mjc400_op_75_td, mjc400_trans_75_td, mjc400_dasm_l },
	{ 5, "TF", "TF", false, mjc400_op_75_tf, mjc400_trans_75_tf, mjc400_dasm_l },
	{ 6, "TA", "TA", false, mjc400_op_75_ta, mjc400_trans_75_ta, mjc400_dasm_l },
	{ 7, "TL", "TL", false, mjc400_op_75_tl, mjc400_trans_75_tl, mjc400_dasm_l }
};

mjc400_opdef mjc400_iset_76[] = {
	{ 0, "RD", "RD", false, mjc400_op_76_rd, mjc400_trans_76_rd, mjc400_dasm_g },
	{ 1, "RF", "RF", false, mjc400_op_76_rf, mjc400_trans_76_rf, mjc400_dasm_g },
	{ 2, "RA", "RA", false, mjc400_op_76_ra, mjc400_trans_76_ra, mjc400_dasm_g },
	{ 3, "RL", "RL", false, mjc400_op_76_rl, mjc400_trans_76_rl, mjc400_dasm_g },
	{ 4, "PD", "PD", false, mjc400_op_76_pd, mjc400_trans_76_pd, mjc400_dasm_g },
	{ 5, "PF", "PF", false, mjc400_op_76_pf, mjc400_trans_76_pf, mjc400_dasm_g },
	{ 6, "PA", "PA", false, mjc400_op_76_pa, mjc400_trans_76_pa, mjc400_dasm_g },
	{ 7, "PL", "PL", false, mjc400_op_76_pl, mjc400_trans_76_pl, mjc400_dasm_g }
};

mjc400_opdef mjc400_iset_77[] = {
	{ 0, "MB", "MB", true,  mjc400_op_77_mb, mjc400_trans_77_mb, mjc400_dasm_bn },
	{ 1, "IM", "IM", true,  mjc400_op_77_im, mjc400_trans_77_im, mjc400_dasm_bn },
	{ 2, "KI", "KI", true,  mjc400_op_77_ki, mjc400_trans_77_ki, mjc400_dasm_bn },
	{ 3, "FI", "FI", true,  mjc400_op_77_fi, mjc400_trans_77_fi, mjc400_dasm_bn },
	{ 4, "SP", "SP", true,  mjc400_op_77_sp, mjc400_trans_77_sp, mjc400_dasm_bn },
	{ 5, "MD", "MD", false, mjc400_op_77_md, mjc400_trans_77_md, mjc400_dasm_bn },
	{ 6, "RZ", "RZ", false, mjc400_op_77_rz, mjc400_trans_77_rz, mjc400_dasm_bn },
	{ 7, "IB", "IB", false, mjc400_op_77_ib, mjc400_trans_77_ib, mjc400_dasm_bn }
};

