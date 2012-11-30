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
#include <stdbool.h>

#include "dasm.h"
#include "dasm_formats.h"

struct dasm_opdef dasm_iset[] = {
	{ 000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 002, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 003, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 004, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 005, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 006, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 007, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 012, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 013, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 014, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 015, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 016, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 017, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	
	{ 020, "LW", true, NULL, NULL, D_2ARGN, T_LW },
	{ 021, "TW", true, NULL, NULL, D_2ARGN, T_TW },
	{ 022, "LS", true, NULL, NULL, D_2ARGN, T_LS },
	{ 023, "RI", true, NULL, NULL, D_2ARGN, T_RI },
	{ 024, "RW", true, NULL, NULL, D_2ARGN, T_RW },
	{ 025, "PW", true, NULL, NULL, D_2ARGN, T_PW },
	{ 026, "RJ", true, NULL, NULL, D_2ARGN, T_RJ },
	{ 027, "IS", true, NULL, NULL, D_2ARGN, T_IS },
	{ 030, "BB", true, NULL, NULL, D_2ARGN, T_BB },
	{ 031, "BM", true, NULL, NULL, D_2ARGN, T_BM },
	{ 032, "BS", true, NULL, NULL, D_2ARGN, T_BS },
	{ 033, "BC", true, NULL, NULL, D_2ARGN, T_BC },
	{ 034, "BN", true, NULL, NULL, D_2ARGN, T_BN },
	{ 035, "OU", true, NULL, NULL, D_2ARGN, T_OU },
	{ 036, "IN", true, NULL, NULL, D_2ARGN, T_IN },

	{ 037, NULL, false, dasm_extcode_37, dasm_iset_37, NULL },

	{ 040, "AW", true, NULL, NULL, D_2ARGN, T_AW },
	{ 041, "AC", true, NULL, NULL, D_2ARGN, T_AC },
	{ 042, "SW", true, NULL, NULL, D_2ARGN, T_SW },
	{ 043, "CW", true, NULL, NULL, D_2ARGN, T_CW },
	{ 044, "OR", true, NULL, NULL, D_2ARGN, T_OR },
	{ 045, "OM", true, NULL, NULL, D_2ARGN, T_OM },
	{ 046, "NR", true, NULL, NULL, D_2ARGN, T_NR },
	{ 047, "NM", true, NULL, NULL, D_2ARGN, T_NM },
	{ 050, "ER", true, NULL, NULL, D_2ARGN, T_ER },
	{ 051, "EM", true, NULL, NULL, D_2ARGN, T_EM },
	{ 052, "XR", true, NULL, NULL, D_2ARGN, T_XR },
	{ 053, "XM", true, NULL, NULL, D_2ARGN, T_XM },
	{ 054, "CL", true, NULL, NULL, D_2ARGN, T_CL },
	{ 055, "LB", true, NULL, NULL, D_2ARGN, T_LB },
	{ 056, "RB", true, NULL, NULL, D_2ARGN, T_RB },
	{ 057, "CB", true, NULL, NULL, D_2ARGN, T_CB },

	{ 060, "AWT", false, NULL, NULL, D_KA1, T_AWT },
	{ 061, "TRB", false, NULL, NULL, D_KA1, T_TRB },
	{ 062, "IRB", false, NULL, NULL, D_KA1, T_IRB },
	{ 063, "DRB", false, NULL, NULL, D_KA1, T_DRB },
	{ 064, "CWT", false, NULL, NULL, D_KA1, T_CWT },
	{ 065, "LWT", false, NULL, NULL, D_KA1, T_LWT },
	{ 066, "LWS", false, NULL, NULL, D_KA1, T_LWS },
	{ 067, "RWS", false, NULL, NULL, D_KA1, T_RWS },

