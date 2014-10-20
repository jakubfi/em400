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

// opcode table (instruction decoder decision table)
struct em400_op *em400_op_tab[0x10000];

// base MERA-400 instruction list
struct em400_instr em400_ilist[] = {
	{ O(020), VARMASK_DABC, { 1, 0, op_lw } },
	{ O(021), VARMASK_DABC, { 1, 0, op_tw } },
	{ O(022), VARMASK_DABC, { 1, 0, op_ls } },
	{ O(023), VARMASK_DABC, { 1, 0, op_ri } },
	{ O(024), VARMASK_DABC, { 1, 0, op_rw } },
	{ O(025), VARMASK_DABC, { 1, 0, op_pw } },
	{ O(026), VARMASK_DABC, { 1, 0, op_rj } },
	{ O(027), VARMASK_DABC, { 1, 0, op_is } },
	{ O(030), VARMASK_DABC, { 1, 0, op_bb } },
	{ O(031), VARMASK_DABC, { 1, 0, op_bm } },
	{ O(032), VARMASK_DABC, { 1, 0, op_bs } },
	{ O(033), VARMASK_DABC, { 1, 0, op_bc } },
	{ O(034), VARMASK_DABC, { 1, 0, op_bn } },
	{ O(035), VARMASK_DABC, { 1, 0, op_ou } },
	{ O(036), VARMASK_DABC, { 1, 0, op_in } },

	{ O(037)+0000, VARMASK_DBC, { 1, 0, op_37_ad } },
	{ O(037)+0100, VARMASK_DBC, { 1, 0, op_37_sd } },
	{ O(037)+0200, VARMASK_DBC, { 1, 0, op_37_mw } },
	{ O(037)+0300, VARMASK_DBC, { 1, 0, op_37_dw } },
	{ O(037)+0400, VARMASK_DBC, { 1, 0, op_37_af } },
	{ O(037)+0500, VARMASK_DBC, { 1, 0, op_37_sf } },
	{ O(037)+0600, VARMASK_DBC, { 1, 0, op_37_mf } },
	{ O(037)+0700, VARMASK_DBC, { 1, 0, op_37_df } },

	{ O(040), VARMASK_DABC, { 1, 0, op_aw } },
	{ O(041), VARMASK_DABC, { 1, 0, op_ac } },
	{ O(042), VARMASK_DABC, { 1, 0, op_sw } },
	{ O(043), VARMASK_DABC, { 1, 0, op_cw } },
	{ O(044), VARMASK_DABC, { 1, 0, op_or } },
	{ O(045), VARMASK_DABC, { 1, 0, op_om } },
	{ O(046), VARMASK_DABC, { 1, 0, op_nr } },
	{ O(047), VARMASK_DABC, { 1, 0, op_nm } },
	{ O(050), VARMASK_DABC, { 1, 0, op_er } },
	{ O(051), VARMASK_DABC, { 1, 0, op_em } },
	{ O(052), VARMASK_DABC, { 1, 0, op_xr } },
	{ O(053), VARMASK_DABC, { 1, 0, op_xm } },
	{ O(054), VARMASK_DABC, { 1, 0, op_cl } },
	{ O(055), VARMASK_DABC, { 1, 0, op_lb } },
	{ O(056), VARMASK_DABC, { 1, 0, op_rb } },
	{ O(057), VARMASK_DABC, { 1, 0, op_cb } },

	{ O(060), VARMASK_DABC, { 0, 1, op_awt } },
	{ O(061), VARMASK_DABC, { 0, 1, op_trb } },
	{ O(062), VARMASK_DABC, { 0, 1, op_irb } },
	{ O(063), VARMASK_DABC, { 0, 1, op_drb } },
	{ O(064), VARMASK_DABC, { 0, 1, op_cwt } },
	{ O(065), VARMASK_DABC, { 0, 1, op_lwt } },
	{ O(066), VARMASK_DABC, { 0, 1, op_lws } },
	{ O(067), VARMASK_DABC, { 0, 1, op_rws } },

	{ O(070)+0000, VARMASK_DBC, { 0, 1, op_70_ujs } },
	{ O(070)+0100, VARMASK_DBC, { 0, 1, op_70_jls } },
	{ O(070)+0200, VARMASK_DBC, { 0, 1, op_70_jes } },
	{ O(070)+0300, VARMASK_DBC, { 0, 1, op_70_jgs } },
	{ O(070)+0400, VARMASK_DBC, { 0, 1, op_70_jvs } },
	{ O(070)+0500, VARMASK_DBC, { 0, 1, op_70_jxs } },
	{ O(070)+0600, VARMASK_DBC, { 0, 1, op_70_jys } },
	{ O(070)+0700, VARMASK_DBC, { 0, 1, op_70_jcs } },

	{ O(071)+00000, VARMASK_BYTE, { 0, 0, op_71_blc } },
	{ O(071)+00400, VARMASK_BYTE, { 0, 0, op_71_exl } },
	{ O(071)+01000, VARMASK_BYTE, { 0, 0, op_71_brc } },
	{ O(071)+01400, VARMASK_BYTE, { 0, 0, op_71_nrf } },

