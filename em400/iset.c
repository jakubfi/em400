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

struct mjc400_opdef mjc400_iset[] = {
	{ 000, false, mjc400_op_illegal, NULL },
	{ 001, false, mjc400_op_illegal, NULL },
	{ 002, false, mjc400_op_illegal, NULL },
	{ 003, false, mjc400_op_illegal, NULL },
	{ 004, false, mjc400_op_illegal, NULL },
	{ 005, false, mjc400_op_illegal, NULL },
	{ 006, false, mjc400_op_illegal, NULL },
	{ 007, false, mjc400_op_illegal, NULL },
	{ 010, false, mjc400_op_illegal, NULL },
	{ 011, false, mjc400_op_illegal, NULL },
	{ 012, false, mjc400_op_illegal, NULL },
	{ 013, false, mjc400_op_illegal, NULL },
	{ 014, false, mjc400_op_illegal, NULL },
	{ 015, false, mjc400_op_illegal, NULL },
	{ 016, false, mjc400_op_illegal, NULL },
	{ 017, false, mjc400_op_illegal, NULL },
	
	{ 020, false, mjc400_op_lw, NULL },
	{ 021, false, mjc400_op_tw, NULL },
	{ 022, false, mjc400_op_ls, NULL },
	{ 023, false, mjc400_op_ri, NULL },
	{ 024, false, mjc400_op_rw, NULL },
	{ 025, false, mjc400_op_pw, NULL },
	{ 026, false, mjc400_op_rj, NULL },
	{ 027, false, mjc400_op_is, NULL },
	{ 030, false, mjc400_op_bb, NULL },
	{ 031, false, mjc400_op_bm, NULL },
	{ 032, false, mjc400_op_bs, NULL },
	{ 033, false, mjc400_op_bc, NULL },
	{ 034, false, mjc400_op_bn, NULL },
	{ 035, false, mjc400_op_ou, NULL },
	{ 036, false, mjc400_op_in, NULL },

	{ 037, false, mjc400_op_37, mjc400_iset_37 },

	{ 040, false, mjc400_op_aw, NULL },
	{ 041, false, mjc400_op_ac, NULL },
	{ 042, false, mjc400_op_sw, NULL },
	{ 043, false, mjc400_op_cw, NULL },
	{ 044, false, mjc400_op_or, NULL },
	{ 045, false, mjc400_op_om, NULL },
	{ 046, false, mjc400_op_nr, NULL },
	{ 047, false, mjc400_op_nm, NULL },
	{ 050, false, mjc400_op_er, NULL },
	{ 051, false, mjc400_op_em, NULL },
	{ 052, false, mjc400_op_xr, NULL },
	{ 053, false, mjc400_op_xm, NULL },
	{ 054, false, mjc400_op_cl, NULL },
	{ 055, false, mjc400_op_lb, NULL },
	{ 056, false, mjc400_op_rb, NULL },
	{ 057, false, mjc400_op_cb, NULL },

	{ 060, false, mjc400_op_awt, NULL },
	{ 061, false, mjc400_op_trb, NULL },
	{ 062, false, mjc400_op_irb, NULL },
	{ 063, false, mjc400_op_drb, NULL },
	{ 064, false, mjc400_op_cwt, NULL },
	{ 065, false, mjc400_op_lwt, NULL },
	{ 066, false, mjc400_op_lws, NULL },
	{ 067, false, mjc400_op_rws, NULL },

	{ 070, false, mjc400_op_70, mjc400_iset_70 },
	{ 071, false, mjc400_op_71, mjc400_iset_71 },
	{ 072, false, mjc400_op_72, mjc400_iset_72 },
	{ 073, false, mjc400_op_73, mjc400_iset_73 },
	{ 074, false, mjc400_op_74, mjc400_iset_74 },
	{ 075, false, mjc400_op_75, mjc400_iset_75 },
	{ 076, false, mjc400_op_76, mjc400_iset_76 },
	{ 077, false, mjc400_op_77, mjc400_iset_77 }
};

struct mjc400_opdef mjc400_iset_37[] = {
	{ 0, false, mjc400_op_37_ad, NULL },
	{ 1, false, mjc400_op_37_sd, NULL },
	{ 2, false, mjc400_op_37_mw, NULL },
	{ 3, false, mjc400_op_37_dw, NULL },
	{ 4, false, mjc400_op_37_af, NULL },
	{ 5, false, mjc400_op_37_sf, NULL },
	{ 6, false, mjc400_op_37_mf, NULL },
	{ 7, false, mjc400_op_37_df, NULL }
};

