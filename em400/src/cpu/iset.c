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
#include <stdbool.h>
#include <inttypes.h>

#include "cpu/iset.h"
#include "cpu/instructions.h"

struct opdef iset[] = {
	{ 000, false, op_illegal, NULL },
	{ 001, false, op_illegal, NULL },
	{ 002, false, op_illegal, NULL },
	{ 003, false, op_illegal, NULL },
	{ 004, false, op_illegal, NULL },
	{ 005, false, op_illegal, NULL },
	{ 006, false, op_illegal, NULL },
	{ 007, false, op_illegal, NULL },
	{ 010, false, op_illegal, NULL },
	{ 011, false, op_illegal, NULL },
	{ 012, false, op_illegal, NULL },
	{ 013, false, op_illegal, NULL },
	{ 014, false, op_illegal, NULL },
	{ 015, false, op_illegal, NULL },
	{ 016, false, op_illegal, NULL },
	{ 017, false, op_illegal, NULL },
	
	{ 020, false, op_lw, NULL },
	{ 021, false, op_tw, NULL },
	{ 022, false, op_ls, NULL },
	{ 023, false, op_ri, NULL },
	{ 024, false, op_rw, NULL },
	{ 025, false, op_pw, NULL },
	{ 026, false, op_rj, NULL },
	{ 027, false, op_is, NULL },
	{ 030, false, op_bb, NULL },
	{ 031, false, op_bm, NULL },
	{ 032, false, op_bs, NULL },
	{ 033, false, op_bc, NULL },
	{ 034, false, op_bn, NULL },
	{ 035, false, op_ou, NULL },
	{ 036, false, op_in, NULL },

	{ 037, false, op_37, iset_37 },

	{ 040, false, op_aw, NULL },
	{ 041, false, op_ac, NULL },
	{ 042, false, op_sw, NULL },
	{ 043, false, op_cw, NULL },
	{ 044, false, op_or, NULL },
	{ 045, false, op_om, NULL },
	{ 046, false, op_nr, NULL },
	{ 047, false, op_nm, NULL },
	{ 050, false, op_er, NULL },
	{ 051, false, op_em, NULL },
	{ 052, false, op_xr, NULL },
	{ 053, false, op_xm, NULL },
	{ 054, false, op_cl, NULL },
	{ 055, false, op_lb, NULL },
	{ 056, false, op_rb, NULL },
	{ 057, false, op_cb, NULL },

	{ 060, false, op_awt, NULL },
	{ 061, false, op_trb, NULL },
	{ 062, false, op_irb, NULL },
	{ 063, false, op_drb, NULL },
	{ 064, false, op_cwt, NULL },
	{ 065, false, op_lwt, NULL },
	{ 066, false, op_lws, NULL },
	{ 067, false, op_rws, NULL },

	{ 070, false, op_70, iset_70 },
	{ 071, false, op_71, iset_71 },
	{ 072, false, op_72, iset_72 },
	{ 073, false, op_73, iset_73 },
	{ 074, false, op_74, iset_74 },
	{ 075, false, op_75, iset_75 },
	{ 076, false, op_76, iset_76 },
	{ 077, false, op_77, iset_77 }
};

struct opdef iset_37[] = {
	{ 0, false, op_37_ad, NULL },
	{ 1, false, op_37_sd, NULL },
	{ 2, false, op_37_mw, NULL },
	{ 3, false, op_37_dw, NULL },
	{ 4, false, op_37_af, NULL },
	{ 5, false, op_37_sf, NULL },
	{ 6, false, op_37_mf, NULL },
	{ 7, false, op_37_df, NULL }
};

struct opdef iset_70[] = {
	{ 0, false, op_70_ujs, NULL},
	{ 1, false, op_70_jls, NULL},
	{ 2, false, op_70_jes, NULL},
	{ 3, false, op_70_jgs, NULL},
	{ 4, false, op_70_jvs, NULL},
	{ 5, false, op_70_jxs, NULL},
	{ 6, false, op_70_jys, NULL},
	{ 7, false, op_70_jcs, NULL}
};

struct opdef iset_71[] = {
	{ 0, false, op_71_blc, NULL },
	{ 1, false, op_71_exl, NULL },
	{ 2, false, op_71_brc, NULL },
	{ 3, false, op_71_nrf, NULL }
};

