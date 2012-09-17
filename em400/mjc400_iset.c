//  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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

mjc400_opdef mjc400_iset[] = {
	{ 000, "-i-", false, mjc400_op_illegal },
	{ 001, "-i-", false, mjc400_op_illegal },
	{ 002, "-i-", false, mjc400_op_illegal },
	{ 003, "-i-", false, mjc400_op_illegal },
	{ 004, "-i-", false, mjc400_op_illegal },
	{ 005, "-i-", false, mjc400_op_illegal },
	{ 006, "-i-", false, mjc400_op_illegal },
	{ 007, "-i-", false, mjc400_op_illegal },
	{ 010, "-i-", false, mjc400_op_illegal },
	{ 011, "-i-", false, mjc400_op_illegal },
	{ 012, "-i-", false, mjc400_op_illegal },
	{ 013, "-i-", false, mjc400_op_illegal },
	{ 014, "-i-", false, mjc400_op_illegal },
	{ 015, "-i-", false, mjc400_op_illegal },
	{ 016, "-i-", false, mjc400_op_illegal },
	{ 017, "-i-", false, mjc400_op_illegal },
	
	{ 020, "LW", false, mjc400_op_lw },
	{ 021, "TW", false, mjc400_op_tw },
	{ 022, "LS", false, mjc400_op_ls },
	{ 023, "RI", false, mjc400_op_ri },
	{ 024, "RW", false, mjc400_op_rw },
	{ 025, "PW", false, mjc400_op_pw },
	{ 026, "RJ", false, mjc400_op_rj },
	{ 027, "IS", false, mjc400_op_is },
	{ 030, "BB", false, mjc400_op_bb },
	{ 031, "BM", false, mjc400_op_bm },
	{ 032, "BS", false, mjc400_op_bs },
	{ 033, "BC", false, mjc400_op_bc },
	{ 034, "BN", false, mjc400_op_bn },
	{ 035, "OU", false, mjc400_op_ou },
	{ 036, "IN", false, mjc400_op_in },

	{ 037, "-i-", false, mjc400_op_37 },

	{ 040, "AW", false, mjc400_op_aw },
	{ 041, "AC", false, mjc400_op_ac },
	{ 042, "SW", false, mjc400_op_sw },
	{ 043, "CW", false, mjc400_op_cw },
	{ 044, "OR", false, mjc400_op_or },
	{ 045, "OM", false, mjc400_op_om },
	{ 046, "NR", false, mjc400_op_nr },
	{ 047, "NM", false, mjc400_op_nm },
	{ 050, "ER", false, mjc400_op_er },
	{ 051, "EM", false, mjc400_op_em },
	{ 052, "XR", false, mjc400_op_xr },
	{ 053, "XM", false, mjc400_op_xm },
	{ 054, "CL", false, mjc400_op_cl },
	{ 055, "LB", false, mjc400_op_lb },
	{ 056, "RB", false, mjc400_op_rb },
	{ 057, "CB", false, mjc400_op_cb },

	{ 060, "AWT", false, mjc400_op_awt },
	{ 061, "TRB", false, mjc400_op_trb },
	{ 062, "IRB", false, mjc400_op_irb },
	{ 063, "DRB", false, mjc400_op_drb },
	{ 064, "CWT", false, mjc400_op_cwt },
	{ 065, "LWT", false, mjc400_op_lwt },
	{ 066, "LWS", false, mjc400_op_lws },
	{ 067, "RWS", false, mjc400_op_rws },

	{ 070, "-i-", false, mjc400_op_70 },
	{ 071, "-i-", false, mjc400_op_71 },
	{ 072, "-i-", false, mjc400_op_72 },
	{ 073, "-i-", false, mjc400_op_73 },
	{ 074, "-i-", false, mjc400_op_74 },
	{ 075, "-i-", false, mjc400_op_75 },
	{ 076, "-i-", false, mjc400_op_76 },
	{ 077, "-i-", false, mjc400_op_77 }
};

// aaa
// 012
mjc400_opdef mjc400_iset_37[] = {
	{ 0, "AD", false, mjc400_op_37_ad },
	{ 1, "SD", false, mjc400_op_37_sd },
	{ 2, "MW", false, mjc400_op_37_mw },
	{ 3, "DW", false, mjc400_op_37_dw },
	{ 4, "AF", false, mjc400_op_37_af },
	{ 5, "SF", false, mjc400_op_37_sf },
	{ 6, "MF", false, mjc400_op_37_mf },
	{ 7, "DF", false, mjc400_op_37_df }
};