struct mjc400_opdef mjc400_iset_70[] = {
	{ 0, false, mjc400_op_70_ujs, NULL},
	{ 1, false, mjc400_op_70_jls, NULL},
	{ 2, false, mjc400_op_70_jes, NULL},
	{ 3, false, mjc400_op_70_jgs, NULL},
	{ 4, false, mjc400_op_70_jvs, NULL},
	{ 5, false, mjc400_op_70_jxs, NULL},
	{ 6, false, mjc400_op_70_jys, NULL},
	{ 7, false, mjc400_op_70_jcs, NULL}
};

struct mjc400_opdef mjc400_iset_71[] = {
	{ 0, false, mjc400_op_71_blc, NULL },
	{ 1, false, mjc400_op_71_exl, NULL },
	{ 2, false, mjc400_op_71_brc, NULL },
	{ 3, false, mjc400_op_71_nrf, NULL }
};

struct mjc400_opdef mjc400_iset_72[] = {
	{ 0b000000, false, mjc400_op_72_ric, NULL },
	{ 0b000001, false, mjc400_op_72_zlb, NULL },
	{ 0b000010, false, mjc400_op_72_sxu, NULL },
	{ 0b000011, false, mjc400_op_72_nga, NULL },
	{ 0b000100, false, mjc400_op_72_slz, NULL },
	{ 0b000101, false, mjc400_op_72_sly, NULL },
	{ 0b000110, false, mjc400_op_72_slx, NULL },
	{ 0b000111, false, mjc400_op_72_sry, NULL },
	{ 0b001000, false, mjc400_op_72_ngl, NULL },
	{ 0b001001, false, mjc400_op_72_rpc, NULL },
	{ 0b001010, false, mjc400_op_illegal, NULL },
	{ 0b001011, false, mjc400_op_illegal, NULL },
	{ 0b001100, false, mjc400_op_illegal, NULL },
	{ 0b001101, false, mjc400_op_illegal, NULL },
	{ 0b001110, false, mjc400_op_illegal, NULL },
	{ 0b001111, false, mjc400_op_illegal, NULL },
	{ 0b010000, false, mjc400_op_72_shc, NULL },
	{ 0b010001, false, mjc400_op_72_shc, NULL },
	{ 0b010010, false, mjc400_op_72_shc, NULL },
	{ 0b010011, false, mjc400_op_72_shc, NULL },
	{ 0b010100, false, mjc400_op_72_shc, NULL },
	{ 0b010101, false, mjc400_op_72_shc, NULL },
	{ 0b010110, false, mjc400_op_72_shc, NULL },
	{ 0b010111, false, mjc400_op_72_shc, NULL },
	{ 0b011000, false, mjc400_op_illegal, NULL },
	{ 0b011001, false, mjc400_op_illegal, NULL },
	{ 0b011010, false, mjc400_op_illegal, NULL },
	{ 0b011011, false, mjc400_op_illegal, NULL },
	{ 0b011100, false, mjc400_op_illegal, NULL },
	{ 0b011101, false, mjc400_op_illegal, NULL },
	{ 0b011110, false, mjc400_op_illegal, NULL },
	{ 0b011111, false, mjc400_op_illegal, NULL },
	{ 0b100000, false, mjc400_op_72_rky, NULL },
	{ 0b100001, false, mjc400_op_72_zrb, NULL },
	{ 0b100010, false, mjc400_op_72_sxl, NULL },
	{ 0b100011, false, mjc400_op_72_ngc, NULL },
	{ 0b100100, false, mjc400_op_72_svz, NULL },
	{ 0b100101, false, mjc400_op_72_svy, NULL },
	{ 0b100110, false, mjc400_op_72_svx, NULL },
	{ 0b100111, false, mjc400_op_72_srx, NULL },
	{ 0b101000, false, mjc400_op_72_srz, NULL },
	{ 0b101001, false, mjc400_op_72_lpc, NULL },
	{ 0b101010, false, mjc400_op_illegal, NULL },
	{ 0b101011, false, mjc400_op_illegal, NULL },
	{ 0b101100, false, mjc400_op_illegal, NULL },
	{ 0b101101, false, mjc400_op_illegal, NULL },
	{ 0b101110, false, mjc400_op_illegal, NULL },
	{ 0b101111, false, mjc400_op_illegal, NULL },
	{ 0b110001, false, mjc400_op_72_shc, NULL },
	{ 0b110000, false, mjc400_op_72_shc, NULL },
	{ 0b110010, false, mjc400_op_72_shc, NULL },
	{ 0b110011, false, mjc400_op_72_shc, NULL },
	{ 0b110100, false, mjc400_op_72_shc, NULL },
	{ 0b110101, false, mjc400_op_72_shc, NULL },
	{ 0b110110, false, mjc400_op_72_shc, NULL },
	{ 0b110111, false, mjc400_op_72_shc, NULL },
	{ 0b111000, false, mjc400_op_illegal, NULL },
	{ 0b111001, false, mjc400_op_illegal, NULL },
	{ 0b111010, false, mjc400_op_illegal, NULL },
	{ 0b111011, false, mjc400_op_illegal, NULL },
	{ 0b111100, false, mjc400_op_illegal, NULL },
	{ 0b111101, false, mjc400_op_illegal, NULL },
	{ 0b111110, false, mjc400_op_illegal, NULL },
	{ 0b111111, false, mjc400_op_illegal, NULL }
};