struct opdef iset_72[] = {
	{ 0b000000, false, op_72_ric, NULL },
	{ 0b000001, false, op_72_zlb, NULL },
	{ 0b000010, false, op_72_sxu, NULL },
	{ 0b000011, false, op_72_nga, NULL },
	{ 0b000100, false, op_72_slz, NULL },
	{ 0b000101, false, op_72_sly, NULL },
	{ 0b000110, false, op_72_slx, NULL },
	{ 0b000111, false, op_72_sry, NULL },
	{ 0b001000, false, op_72_ngl, NULL },
	{ 0b001001, false, op_72_rpc, NULL },
	{ 0b001010, false, op_illegal, NULL },
	{ 0b001011, false, op_illegal, NULL },
	{ 0b001100, false, op_illegal, NULL },
	{ 0b001101, false, op_illegal, NULL },
	{ 0b001110, false, op_illegal, NULL },
	{ 0b001111, false, op_illegal, NULL },
	{ 0b010000, false, op_72_shc, NULL },
	{ 0b010001, false, op_72_shc, NULL },
	{ 0b010010, false, op_72_shc, NULL },
	{ 0b010011, false, op_72_shc, NULL },
	{ 0b010100, false, op_72_shc, NULL },
	{ 0b010101, false, op_72_shc, NULL },
	{ 0b010110, false, op_72_shc, NULL },
	{ 0b010111, false, op_72_shc, NULL },
	{ 0b011000, false, op_illegal, NULL },
	{ 0b011001, false, op_illegal, NULL },
	{ 0b011010, false, op_illegal, NULL },
	{ 0b011011, false, op_illegal, NULL },
	{ 0b011100, false, op_illegal, NULL },
	{ 0b011101, false, op_illegal, NULL },
	{ 0b011110, false, op_illegal, NULL },
	{ 0b011111, false, op_illegal, NULL },
	{ 0b100000, false, op_72_rky, NULL },
	{ 0b100001, false, op_72_zrb, NULL },
	{ 0b100010, false, op_72_sxl, NULL },
	{ 0b100011, false, op_72_ngc, NULL },
	{ 0b100100, false, op_72_svz, NULL },
	{ 0b100101, false, op_72_svy, NULL },
	{ 0b100110, false, op_72_svx, NULL },
	{ 0b100111, false, op_72_srx, NULL },
	{ 0b101000, false, op_72_srz, NULL },
	{ 0b101001, false, op_72_lpc, NULL },
	{ 0b101010, false, op_illegal, NULL },
	{ 0b101011, false, op_illegal, NULL },
	{ 0b101100, false, op_illegal, NULL },
	{ 0b101101, false, op_illegal, NULL },
	{ 0b101110, false, op_illegal, NULL },
	{ 0b101111, false, op_illegal, NULL },
	{ 0b110001, false, op_72_shc, NULL },
	{ 0b110000, false, op_72_shc, NULL },
	{ 0b110010, false, op_72_shc, NULL },
	{ 0b110011, false, op_72_shc, NULL },
	{ 0b110100, false, op_72_shc, NULL },
	{ 0b110101, false, op_72_shc, NULL },
	{ 0b110110, false, op_72_shc, NULL },
	{ 0b110111, false, op_72_shc, NULL },
	{ 0b111000, false, op_illegal, NULL },
	{ 0b111001, false, op_illegal, NULL },
	{ 0b111010, false, op_illegal, NULL },
	{ 0b111011, false, op_illegal, NULL },
	{ 0b111100, false, op_illegal, NULL },
	{ 0b111101, false, op_illegal, NULL },
	{ 0b111110, false, op_illegal, NULL },
	{ 0b111111, false, op_illegal, NULL }
};

struct opdef iset_73[] = {
	{ 0b0000000, true, op_73_hlt, NULL },
	{ 0b0000001, true, op_73_hlt, NULL },
	{ 0b0000010, true, op_73_hlt, NULL },
	{ 0b0000011, true, op_73_hlt, NULL },
	{ 0b0000100, true, op_73_hlt, NULL },
	{ 0b0000101, true, op_73_hlt, NULL },
	{ 0b0000110, true, op_73_hlt, NULL },
	{ 0b0000111, true, op_73_hlt, NULL },