// aaa
// 012
mjc400_opdef mjc400_iset_70[] = {
	{ 0, "UJS", false, mjc400_op_70_ujs },
	{ 1, "JLS", false, mjc400_op_70_jls },
	{ 2, "JES", false, mjc400_op_70_jes },
	{ 3, "JGS", false, mjc400_op_70_jgs },
	{ 4, "JVS", false, mjc400_op_70_jvs },
	{ 5, "JXS", false, mjc400_op_70_jxs },
	{ 6, "JYS", false, mjc400_op_70_jys },
	{ 7, "JCS", false, mjc400_op_70_jcs }
};

// d a
// 0 0
mjc400_opdef mjc400_iset_71[] = {
	{ 0, "BLC", false, mjc400_op_71_blc },
	{ 1, "EXL", false, mjc400_op_71_exl },
	{ 2, "BRC", false, mjc400_op_71_brc },
	{ 3, "NRF", false, mjc400_op_71_nrf }
};

// d bb ccc
// 0 12 012
mjc400_opdef mjc400_iset_72[] = {
	{ 0b000000, "RIC", false, mjc400_op_72_ric },
	{ 0b000001, "ZLB", false, mjc400_op_72_zlb },
	{ 0b000010, "SXU", false, mjc400_op_72_sxu },
	{ 0b000011, "NGA", false, mjc400_op_72_nga },
	{ 0b000100, "SLZ", false, mjc400_op_72_slz },
	{ 0b000101, "SLY", false, mjc400_op_72_sly },
	{ 0b000110, "SLX", false, mjc400_op_72_slx },
	{ 0b000111, "SRY", false, mjc400_op_72_sry },
	{ 0b001000, "NGL", false, mjc400_op_72_ngl },
	{ 0b001001, "RPC", false, mjc400_op_72_rpc },
	{ 0b001010, "-i-", false, mjc400_op_illegal },
	{ 0b001011, "-i-", false, mjc400_op_illegal },
	{ 0b001100, "-i-", false, mjc400_op_illegal },
	{ 0b001101, "-i-", false, mjc400_op_illegal },
	{ 0b001110, "-i-", false, mjc400_op_illegal },
	{ 0b001111, "-i-", false, mjc400_op_illegal },
	{ 0b010000, "SHC", false, mjc400_op_72_shc },
	{ 0b010001, "SHC", false, mjc400_op_72_shc },
	{ 0b010010, "SHC", false, mjc400_op_72_shc },
	{ 0b010011, "SHC", false, mjc400_op_72_shc },
	{ 0b010100, "SHC", false, mjc400_op_72_shc },
	{ 0b010101, "SHC", false, mjc400_op_72_shc },
	{ 0b010110, "SHC", false, mjc400_op_72_shc },
	{ 0b010111, "SHC", false, mjc400_op_72_shc },
	{ 0b011000, "-i-", false, mjc400_op_illegal },
	{ 0b011001, "-i-", false, mjc400_op_illegal },
	{ 0b011010, "-i-", false, mjc400_op_illegal },
	{ 0b011011, "-i-", false, mjc400_op_illegal },
	{ 0b011100, "-i-", false, mjc400_op_illegal },
	{ 0b011101, "-i-", false, mjc400_op_illegal },
	{ 0b011110, "-i-", false, mjc400_op_illegal },
	{ 0b011111, "-i-", false, mjc400_op_illegal },
	{ 0b100000, "RKY", false, mjc400_op_72_rky },
	{ 0b100001, "ZRB", false, mjc400_op_72_zrb },
	{ 0b100010, "SXL", false, mjc400_op_72_sxl },
	{ 0b100011, "NGC", false, mjc400_op_72_ngc },
	{ 0b100100, "SVZ", false, mjc400_op_72_svz },
	{ 0b100101, "SVY", false, mjc400_op_72_svy },
	{ 0b100110, "SVX", false, mjc400_op_72_svx },
	{ 0b100111, "SRX", false, mjc400_op_72_srx },
	{ 0b101000, "SRZ", false, mjc400_op_72_srz },
	{ 0b101001, "LPC", false, mjc400_op_72_lpc },
	{ 0b101010, "-i-", false, mjc400_op_illegal },
	{ 0b101011, "-i-", false, mjc400_op_illegal },
	{ 0b101100, "-i-", false, mjc400_op_illegal },
	{ 0b101101, "-i-", false, mjc400_op_illegal },
	{ 0b101110, "-i-", false, mjc400_op_illegal },
	{ 0b101111, "-i-", false, mjc400_op_illegal },
	{ 0b110001, "SHC", false, mjc400_op_72_shc },
	{ 0b110000, "SHC", false, mjc400_op_72_shc },
	{ 0b110010, "SHC", false, mjc400_op_72_shc },
	{ 0b110011, "SHC", false, mjc400_op_72_shc },
	{ 0b110100, "SHC", false, mjc400_op_72_shc },
	{ 0b110101, "SHC", false, mjc400_op_72_shc },
	{ 0b110110, "SHC", false, mjc400_op_72_shc },
	{ 0b110111, "SHC", false, mjc400_op_72_shc },
	{ 0b111000, "-i-", false, mjc400_op_illegal },
	{ 0b111001, "-i-", false, mjc400_op_illegal },
	{ 0b111010, "-i-", false, mjc400_op_illegal },
	{ 0b111011, "-i-", false, mjc400_op_illegal },
	{ 0b111100, "-i-", false, mjc400_op_illegal },
	{ 0b111101, "-i-", false, mjc400_op_illegal },
	{ 0b111110, "-i-", false, mjc400_op_illegal },
	{ 0b111111, "-i-", false, mjc400_op_illegal }
};