struct mjc400_opdef mjc400_iset_73[] = {
	{ 0b0000000, true, mjc400_op_73_hlt, NULL },
	{ 0b0000001, true, mjc400_op_73_hlt, NULL },
	{ 0b0000010, true, mjc400_op_73_hlt, NULL },
	{ 0b0000011, true, mjc400_op_73_hlt, NULL },
	{ 0b0000100, true, mjc400_op_73_hlt, NULL },
	{ 0b0000101, true, mjc400_op_73_hlt, NULL },
	{ 0b0000110, true, mjc400_op_73_hlt, NULL },
	{ 0b0000111, true, mjc400_op_73_hlt, NULL },

	{ 0b0001000, true, mjc400_op_73_mcl, NULL },
	{ 0b0001001, true, mjc400_op_73_mcl, NULL },
	{ 0b0001010, true, mjc400_op_73_mcl, NULL },
	{ 0b0001011, true, mjc400_op_73_mcl, NULL },
	{ 0b0001100, true, mjc400_op_73_mcl, NULL },
	{ 0b0001101, true, mjc400_op_73_mcl, NULL },
	{ 0b0001110, true, mjc400_op_73_mcl, NULL },
	{ 0b0001111, true, mjc400_op_73_mcl, NULL },

	{ 0b0010000, true, mjc400_op_73_cit, NULL },
	{ 0b0010001, true, mjc400_op_73_sil, NULL },
	{ 0b0010010, true, mjc400_op_73_siu, NULL },
	{ 0b0010011, true, mjc400_op_73_sit, NULL },
	{ 0b0010100, true, mjc400_op_73_six, NULL },
	{ 0b0010101, false, mjc400_op_illegal, NULL },
	{ 0b0010110, false, mjc400_op_illegal, NULL },
	{ 0b0010111, false, mjc400_op_illegal, NULL },

	{ 0b0011000, true, mjc400_op_73_giu, NULL },
	{ 0b0011001, true, mjc400_op_73_giu, NULL },
	{ 0b0011010, true, mjc400_op_73_giu, NULL },
	{ 0b0011011, true, mjc400_op_73_giu, NULL },
	{ 0b0011100, true, mjc400_op_73_giu, NULL },
	{ 0b0011101, true, mjc400_op_73_giu, NULL },
	{ 0b0011110, true, mjc400_op_73_giu, NULL },
	{ 0b0011111, true, mjc400_op_73_giu, NULL },

	{ 0b0100000, true, mjc400_op_73_lip, NULL },
	{ 0b0100001, true, mjc400_op_73_lip, NULL },
	{ 0b0100010, true, mjc400_op_73_lip, NULL },
	{ 0b0100011, true, mjc400_op_73_lip, NULL },
	{ 0b0100100, true, mjc400_op_73_lip, NULL },
	{ 0b0100101, true, mjc400_op_73_lip, NULL },
	{ 0b0100110, true, mjc400_op_73_lip, NULL },
	{ 0b0100111, true, mjc400_op_73_lip, NULL },