	{ 0b0001000, true, op_73_mcl, NULL },
	{ 0b0001001, true, op_73_mcl, NULL },
	{ 0b0001010, true, op_73_mcl, NULL },
	{ 0b0001011, true, op_73_mcl, NULL },
	{ 0b0001100, true, op_73_mcl, NULL },
	{ 0b0001101, true, op_73_mcl, NULL },
	{ 0b0001110, true, op_73_mcl, NULL },
	{ 0b0001111, true, op_73_mcl, NULL },

	{ 0b0010000, true, op_73_cit, NULL },
	{ 0b0010001, true, op_73_sil, NULL },
	{ 0b0010010, true, op_73_siu, NULL },
	{ 0b0010011, true, op_73_sit, NULL },
	{ 0b0010100, true, op_73_sint, NULL },
	{ 0b0010101, false, op_illegal, NULL },
	{ 0b0010110, false, op_illegal, NULL },
	{ 0b0010111, false, op_illegal, NULL },

	{ 0b0011000, true, op_73_giu, NULL },
	{ 0b0011001, true, op_73_giu, NULL },
	{ 0b0011010, true, op_73_giu, NULL },
	{ 0b0011011, true, op_73_giu, NULL },
	{ 0b0011100, true, op_73_giu, NULL },
	{ 0b0011101, true, op_73_giu, NULL },
	{ 0b0011110, true, op_73_giu, NULL },
	{ 0b0011111, true, op_73_giu, NULL },

	{ 0b0100000, true, op_73_lip, NULL },
	{ 0b0100001, true, op_73_lip, NULL },
	{ 0b0100010, true, op_73_lip, NULL },
	{ 0b0100011, true, op_73_lip, NULL },
	{ 0b0100100, true, op_73_lip, NULL },
	{ 0b0100101, true, op_73_lip, NULL },
	{ 0b0100110, true, op_73_lip, NULL },
	{ 0b0100111, true, op_73_lip, NULL },

	{ 0b0101000, false, op_illegal, NULL },
	{ 0b0101001, false, op_illegal, NULL },
	{ 0b0101010, false, op_illegal, NULL },
	{ 0b0101011, false, op_illegal, NULL },
	{ 0b0101100, false, op_illegal, NULL },
	{ 0b0101101, false, op_illegal, NULL },
	{ 0b0101110, false, op_illegal, NULL },
	{ 0b0101111, false, op_illegal, NULL },

	{ 0b0110000, false, op_illegal, NULL },
	{ 0b0110001, false, op_illegal, NULL },
	{ 0b0110010, false, op_illegal, NULL },
	{ 0b0110011, false, op_illegal, NULL },
	{ 0b0110100, false, op_illegal, NULL },
	{ 0b0110101, false, op_illegal, NULL },
	{ 0b0110110, false, op_illegal, NULL },
	{ 0b0110111, false, op_illegal, NULL },

	{ 0b0111000, false, op_illegal, NULL },
	{ 0b0111001, false, op_illegal, NULL },
	{ 0b0111010, false, op_illegal, NULL },
	{ 0b0111011, false, op_illegal, NULL },
	{ 0b0111100, false, op_illegal, NULL },
	{ 0b0111101, false, op_illegal, NULL },
	{ 0b0111110, false, op_illegal, NULL },
	{ 0b0111111, false, op_illegal, NULL },

	{ 0b1000000, true, op_73_hlt, NULL },
	{ 0b1000001, true, op_73_hlt, NULL },
	{ 0b1000010, true, op_73_hlt, NULL },
	{ 0b1000011, true, op_73_hlt, NULL },
	{ 0b1000100, true, op_73_hlt, NULL },
	{ 0b1000101, true, op_73_hlt, NULL },
	{ 0b1000110, true, op_73_hlt, NULL },
	{ 0b1000111, true, op_73_hlt, NULL },

	{ 0b1001000, true, op_73_mcl, NULL },
	{ 0b1001001, true, op_73_mcl, NULL },
	{ 0b1001010, true, op_73_mcl, NULL },
	{ 0b1001011, true, op_73_mcl, NULL },
	{ 0b1001100, true, op_73_mcl, NULL },
	{ 0b1001101, true, op_73_mcl, NULL },
	{ 0b1001110, true, op_73_mcl, NULL },
	{ 0b1001111, true, op_73_mcl, NULL },

	{ 0b1010000, true, op_73_cit, NULL },
	{ 0b1010001, true, op_73_sil, NULL },
	{ 0b1010010, true, op_73_siu, NULL },
	{ 0b1010011, true, op_73_sit, NULL },
	{ 0b1010100, true, op_73_sind, NULL },
	{ 0b1010101, false, op_illegal, NULL },
	{ 0b1010110, false, op_illegal, NULL },
	{ 0b1010111, false, op_illegal, NULL },