// d aaa cc
// 0 012 12
mjc400_opdef mjc400_iset_73[] = {
	{ 0b000000, "HLT", true, mjc400_op_73_hlt },
	{ 0b000001, "HLT", true, mjc400_op_73_hlt },
	{ 0b000010, "HLT", true, mjc400_op_73_hlt },
	{ 0b000011, "HLT", true, mjc400_op_73_hlt },
	{ 0b000100, "MCL", true, mjc400_op_73_mcl },
	{ 0b000101, "MCL", true, mjc400_op_73_mcl },
	{ 0b000110, "MCL", true, mjc400_op_73_mcl },
	{ 0b000111, "MCL", true, mjc400_op_73_mcl },
	{ 0b001000, "CIT", true, mjc400_op_73_cit },
	{ 0b001001, "SIL", true, mjc400_op_73_sil },
	{ 0b001010, "SIU", true, mjc400_op_73_siu },
	{ 0b001011, "SIT", true, mjc400_op_73_sit },
	{ 0b001100, "GIU", true, mjc400_op_73_giu },
	{ 0b001101, "GIU", true, mjc400_op_73_giu },
	{ 0b001110, "GIU", true, mjc400_op_73_giu },
	{ 0b001111, "GIU", true, mjc400_op_73_giu },
	{ 0b010000, "LIP", true, mjc400_op_73_lip },
	{ 0b010001, "LIP", true, mjc400_op_73_lip },
	{ 0b010010, "LIP", true, mjc400_op_73_lip },
	{ 0b010011, "LIP", true, mjc400_op_73_lip },
	{ 0b010100, "-i-", false, mjc400_op_illegal },
	{ 0b010101, "-i-", false, mjc400_op_illegal },
	{ 0b010110, "-i-", false, mjc400_op_illegal },
	{ 0b010111, "-i-", false, mjc400_op_illegal },
	{ 0b011000, "-i-", false, mjc400_op_illegal },
	{ 0b011001, "-i-", false, mjc400_op_illegal },
	{ 0b011010, "-i-", false, mjc400_op_illegal },
	{ 0b011011, "-i-", false, mjc400_op_illegal },
	{ 0b011100, "-i-", false, mjc400_op_illegal },
	{ 0b011101, "-i-", false, mjc400_op_illegal },
	{ 0b011110, "-i-", false, mjc400_op_illegal },
	{ 0b011111, "-i-", false, mjc400_op_illegal },
	{ 0b100000, "HLT", true, mjc400_op_73_hlt },
	{ 0b100001, "HLT", true, mjc400_op_73_hlt },
	{ 0b100010, "HLT", true, mjc400_op_73_hlt },
	{ 0b100011, "HLT", true, mjc400_op_73_hlt },
	{ 0b100100, "MCL", true, mjc400_op_73_mcl },
	{ 0b100101, "MCL", true, mjc400_op_73_mcl },
	{ 0b100110, "MCL", true, mjc400_op_73_mcl },
	{ 0b100111, "MCL", true, mjc400_op_73_mcl },
	{ 0b101000, "CIT", true, mjc400_op_73_cit },
	{ 0b101001, "SIL", true, mjc400_op_73_sil },
	{ 0b101010, "SIU", true, mjc400_op_73_siu },
	{ 0b101011, "SIT", true, mjc400_op_73_sit },
	{ 0b101100, "GIL", true, mjc400_op_73_gil },
	{ 0b101101, "GIL", true, mjc400_op_73_gil },
	{ 0b101110, "GIL", true, mjc400_op_73_gil },
	{ 0b101111, "GIL", true, mjc400_op_73_gil },
	{ 0b110001, "LIP", true, mjc400_op_73_lip },
	{ 0b110000, "LIP", true, mjc400_op_73_lip },
	{ 0b110010, "LIP", true, mjc400_op_73_lip },
	{ 0b110011, "LIP", true, mjc400_op_73_lip },
	{ 0b110100, "-i-", false, mjc400_op_illegal },
	{ 0b110101, "-i-", false, mjc400_op_illegal },
	{ 0b110110, "-i-", false, mjc400_op_illegal },
	{ 0b110111, "-i-", false, mjc400_op_illegal },
	{ 0b111000, "-i-", false, mjc400_op_illegal },
	{ 0b111001, "-i-", false, mjc400_op_illegal },
	{ 0b111010, "-i-", false, mjc400_op_illegal },
	{ 0b111011, "-i-", false, mjc400_op_illegal },
	{ 0b111100, "-i-", false, mjc400_op_illegal },
	{ 0b111101, "-i-", false, mjc400_op_illegal },
	{ 0b111110, "-i-", false, mjc400_op_illegal },
	{ 0b111111, "-i-", false, mjc400_op_illegal },
};