	{ 070, NULL, false, dasm_extcode_70, dasm_iset_70, NULL },
	{ 071, NULL, false, dasm_extcode_71, dasm_iset_71, NULL },
	{ 072, NULL, false, dasm_extcode_72, dasm_iset_72, NULL },
	{ 073, NULL, false, dasm_extcode_73, dasm_iset_73, NULL },
	{ 074, NULL, false, dasm_extcode_74, dasm_iset_74, NULL },
	{ 075, NULL, false, dasm_extcode_75, dasm_iset_75, NULL },
	{ 076, NULL, false, dasm_extcode_76, dasm_iset_76, NULL },
	{ 077, NULL, false, dasm_extcode_77, dasm_iset_77, NULL }
};

struct dasm_opdef dasm_iset_37[] = {
	{ 0, "AD", true, NULL, NULL, D_FD, T_AD },
	{ 1, "SD", true, NULL, NULL, D_FD, T_SD },
	{ 2, "MW", true, NULL, NULL, D_FD, T_MW },
	{ 3, "DW", true, NULL, NULL, D_FD, T_DW },
	{ 4, "AF", true, NULL, NULL, D_FD, T_AF },
	{ 5, "SF", true, NULL, NULL, D_FD, T_SF },
	{ 6, "MF", true, NULL, NULL, D_FD, T_MF },
	{ 7, "DF", true, NULL, NULL, D_FD, T_DF }
};

struct dasm_opdef dasm_iset_70[] = {
	{ 0, "UJS", false, NULL, NULL, D_JS, T_UJS },
	{ 1, "JLS", false, NULL, NULL, D_JS, T_JLS },
	{ 2, "JES", false, NULL, NULL, D_JS, T_JES },
	{ 3, "JGS", false, NULL, NULL, D_JS, T_JGS },
	{ 4, "JVS", false, NULL, NULL, D_JS, T_JVS },
	{ 5, "JXS", false, NULL, NULL, D_JS, T_JXS },
	{ 6, "JYS", false, NULL, NULL, D_JS, T_JYS },
	{ 7, "JCS", false, NULL, NULL, D_JS, T_JCS }
};

struct dasm_opdef dasm_iset_71[] = {
	{ 0, "BLC", false, NULL, NULL, D_KA2, T_BLC },
	{ 1, "EXL", false, NULL, NULL, D_KA2, T_EXL },
	{ 2, "BRC", false, NULL, NULL, D_KA2, T_BRC },
	{ 3, "NRF", false, NULL, NULL, D_KA2, T_NRF }
};

struct dasm_opdef dasm_iset_72[] = {
	{ 0b000000, "RIC", false, NULL, NULL, D_C, T_RIC },
	{ 0b000001, "ZLB", false, NULL, NULL, D_C, T_ZLB },
	{ 0b000010, "SXU", false, NULL, NULL, D_C, T_SXU },
	{ 0b000011, "NGA", false, NULL, NULL, D_C, T_NGA },
	{ 0b000100, "SLZ", false, NULL, NULL, D_C, T_SLZ },
	{ 0b000101, "SLY", false, NULL, NULL, D_C, T_SLY },
	{ 0b000110, "SLX", false, NULL, NULL, D_C, T_SLX },
	{ 0b000111, "SRY", false, NULL, NULL, D_C, T_SRY },
	{ 0b001000, "NGL", false, NULL, NULL, D_C, T_NGL },
	{ 0b001001, "RPC", false, NULL, NULL, D_C, T_RPC },
	{ 0b001010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b001111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b010000, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010001, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010010, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010011, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010100, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010101, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010110, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b010111, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b011000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b011111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b100000, "RKY", false, NULL, NULL, D_C, T_RKY },
	{ 0b100001, "ZRB", false, NULL, NULL, D_C, T_ZRB },
	{ 0b100010, "SXL", false, NULL, NULL, D_C, T_SXL },
	{ 0b100011, "NGC", false, NULL, NULL, D_C, T_NGC },
	{ 0b100100, "SVZ", false, NULL, NULL, D_C, T_SVZ },
	{ 0b100101, "SVY", false, NULL, NULL, D_C, T_SVY },
	{ 0b100110, "SVX", false, NULL, NULL, D_C, T_SVX },
	{ 0b100111, "SRX", false, NULL, NULL, D_C, T_SRX },
	{ 0b101000, "SRZ", false, NULL, NULL, D_C, T_SRZ },
	{ 0b101001, "LPC", false, NULL, NULL, D_C, T_LPC },
	{ 0b101010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b101111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b110001, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110000, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110010, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110011, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110100, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110101, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110110, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b110111, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b111000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b111111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL }
};

struct dasm_opdef dasm_iset_73[] = {
	{ 0b0000000, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000001, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000010, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000011, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000100, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000101, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000110, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b0000111, "HLT", false, NULL, NULL, D_S, T_HLT },

	{ 0b0001000, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001001, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001010, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001011, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001100, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001101, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001110, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b0001111, "MCL", false, NULL, NULL, D_S, T_MCL },

	{ 0b0010000, "CIT", false, NULL, NULL, D_S, T_CIT },
	{ 0b0010001, "SIL", false, NULL, NULL, D_S, T_SIL },
	{ 0b0010010, "SIU", false, NULL, NULL, D_S, T_SIU },
	{ 0b0010011, "SIT", false, NULL, NULL, D_S, T_SIT },
	{ 0b0010100, "SIX", false, NULL, NULL, D_S, T_SIX },
	{ 0b0010101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0010110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0010111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b0011000, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011001, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011010, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011011, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011100, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011101, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011110, "GIU", false, NULL, NULL, D_S, T_GIU },
	{ 0b0011111, "GIU", false, NULL, NULL, D_S, T_GIU },

