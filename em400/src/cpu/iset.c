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
	{ 000, false, false, false, NULL, NULL },
	{ 001, false, false, false, NULL, NULL },
	{ 002, false, false, false, NULL, NULL },
	{ 003, false, false, false, NULL, NULL },
	{ 004, false, false, false, NULL, NULL },
	{ 005, false, false, false, NULL, NULL },
	{ 006, false, false, false, NULL, NULL },
	{ 007, false, false, false, NULL, NULL },
	{ 010, false, false, false, NULL, NULL },
	{ 011, false, false, false, NULL, NULL },
	{ 012, false, false, false, NULL, NULL },
	{ 013, false, false, false, NULL, NULL },
	{ 014, false, false, false, NULL, NULL },
	{ 015, false, false, false, NULL, NULL },
	{ 016, false, false, false, NULL, NULL },
	{ 017, false, false, false, NULL, NULL },
	
	{ 020, true, false, false, op_lw, NULL },
	{ 021, true, false, false, op_tw, NULL },
	{ 022, true, false, false, op_ls, NULL },
	{ 023, true, false, false, op_ri, NULL },
	{ 024, true, false, false, op_rw, NULL },
	{ 025, true, false, false, op_pw, NULL },
	{ 026, true, false, false, op_rj, NULL },
	{ 027, true, false, false, op_is, NULL },
	{ 030, true, false, false, op_bb, NULL },
	{ 031, true, false, false, op_bm, NULL },
	{ 032, true, false, false, op_bs, NULL },
	{ 033, true, false, false, op_bc, NULL },
	{ 034, true, false, false, op_bn, NULL },
	{ 035, true, false, false, op_ou, NULL },
	{ 036, true, false, false, op_in, NULL },

	{ 037, true, false, false, op_37, iset_37 },

	{ 040, true, false, false, op_aw, NULL },
	{ 041, true, false, false, op_ac, NULL },
	{ 042, true, false, false, op_sw, NULL },
	{ 043, true, false, false, op_cw, NULL },
	{ 044, true, false, false, op_or, NULL },
	{ 045, true, false, false, op_om, NULL },
	{ 046, true, false, false, op_nr, NULL },
	{ 047, true, false, false, op_nm, NULL },
	{ 050, true, false, false, op_er, NULL },
	{ 051, true, false, false, op_em, NULL },
	{ 052, true, false, false, op_xr, NULL },
	{ 053, true, false, false, op_xm, NULL },
	{ 054, true, false, false, op_cl, NULL },
	{ 055, true, false, false, op_lb, NULL },
	{ 056, true, false, false, op_rb, NULL },
	{ 057, true, false, false, op_cb, NULL },

	{ 060, false, true, false, op_awt, NULL },
	{ 061, false, true, false, op_trb, NULL },
	{ 062, false, true, false, op_irb, NULL },
	{ 063, false, true, false, op_drb, NULL },
	{ 064, false, true, false, op_cwt, NULL },
	{ 065, false, true, false, op_lwt, NULL },
	{ 066, false, true, false, op_lws, NULL },
	{ 067, false, true, false, op_rws, NULL },

	{ 070, false, true,  false, op_70, iset_70 },
	{ 071, false, false, false, op_71, iset_71 },
	{ 072, false, false, false, op_72, iset_72 },
	{ 073, false, false, true,  op_73, iset_73 },
	{ 074, true,  false, false, op_74, iset_74 },
	{ 075, true,  false, false, op_75, iset_75 },
	{ 076, true,  false, false, op_76, iset_76 },
	{ 077, true,  false, false, op_77, iset_77 }
};

struct opdef iset_37[] = {
	{ 0, true, false, false, op_37_ad, NULL },
	{ 1, true, false, false, op_37_sd, NULL },
	{ 2, true, false, false, op_37_mw, NULL },
	{ 3, true, false, false, op_37_dw, NULL },
	{ 4, true, false, false, op_37_af, NULL },
	{ 5, true, false, false, op_37_sf, NULL },
	{ 6, true, false, false, op_37_mf, NULL },
	{ 7, true, false, false, op_37_df, NULL }
};