	{ 0b0101000, false, mjc400_op_illegal, NULL },
	{ 0b0101001, false, mjc400_op_illegal, NULL },
	{ 0b0101010, false, mjc400_op_illegal, NULL },
	{ 0b0101011, false, mjc400_op_illegal, NULL },
	{ 0b0101100, false, mjc400_op_illegal, NULL },
	{ 0b0101101, false, mjc400_op_illegal, NULL },
	{ 0b0101110, false, mjc400_op_illegal, NULL },
	{ 0b0101111, false, mjc400_op_illegal, NULL },

	{ 0b0110000, false, mjc400_op_illegal, NULL },
	{ 0b0110001, false, mjc400_op_illegal, NULL },
	{ 0b0110010, false, mjc400_op_illegal, NULL },
	{ 0b0110011, false, mjc400_op_illegal, NULL },
	{ 0b0110100, false, mjc400_op_illegal, NULL },
	{ 0b0110101, false, mjc400_op_illegal, NULL },
	{ 0b0110110, false, mjc400_op_illegal, NULL },
	{ 0b0110111, false, mjc400_op_illegal, NULL },

	{ 0b0111000, false, mjc400_op_illegal, NULL },
	{ 0b0111001, false, mjc400_op_illegal, NULL },
	{ 0b0111010, false, mjc400_op_illegal, NULL },
	{ 0b0111011, false, mjc400_op_illegal, NULL },
	{ 0b0111100, false, mjc400_op_illegal, NULL },
	{ 0b0111101, false, mjc400_op_illegal, NULL },
	{ 0b0111110, false, mjc400_op_illegal, NULL },
	{ 0b0111111, false, mjc400_op_illegal, NULL },

	{ 0b1000000, true, mjc400_op_73_hlt, NULL },
	{ 0b1000001, true, mjc400_op_73_hlt, NULL },
	{ 0b1000010, true, mjc400_op_73_hlt, NULL },
	{ 0b1000011, true, mjc400_op_73_hlt, NULL },
	{ 0b1000100, true, mjc400_op_73_hlt, NULL },
	{ 0b1000101, true, mjc400_op_73_hlt, NULL },
	{ 0b1000110, true, mjc400_op_73_hlt, NULL },
	{ 0b1000111, true, mjc400_op_73_hlt, NULL },

	{ 0b1001000, true, mjc400_op_73_mcl, NULL },
	{ 0b1001001, true, mjc400_op_73_mcl, NULL },
	{ 0b1001010, true, mjc400_op_73_mcl, NULL },
	{ 0b1001011, true, mjc400_op_73_mcl, NULL },
	{ 0b1001100, true, mjc400_op_73_mcl, NULL },
	{ 0b1001101, true, mjc400_op_73_mcl, NULL },
	{ 0b1001110, true, mjc400_op_73_mcl, NULL },
	{ 0b1001111, true, mjc400_op_73_mcl, NULL },

	{ 0b1010000, true, mjc400_op_73_cit, NULL },
	{ 0b1010001, true, mjc400_op_73_sil, NULL },
	{ 0b1010010, true, mjc400_op_73_siu, NULL },
	{ 0b1010011, true, mjc400_op_73_sit, NULL },
	{ 0b1010100, true, mjc400_op_73_cix, NULL },
	{ 0b1010101, false, mjc400_op_illegal, NULL },
	{ 0b1010110, false, mjc400_op_illegal, NULL },
	{ 0b1010111, false, mjc400_op_illegal, NULL },

	{ 0b1011000, true, mjc400_op_73_gil, NULL },
	{ 0b1011001, true, mjc400_op_73_gil, NULL },
	{ 0b1011010, true, mjc400_op_73_gil, NULL },
	{ 0b1011011, true, mjc400_op_73_gil, NULL },
	{ 0b1011100, true, mjc400_op_73_gil, NULL },
	{ 0b1011101, true, mjc400_op_73_gil, NULL },
	{ 0b1011110, true, mjc400_op_73_gil, NULL },
	{ 0b1011111, true, mjc400_op_73_gil, NULL },