	{ 0b0100000, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100001, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100010, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100011, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100100, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100101, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100110, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b0100111, "LIP", false, NULL, NULL, D_S, T_LIP },

	{ 0b0101000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b0110000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b0111000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b1000000, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000001, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000010, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000011, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000100, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000101, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000110, "HLT", false, NULL, NULL, D_S, T_HLT },
	{ 0b1000111, "HLT", false, NULL, NULL, D_S, T_HLT },

	{ 0b1001000, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001001, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001010, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001011, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001100, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001101, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001110, "MCL", false, NULL, NULL, D_S, T_MCL },
	{ 0b1001111, "MCL", false, NULL, NULL, D_S, T_MCL },

	{ 0b1010000, "CIT", false, NULL, NULL, D_S, T_CIT },
	{ 0b1010001, "SIL", false, NULL, NULL, D_S, T_SIL },
	{ 0b1010010, "SIU", false, NULL, NULL, D_S, T_SIU },
	{ 0b1010011, "SIT", false, NULL, NULL, D_S, T_SIT },
	{ 0b1010100, "CIX", false, NULL, NULL, D_S, T_CIX },
	{ 0b1010101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1010110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1010111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b1011000, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011001, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011010, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011011, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011100, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011101, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011110, "GIL", false, NULL, NULL, D_S, T_GIL },
	{ 0b1011111, "GIL", false, NULL, NULL, D_S, T_GIL },

	{ 0b1100001, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100000, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100010, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100011, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100101, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100100, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100110, "LIP", false, NULL, NULL, D_S, T_LIP },
	{ 0b1100111, "LIP", false, NULL, NULL, D_S, T_LIP },

	{ 0b1101000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b1110000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },

	{ 0b1111000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL }

};

struct dasm_opdef dasm_iset_74[] = {
	{ 0, "UJ", true, NULL, NULL, D_J, T_UJ },
	{ 1, "JL", true, NULL, NULL, D_J, T_JL },
	{ 2, "JE", true, NULL, NULL, D_J, T_JE },
	{ 3, "JG", true, NULL, NULL, D_J, T_JG },
	{ 4, "JZ", true, NULL, NULL, D_J, T_JZ },
	{ 5, "JM", true, NULL, NULL, D_J, T_JM },
	{ 6, "JN", true, NULL, NULL, D_J, T_JN },
	{ 7, "LJ", true, NULL, NULL, D_J, T_LJ }
};

struct dasm_opdef dasm_iset_75[] = {
	{ 0, "LD", true, NULL, NULL, D_L, T_LD },
	{ 1, "LF", true, NULL, NULL, D_L, T_LF },
	{ 2, "LA", true, NULL, NULL, D_L, T_LA },
	{ 3, "LL", true, NULL, NULL, D_L, T_LL },
	{ 4, "TD", true, NULL, NULL, D_L, T_TD },
	{ 5, "TF", true, NULL, NULL, D_L, T_TF },
	{ 6, "TA", true, NULL, NULL, D_L, T_TA },
	{ 7, "TL", true, NULL, NULL, D_L, T_TL }
};

struct dasm_opdef dasm_iset_76[] = {
	{ 0, "RD", true, NULL, NULL, D_G, T_RD },
	{ 1, "RF", true, NULL, NULL, D_G, T_RF },
	{ 2, "RA", true, NULL, NULL, D_G, T_RA },
	{ 3, "RL", true, NULL, NULL, D_G, T_RL },
	{ 4, "PD", true, NULL, NULL, D_G, T_PD },
	{ 5, "PF", true, NULL, NULL, D_G, T_PF },
	{ 6, "PA", true, NULL, NULL, D_G, T_PA },
	{ 7, "PL", true, NULL, NULL, D_G, T_PL }
};

struct dasm_opdef dasm_iset_77[] = {
	{ 0, "MB", true, NULL, NULL, D_BN, T_MB },
	{ 1, "IM", true, NULL, NULL, D_BN, T_IM },
	{ 2, "KI", true, NULL, NULL, D_BN, T_KI },
	{ 3, "FI", true, NULL, NULL, D_BN, T_FI },
	{ 4, "SP", true, NULL, NULL, D_BN, T_SP },
	{ 5, "MD", true, NULL, NULL, D_BN, T_MD },
	{ 6, "RZ", true, NULL, NULL, D_BN, T_RZ },
	{ 7, "IB", true, NULL, NULL, D_BN, T_IB }
};

// vim: tabstop=4