struct opdef iset_70[] = {
	{ 0, false, true, false, op_70_ujs, NULL},
	{ 1, false, true, false, op_70_jls, NULL},
	{ 2, false, true, false, op_70_jes, NULL},
	{ 3, false, true, false, op_70_jgs, NULL},
	{ 4, false, true, false, op_70_jvs, NULL},
	{ 5, false, true, false, op_70_jxs, NULL},
	{ 6, false, true, false, op_70_jys, NULL},
	{ 7, false, true, false, op_70_jcs, NULL}
};

struct opdef iset_71[] = {
	{ 0, false, false, false, op_71_blc, NULL },
	{ 1, false, false, false, op_71_exl, NULL },
	{ 2, false, false, false, op_71_brc, NULL },
	{ 3, false, false, false, op_71_nrf, NULL }
};

struct opdef iset_72[] = {
	{ 0b000000, false, false, false, op_72_ric, NULL },
	{ 0b000001, false, false, false, op_72_zlb, NULL },
	{ 0b000010, false, false, false, op_72_sxu, NULL },
	{ 0b000011, false, false, false, op_72_nga, NULL },
	{ 0b000100, false, false, false, op_72_slz, NULL },
	{ 0b000101, false, false, false, op_72_sly, NULL },
	{ 0b000110, false, false, false, op_72_slx, NULL },
	{ 0b000111, false, false, false, op_72_sry, NULL },
	{ 0b001000, false, false, false, op_72_ngl, NULL },
	{ 0b001001, false, false, false, op_72_rpc, NULL },
	{ 0b001010, false, false, false, NULL, NULL },
	{ 0b001011, false, false, false, NULL, NULL },
	{ 0b001100, false, false, false, NULL, NULL },
	{ 0b001101, false, false, false, NULL, NULL },
	{ 0b001110, false, false, false, NULL, NULL },
	{ 0b001111, false, false, false, NULL, NULL },
	{ 0b010000, false, false, false, op_72_shc, NULL },
	{ 0b010001, false, false, false, op_72_shc, NULL },
	{ 0b010010, false, false, false, op_72_shc, NULL },
	{ 0b010011, false, false, false, op_72_shc, NULL },
	{ 0b010100, false, false, false, op_72_shc, NULL },
	{ 0b010101, false, false, false, op_72_shc, NULL },
	{ 0b010110, false, false, false, op_72_shc, NULL },
	{ 0b010111, false, false, false, op_72_shc, NULL },
	{ 0b011000, false, false, false, NULL, NULL },
	{ 0b011001, false, false, false, NULL, NULL },
	{ 0b011010, false, false, false, NULL, NULL },
	{ 0b011011, false, false, false, NULL, NULL },
	{ 0b011100, false, false, false, NULL, NULL },
	{ 0b011101, false, false, false, NULL, NULL },
	{ 0b011110, false, false, false, NULL, NULL },
	{ 0b011111, false, false, false, NULL, NULL },
	{ 0b100000, false, false, false, op_72_rky, NULL },
	{ 0b100001, false, false, false, op_72_zrb, NULL },
	{ 0b100010, false, false, false, op_72_sxl, NULL },
	{ 0b100011, false, false, false, op_72_ngc, NULL },
	{ 0b100100, false, false, false, op_72_svz, NULL },
	{ 0b100101, false, false, false, op_72_svy, NULL },
	{ 0b100110, false, false, false, op_72_svx, NULL },
	{ 0b100111, false, false, false, op_72_srx, NULL },
	{ 0b101000, false, false, false, op_72_srz, NULL },
	{ 0b101001, false, false, false, op_72_lpc, NULL },
	{ 0b101010, false, false, false, NULL, NULL },
	{ 0b101011, false, false, false, NULL, NULL },
	{ 0b101100, false, false, false, NULL, NULL },
	{ 0b101101, false, false, false, NULL, NULL },
	{ 0b101110, false, false, false, NULL, NULL },
	{ 0b101111, false, false, false, NULL, NULL },
	{ 0b110001, false, false, false, op_72_shc, NULL },
	{ 0b110000, false, false, false, op_72_shc, NULL },
	{ 0b110010, false, false, false, op_72_shc, NULL },
	{ 0b110011, false, false, false, op_72_shc, NULL },
	{ 0b110100, false, false, false, op_72_shc, NULL },
	{ 0b110101, false, false, false, op_72_shc, NULL },
	{ 0b110110, false, false, false, op_72_shc, NULL },
	{ 0b110111, false, false, false, op_72_shc, NULL },
	{ 0b111000, false, false, false, NULL, NULL },
	{ 0b111001, false, false, false, NULL, NULL },
	{ 0b111010, false, false, false, NULL, NULL },
	{ 0b111011, false, false, false, NULL, NULL },
	{ 0b111100, false, false, false, NULL, NULL },
	{ 0b111101, false, false, false, NULL, NULL },
	{ 0b111110, false, false, false, NULL, NULL },
	{ 0b111111, false, false, false, NULL, NULL }
};