	{ 0b1011000, true, op_73_gil, NULL },
	{ 0b1011001, true, op_73_gil, NULL },
	{ 0b1011010, true, op_73_gil, NULL },
	{ 0b1011011, true, op_73_gil, NULL },
	{ 0b1011100, true, op_73_gil, NULL },
	{ 0b1011101, true, op_73_gil, NULL },
	{ 0b1011110, true, op_73_gil, NULL },
	{ 0b1011111, true, op_73_gil, NULL },

	{ 0b1100001, true, op_73_lip, NULL },
	{ 0b1100000, true, op_73_lip, NULL },
	{ 0b1100010, true, op_73_lip, NULL },
	{ 0b1100011, true, op_73_lip, NULL },
	{ 0b1100101, true, op_73_lip, NULL },
	{ 0b1100100, true, op_73_lip, NULL },
	{ 0b1100110, true, op_73_lip, NULL },
	{ 0b1100111, true, op_73_lip, NULL },

	{ 0b1101000, false, op_illegal, NULL},
	{ 0b1101001, false, op_illegal, NULL},
	{ 0b1101010, false, op_illegal, NULL},
	{ 0b1101011, false, op_illegal, NULL},
	{ 0b1101100, false, op_illegal, NULL},
	{ 0b1101101, false, op_illegal, NULL},
	{ 0b1101110, false, op_illegal, NULL},
	{ 0b1101111, false, op_illegal, NULL},

	{ 0b1110000, false, op_illegal, NULL},
	{ 0b1110001, false, op_illegal, NULL},
	{ 0b1110010, false, op_illegal, NULL},
	{ 0b1110011, false, op_illegal, NULL},
	{ 0b1110100, false, op_illegal, NULL},
	{ 0b1110101, false, op_illegal, NULL},
	{ 0b1110110, false, op_illegal, NULL},
	{ 0b1110111, false, op_illegal, NULL},

	{ 0b1111000, false, op_illegal, NULL},
	{ 0b1111001, false, op_illegal, NULL},
	{ 0b1111010, false, op_illegal, NULL},
	{ 0b1111011, false, op_illegal, NULL},
	{ 0b1111100, false, op_illegal, NULL},
	{ 0b1111101, false, op_illegal, NULL},
	{ 0b1111110, false, op_illegal, NULL},
	{ 0b1111111, false, op_illegal, NULL}

};

struct opdef iset_74[] = {
	{ 0, false, op_74_uj, NULL },
	{ 1, false, op_74_jl, NULL },
	{ 2, false, op_74_je, NULL },
	{ 3, false, op_74_jg, NULL },
	{ 4, false, op_74_jz, NULL },
	{ 5, false, op_74_jm, NULL },
	{ 6, false, op_74_jn, NULL },
	{ 7, false, op_74_lj, NULL }
};

struct opdef iset_75[] = {
	{ 0, false, op_75_ld, NULL },
	{ 1, false, op_75_lf, NULL },
	{ 2, false, op_75_la, NULL },
	{ 3, false, op_75_ll, NULL },
	{ 4, false, op_75_td, NULL },
	{ 5, false, op_75_tf, NULL },
	{ 6, false, op_75_ta, NULL },
	{ 7, false, op_75_tl, NULL }
};

struct opdef iset_76[] = {
	{ 0, false, op_76_rd, NULL },
	{ 1, false, op_76_rf, NULL },
	{ 2, false, op_76_ra, NULL },
	{ 3, false, op_76_rl, NULL },
	{ 4, false, op_76_pd, NULL },
	{ 5, false, op_76_pf, NULL },
	{ 6, false, op_76_pa, NULL },
	{ 7, false, op_76_pl, NULL }
};

struct opdef iset_77[] = {
	{ 0, true,  op_77_mb, NULL },
	{ 1, true,  op_77_im, NULL },
	{ 2, true,  op_77_ki, NULL },
	{ 3, true,  op_77_fi, NULL },
	{ 4, true,  op_77_sp, NULL },
	{ 5, false, op_77_md, NULL },
	{ 6, false, op_77_rz, NULL },
	{ 7, false, op_77_ib, NULL }
};

// vim: tabstop=4 shiftwidth=4 autoindent