// aaa
// 012
mjc400_opdef mjc400_iset_74[] = {
	{ 0, "UJ", false, mjc400_op_74_uj },
	{ 1, "JL", false, mjc400_op_74_jl },
	{ 2, "JE", false, mjc400_op_74_je },
	{ 3, "JG", false, mjc400_op_74_jg },
	{ 4, "JZ", false, mjc400_op_74_jz },
	{ 5, "JM", false, mjc400_op_74_jm },
	{ 6, "JN", false, mjc400_op_74_jn },
	{ 7, "LJ", false, mjc400_op_74_lj }
};

// aaa
// 012
mjc400_opdef mjc400_iset_75[] = {
	{ 0, "LD", false, mjc400_op_75_ld },
	{ 1, "LF", false, mjc400_op_75_lf },
	{ 2, "LA", false, mjc400_op_75_la },
	{ 3, "LL", false, mjc400_op_75_ll },
	{ 4, "TD", false, mjc400_op_75_td },
	{ 5, "TF", false, mjc400_op_75_tf },
	{ 6, "TA", false, mjc400_op_75_ta },
	{ 7, "TL", false, mjc400_op_75_tl }
};

// aaa
// 012
mjc400_opdef mjc400_iset_76[] = {
	{ 0, "RD", false, mjc400_op_76_rd },
	{ 1, "RF", false, mjc400_op_76_rf },
	{ 2, "RA", false, mjc400_op_76_ra },
	{ 3, "RL", false, mjc400_op_76_rl },
	{ 4, "PD", false, mjc400_op_76_pd },
	{ 5, "PF", false, mjc400_op_76_pf },
	{ 6, "PA", false, mjc400_op_76_pa },
	{ 7, "PL", false, mjc400_op_76_pl }
};

// aaa
// 012
mjc400_opdef mjc400_iset_77[] = {
	{ 0, "MB", true, mjc400_op_77_mb },
	{ 1, "IM", true, mjc400_op_77_im },
	{ 2, "KI", true, mjc400_op_77_ki },
	{ 3, "FI", true, mjc400_op_77_fi },
	{ 4, "SP", true, mjc400_op_77_sp },
	{ 5, "MD", false, mjc400_op_77_md },
	{ 6, "RZ", false, mjc400_op_77_rz },
	{ 7, "IB", false, mjc400_op_77_ib }
};