struct opdef iset_73[] = {
	{ 0b0000000, false, true, true, op_73_hlt, NULL },
	{ 0b0000001, false, true, true, op_73_hlt, NULL },
	{ 0b0000010, false, true, true, op_73_hlt, NULL },
	{ 0b0000011, false, true, true, op_73_hlt, NULL },
	{ 0b0000100, false, true, true, op_73_hlt, NULL },
	{ 0b0000101, false, true, true, op_73_hlt, NULL },
	{ 0b0000110, false, true, true, op_73_hlt, NULL },
	{ 0b0000111, false, true, true, op_73_hlt, NULL },

	{ 0b0001000, false, false, true, op_73_mcl, NULL },
	{ 0b0001001, false, false, true, op_73_mcl, NULL },
	{ 0b0001010, false, false, true, op_73_mcl, NULL },
	{ 0b0001011, false, false, true, op_73_mcl, NULL },
	{ 0b0001100, false, false, true, op_73_mcl, NULL },
	{ 0b0001101, false, false, true, op_73_mcl, NULL },
	{ 0b0001110, false, false, true, op_73_mcl, NULL },
	{ 0b0001111, false, false, true, op_73_mcl, NULL },

	{ 0b0010000, false, false, true, op_73_cit, NULL },
	{ 0b0010001, false, false, true, op_73_sil, NULL },
	{ 0b0010010, false, false, true, op_73_siu, NULL },
	{ 0b0010011, false, false, true, op_73_sit, NULL },
	{ 0b0010100, false, false, true, op_73_sint, NULL },
	{ 0b0010101, false, false, false, NULL, NULL },
	{ 0b0010110, false, false, false, NULL, NULL },
	{ 0b0010111, false, false, false, NULL, NULL },

	{ 0b0011000, false, false, true, op_73_giu, NULL },
	{ 0b0011001, false, false, true, op_73_giu, NULL },
	{ 0b0011010, false, false, true, op_73_giu, NULL },
	{ 0b0011011, false, false, true, op_73_giu, NULL },
	{ 0b0011100, false, false, true, op_73_giu, NULL },
	{ 0b0011101, false, false, true, op_73_giu, NULL },
	{ 0b0011110, false, false, true, op_73_giu, NULL },
	{ 0b0011111, false, false, true, op_73_giu, NULL },

	{ 0b0100000, false, false, true, op_73_lip, NULL },
	{ 0b0100001, false, false, true, op_73_lip, NULL },
	{ 0b0100010, false, false, true, op_73_lip, NULL },
	{ 0b0100011, false, false, true, op_73_lip, NULL },
	{ 0b0100100, false, false, true, op_73_lip, NULL },
	{ 0b0100101, false, false, true, op_73_lip, NULL },
	{ 0b0100110, false, false, true, op_73_lip, NULL },
	{ 0b0100111, false, false, true, op_73_lip, NULL },