	{ O(072)+00000, VARMASK_A,   { 0, 0, op_72_ric } },
	{ O(072)+00001, VARMASK_A,   { 0, 0, op_72_zlb } },
	{ O(072)+00002, VARMASK_A,   { 0, 0, op_72_sxu } },
	{ O(072)+00003, VARMASK_A,   { 0, 0, op_72_nga } },
	{ O(072)+00004, VARMASK_A,   { 0, 0, op_72_slz } },
	{ O(072)+00005, VARMASK_A,   { 0, 0, op_72_sly } },
	{ O(072)+00006, VARMASK_A,   { 0, 0, op_72_slx } },
	{ O(072)+00007, VARMASK_A,   { 0, 0, op_72_sry } },
	{ O(072)+00010, VARMASK_A,   { 0, 0, op_72_ngl } },
	{ O(072)+00011, VARMASK_A,   { 0, 0, op_72_rpc } },
	{ O(072)+00020, VARMASK_DAC, { 0, 0, op_72_shc } },
	{ O(072)+01000, VARMASK_A,   { 0, 0, op_72_rky } },
	{ O(072)+01001, VARMASK_A,   { 0, 0, op_72_zrb } },
	{ O(072)+01002, VARMASK_A,   { 0, 0, op_72_sxl } },
	{ O(072)+01003, VARMASK_A,   { 0, 0, op_72_ngc } },
	{ O(072)+01004, VARMASK_A,   { 0, 0, op_72_svz } },
	{ O(072)+01005, VARMASK_A,   { 0, 0, op_72_svy } },
	{ O(072)+01006, VARMASK_A,   { 0, 0, op_72_svx } },
	{ O(072)+01007, VARMASK_A,   { 0, 0, op_72_srx } },
	{ O(072)+01010, VARMASK_A,   { 0, 0, op_72_srz } },
	{ O(072)+01011, VARMASK_A,   { 0, 0, op_72_lpc } },

	{ O(073)+00000, VARMASK_DBC, { 0, 1, op_73_hlt } },
	{ O(073)+00100, VARMASK_DBC, { 0, 0, op_73_mcl } },
	{ O(073)+00200, VARMASK_DBC, { 0, 0, op_73_softint } },
	{ O(073)+00300, VARMASK_BC,  { 0, 0, op_73_giu } },
	{ O(073)+00400, VARMASK_DBC, { 0, 0, op_73_lip } },
	{ O(073)+01300, VARMASK_BC,  { 0, 0, op_73_gil } },
	{ O(073)+00500, VARMASK_DBC, { 0, 0, op_73_cron } },

	{ O(074)+0000, VARMASK_DBC, { 1, 0, op_74_uj } },
	{ O(074)+0100, VARMASK_DBC, { 1, 0, op_74_jl } },
	{ O(074)+0200, VARMASK_DBC, { 1, 0, op_74_je } },
	{ O(074)+0300, VARMASK_DBC, { 1, 0, op_74_jg } },
	{ O(074)+0400, VARMASK_DBC, { 1, 0, op_74_jz } },
	{ O(074)+0500, VARMASK_DBC, { 1, 0, op_74_jm } },
	{ O(074)+0600, VARMASK_DBC, { 1, 0, op_74_jn } },
	{ O(074)+0700, VARMASK_DBC, { 1, 0, op_74_lj } },

	{ O(075)+0000, VARMASK_DBC, { 1, 0, op_75_ld } },
	{ O(075)+0100, VARMASK_DBC, { 1, 0, op_75_lf } },
	{ O(075)+0200, VARMASK_DBC, { 1, 0, op_75_la } },
	{ O(075)+0300, VARMASK_DBC, { 1, 0, op_75_ll } },
	{ O(075)+0400, VARMASK_DBC, { 1, 0, op_75_td } },
	{ O(075)+0500, VARMASK_DBC, { 1, 0, op_75_tf } },
	{ O(075)+0600, VARMASK_DBC, { 1, 0, op_75_ta } },
	{ O(075)+0700, VARMASK_DBC, { 1, 0, op_75_tl } },

	{ O(076)+0000, VARMASK_DBC, { 1, 0, op_76_rd } },
	{ O(076)+0100, VARMASK_DBC, { 1, 0, op_76_rf } },
	{ O(076)+0200, VARMASK_DBC, { 1, 0, op_76_ra } },
	{ O(076)+0300, VARMASK_DBC, { 1, 0, op_76_rl } },
	{ O(076)+0400, VARMASK_DBC, { 1, 0, op_76_pd } },
	{ O(076)+0500, VARMASK_DBC, { 1, 0, op_76_pf } },
	{ O(076)+0600, VARMASK_DBC, { 1, 0, op_76_pa } },
	{ O(076)+0700, VARMASK_DBC, { 1, 0, op_76_pl } },

	{ O(077)+0000, VARMASK_DBC, { 1, 0, op_77_mb } },
	{ O(077)+0100, VARMASK_DBC, { 1, 0, op_77_im } },
	{ O(077)+0200, VARMASK_DBC, { 1, 0, op_77_ki } },
	{ O(077)+0300, VARMASK_DBC, { 1, 0, op_77_fi } },
	{ O(077)+0400, VARMASK_DBC, { 1, 0, op_77_sp } },
	{ O(077)+0500, VARMASK_DBC, { 1, 0, op_77_md } },
	{ O(077)+0600, VARMASK_DBC, { 1, 0, op_77_rz } },
	{ O(077)+0700, VARMASK_DBC, { 1, 0, op_77_ib } },

	{ 0, 0, { 0, 0, NULL } }
};

// illegal instruction
struct em400_instr em400_instr_illegal = { 0, VARMASK_ALL, { 0, 0, op_illegal} };

// vim: tabstop=4 shiftwidth=4 autoindent