	{ 0b1100001, true, mjc400_op_73_lip, NULL },
	{ 0b1100000, true, mjc400_op_73_lip, NULL },
	{ 0b1100010, true, mjc400_op_73_lip, NULL },
	{ 0b1100011, true, mjc400_op_73_lip, NULL },
	{ 0b1100101, true, mjc400_op_73_lip, NULL },
	{ 0b1100100, true, mjc400_op_73_lip, NULL },
	{ 0b1100110, true, mjc400_op_73_lip, NULL },
	{ 0b1100111, true, mjc400_op_73_lip, NULL },

	{ 0b1101000, false, mjc400_op_illegal, NULL},
	{ 0b1101001, false, mjc400_op_illegal, NULL},
	{ 0b1101010, false, mjc400_op_illegal, NULL},
	{ 0b1101011, false, mjc400_op_illegal, NULL},
	{ 0b1101100, false, mjc400_op_illegal, NULL},
	{ 0b1101101, false, mjc400_op_illegal, NULL},
	{ 0b1101110, false, mjc400_op_illegal, NULL},
	{ 0b1101111, false, mjc400_op_illegal, NULL},

	{ 0b1110000, false, mjc400_op_illegal, NULL},
	{ 0b1110001, false, mjc400_op_illegal, NULL},
	{ 0b1110010, false, mjc400_op_illegal, NULL},
	{ 0b1110011, false, mjc400_op_illegal, NULL},
	{ 0b1110100, false, mjc400_op_illegal, NULL},
	{ 0b1110101, false, mjc400_op_illegal, NULL},
	{ 0b1110110, false, mjc400_op_illegal, NULL},
	{ 0b1110111, false, mjc400_op_illegal, NULL},

	{ 0b1111000, false, mjc400_op_illegal, NULL},
	{ 0b1111001, false, mjc400_op_illegal, NULL},
	{ 0b1111010, false, mjc400_op_illegal, NULL},
	{ 0b1111011, false, mjc400_op_illegal, NULL},
	{ 0b1111100, false, mjc400_op_illegal, NULL},
	{ 0b1111101, false, mjc400_op_illegal, NULL},
	{ 0b1111110, false, mjc400_op_illegal, NULL},
	{ 0b1111111, false, mjc400_op_illegal, NULL}

};

struct mjc400_opdef mjc400_iset_74[] = {
	{ 0, false, mjc400_op_74_uj, NULL },
	{ 1, false, mjc400_op_74_jl, NULL },
	{ 2, false, mjc400_op_74_je, NULL },
	{ 3, false, mjc400_op_74_jg, NULL },
	{ 4, false, mjc400_op_74_jz, NULL },
	{ 5, false, mjc400_op_74_jm, NULL },
	{ 6, false, mjc400_op_74_jn, NULL },
	{ 7, false, mjc400_op_74_lj, NULL }
};

struct mjc400_opdef mjc400_iset_75[] = {
	{ 0, false, mjc400_op_75_ld, NULL },
	{ 1, false, mjc400_op_75_lf, NULL },
	{ 2, false, mjc400_op_75_la, NULL },
	{ 3, false, mjc400_op_75_ll, NULL },
	{ 4, false, mjc400_op_75_td, NULL },
	{ 5, false, mjc400_op_75_tf, NULL },
	{ 6, false, mjc400_op_75_ta, NULL },
	{ 7, false, mjc400_op_75_tl, NULL }
};

struct mjc400_opdef mjc400_iset_76[] = {
	{ 0, false, mjc400_op_76_rd, NULL },
	{ 1, false, mjc400_op_76_rf, NULL },
	{ 2, false, mjc400_op_76_ra, NULL },
	{ 3, false, mjc400_op_76_rl, NULL },
	{ 4, false, mjc400_op_76_pd, NULL },
	{ 5, false, mjc400_op_76_pf, NULL },
	{ 6, false, mjc400_op_76_pa, NULL },
	{ 7, false, mjc400_op_76_pl, NULL }
};

struct mjc400_opdef mjc400_iset_77[] = {
	{ 0, true,  mjc400_op_77_mb, NULL },
	{ 1, true,  mjc400_op_77_im, NULL },
	{ 2, true,  mjc400_op_77_ki, NULL },
	{ 3, true,  mjc400_op_77_fi, NULL },
	{ 4, true,  mjc400_op_77_sp, NULL },
	{ 5, false, mjc400_op_77_md, NULL },
	{ 6, false, mjc400_op_77_rz, NULL },
	{ 7, false, mjc400_op_77_ib, NULL }
};

// vim: tabstop=4