	{ 0b0101000, false, false, false, NULL, NULL },
	{ 0b0101001, false, false, false, NULL, NULL },
	{ 0b0101010, false, false, false, NULL, NULL },
	{ 0b0101011, false, false, false, NULL, NULL },
	{ 0b0101100, false, false, false, NULL, NULL },
	{ 0b0101101, false, false, false, NULL, NULL },
	{ 0b0101110, false, false, false, NULL, NULL },
	{ 0b0101111, false, false, false, NULL, NULL },

	{ 0b0110000, false, false, false, NULL, NULL },
	{ 0b0110001, false, false, false, NULL, NULL },
	{ 0b0110010, false, false, false, NULL, NULL },
	{ 0b0110011, false, false, false, NULL, NULL },
	{ 0b0110100, false, false, false, NULL, NULL },
	{ 0b0110101, false, false, false, NULL, NULL },
	{ 0b0110110, false, false, false, NULL, NULL },
	{ 0b0110111, false, false, false, NULL, NULL },

	{ 0b0111000, false, false, false, NULL, NULL },
	{ 0b0111001, false, false, false, NULL, NULL },
	{ 0b0111010, false, false, false, NULL, NULL },
	{ 0b0111011, false, false, false, NULL, NULL },
	{ 0b0111100, false, false, false, NULL, NULL },
	{ 0b0111101, false, false, false, NULL, NULL },
	{ 0b0111110, false, false, false, NULL, NULL },
	{ 0b0111111, false, false, false, NULL, NULL },

	{ 0b1000000, false, true, true, op_73_hlt, NULL },
	{ 0b1000001, false, true, true, op_73_hlt, NULL },
	{ 0b1000010, false, true, true, op_73_hlt, NULL },
	{ 0b1000011, false, true, true, op_73_hlt, NULL },
	{ 0b1000100, false, true, true, op_73_hlt, NULL },
	{ 0b1000101, false, true, true, op_73_hlt, NULL },
	{ 0b1000110, false, true, true, op_73_hlt, NULL },
	{ 0b1000111, false, true, true, op_73_hlt, NULL },

	{ 0b1001000, false, false, true, op_73_mcl, NULL },
	{ 0b1001001, false, false, true, op_73_mcl, NULL },
	{ 0b1001010, false, false, true, op_73_mcl, NULL },
	{ 0b1001011, false, false, true, op_73_mcl, NULL },
	{ 0b1001100, false, false, true, op_73_mcl, NULL },
	{ 0b1001101, false, false, true, op_73_mcl, NULL },
	{ 0b1001110, false, false, true, op_73_mcl, NULL },
	{ 0b1001111, false, false, true, op_73_mcl, NULL },

	{ 0b1010000, false, false, true, op_73_cit, NULL },
	{ 0b1010001, false, false, true, op_73_sil, NULL },
	{ 0b1010010, false, false, true, op_73_siu, NULL },
	{ 0b1010011, false, false, true, op_73_sit, NULL },
	{ 0b1010100, false, false, true, op_73_sind, NULL },
	{ 0b1010101, false, false, false, NULL, NULL },
	{ 0b1010110, false, false, false, NULL, NULL },
	{ 0b1010111, false, false, false, NULL, NULL },

	{ 0b1011000, false, false, true, op_73_gil, NULL },
	{ 0b1011001, false, false, true, op_73_gil, NULL },
	{ 0b1011010, false, false, true, op_73_gil, NULL },
	{ 0b1011011, false, false, true, op_73_gil, NULL },
	{ 0b1011100, false, false, true, op_73_gil, NULL },
	{ 0b1011101, false, false, true, op_73_gil, NULL },
	{ 0b1011110, false, false, true, op_73_gil, NULL },
	{ 0b1011111, false, false, true, op_73_gil, NULL },

	{ 0b1100001, false, false, true, op_73_lip, NULL },
	{ 0b1100000, false, false, true, op_73_lip, NULL },
	{ 0b1100010, false, false, true, op_73_lip, NULL },
	{ 0b1100011, false, false, true, op_73_lip, NULL },
	{ 0b1100101, false, false, true, op_73_lip, NULL },
	{ 0b1100100, false, false, true, op_73_lip, NULL },
	{ 0b1100110, false, false, true, op_73_lip, NULL },
	{ 0b1100111, false, false, true, op_73_lip, NULL },

	{ 0b1101000, false, false, false, NULL, NULL},
	{ 0b1101001, false, false, false, NULL, NULL},
	{ 0b1101010, false, false, false, NULL, NULL},
	{ 0b1101011, false, false, false, NULL, NULL},
	{ 0b1101100, false, false, false, NULL, NULL},
	{ 0b1101101, false, false, false, NULL, NULL},
	{ 0b1101110, false, false, false, NULL, NULL},
	{ 0b1101111, false, false, false, NULL, NULL},

	{ 0b1110000, false, false, false, NULL, NULL},
	{ 0b1110001, false, false, false, NULL, NULL},
	{ 0b1110010, false, false, false, NULL, NULL},
	{ 0b1110011, false, false, false, NULL, NULL},
	{ 0b1110100, false, false, false, NULL, NULL},
	{ 0b1110101, false, false, false, NULL, NULL},
	{ 0b1110110, false, false, false, NULL, NULL},
	{ 0b1110111, false, false, false, NULL, NULL},

	{ 0b1111000, false, false, false, NULL, NULL},
	{ 0b1111001, false, false, false, NULL, NULL},
	{ 0b1111010, false, false, false, NULL, NULL},
	{ 0b1111011, false, false, false, NULL, NULL},
	{ 0b1111100, false, false, false, NULL, NULL},
	{ 0b1111101, false, false, false, NULL, NULL},
	{ 0b1111110, false, false, false, NULL, NULL},
	{ 0b1111111, false, false, false, NULL, NULL}

};

struct opdef iset_74[] = {
	{ 0, true, false, false, op_74_uj, NULL },
	{ 1, true, false, false, op_74_jl, NULL },
	{ 2, true, false, false, op_74_je, NULL },
	{ 3, true, false, false, op_74_jg, NULL },
	{ 4, true, false, false, op_74_jz, NULL },
	{ 5, true, false, false, op_74_jm, NULL },
	{ 6, true, false, false, op_74_jn, NULL },
	{ 7, true, false, false, op_74_lj, NULL }
};

struct opdef iset_75[] = {
	{ 0, true, false, false, op_75_ld, NULL },
	{ 1, true, false, false, op_75_lf, NULL },
	{ 2, true, false, false, op_75_la, NULL },
	{ 3, true, false, false, op_75_ll, NULL },
	{ 4, true, false, false, op_75_td, NULL },
	{ 5, true, false, false, op_75_tf, NULL },
	{ 6, true, false, false, op_75_ta, NULL },
	{ 7, true, false, false, op_75_tl, NULL }
};

struct opdef iset_76[] = {
	{ 0, true, false, false, op_76_rd, NULL },
	{ 1, true, false, false, op_76_rf, NULL },
	{ 2, true, false, false, op_76_ra, NULL },
	{ 3, true, false, false, op_76_rl, NULL },
	{ 4, true, false, false, op_76_pd, NULL },
	{ 5, true, false, false, op_76_pf, NULL },
	{ 6, true, false, false, op_76_pa, NULL },
	{ 7, true, false, false, op_76_pl, NULL }
};

struct opdef iset_77[] = {
	{ 0, true, false, true,  op_77_mb, NULL },
	{ 1, true, false, true,  op_77_im, NULL },
	{ 2, true, false, true,  op_77_ki, NULL },
	{ 3, true, false, true,  op_77_fi, NULL },
	{ 4, true, false, true,  op_77_sp, NULL },
	{ 5, true, false, false, op_77_md, NULL },
	{ 6, true, false, false, op_77_rz, NULL },
	{ 7, true, false, false, op_77_ib, NULL }
};

// vim: tabstop=4 shiftwidth=4 autoindent
