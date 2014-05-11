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

#include "dasm/dasm_formats.h"
#include "dasm/dasm_iset.h"

#include "cpu/cpu.h"       /* EXT_OP_xx macros         */
#include "cpu/registers.h" /* .. need _A() and friends */

static struct dasm_opdef dasm_iset_37[];
static struct dasm_opdef dasm_iset_70[];
static struct dasm_opdef dasm_iset_71[];
static struct dasm_opdef dasm_iset_72[];
static struct dasm_opdef dasm_iset_73[];
static struct dasm_opdef dasm_iset_74[];
static struct dasm_opdef dasm_iset_75[];
static struct dasm_opdef dasm_iset_76[];
static struct dasm_opdef dasm_iset_77[];

/* 
 * Unroll macros to become first class functions
 * to be point towards them in the data structure
 */
#define EXT_OP_FUNC(i) static int dt_extcode_##i(int p) { return EXT_OP_##i(p); }

EXT_OP_FUNC(37)
EXT_OP_FUNC(70)
EXT_OP_FUNC(71)
EXT_OP_FUNC(72)
EXT_OP_FUNC(73)
EXT_OP_FUNC(74)
EXT_OP_FUNC(75)
EXT_OP_FUNC(76)
EXT_OP_FUNC(77)
#undef EXT_OP_FUNC

static struct dasm_opdef dasm_iset_37[] = {
	{ 0, "AD", true, NULL, NULL, D_FD, T_AD },
	{ 1, "SD", true, NULL, NULL, D_FD, T_SD },
	{ 2, "MW", true, NULL, NULL, D_FD, T_MW },
	{ 3, "DW", true, NULL, NULL, D_FD, T_DW },
	{ 4, "AF", true, NULL, NULL, D_FD, T_AF },
	{ 5, "SF", true, NULL, NULL, D_FD, T_SF },
	{ 6, "MF", true, NULL, NULL, D_FD, T_MF },
	{ 7, "DF", true, NULL, NULL, D_FD, T_DF }
};

static struct dasm_opdef dasm_iset_70[] = {
	{ 0, "UJS", false, NULL, NULL, D_JS, T_UJS },
	{ 1, "JLS", false, NULL, NULL, D_JS, T_JLS },
	{ 2, "JES", false, NULL, NULL, D_JS, T_JES },
	{ 3, "JGS", false, NULL, NULL, D_JS, T_JGS },
	{ 4, "JVS", false, NULL, NULL, D_JS, T_JVS },
	{ 5, "JXS", false, NULL, NULL, D_JS, T_JXS },
	{ 6, "JYS", false, NULL, NULL, D_JS, T_JYS },
	{ 7, "JCS", false, NULL, NULL, D_JS, T_JCS }
};

static struct dasm_opdef dasm_iset_71[] = {
	{ 0, "BLC", false, NULL, NULL, D_KA2, T_BLC },
	{ 1, "EXL", false, NULL, NULL, D_KA2, T_EXL },
	{ 2, "BRC", false, NULL, NULL, D_KA2, T_BRC },
	{ 3, "NRF", false, NULL, NULL, D_KA2, T_NRF }
};

static struct dasm_opdef dasm_iset_72[] = {
	{ 0b0000000, "RIC", false, NULL, NULL, D_C, T_RIC },
	{ 0b0000001, "ZLB", false, NULL, NULL, D_C, T_ZLB },
	{ 0b0000010, "SXU", false, NULL, NULL, D_C, T_SXU },
	{ 0b0000011, "NGA", false, NULL, NULL, D_C, T_NGA },
	{ 0b0000100, "SLZ", false, NULL, NULL, D_C, T_SLZ },
	{ 0b0000101, "SLY", false, NULL, NULL, D_C, T_SLY },
	{ 0b0000110, "SLX", false, NULL, NULL, D_C, T_SLX },
	{ 0b0000111, "SRY", false, NULL, NULL, D_C, T_SRY },
	{ 0b0001000, "NGL", false, NULL, NULL, D_C, T_NGL },
	{ 0b0001001, "RPC", false, NULL, NULL, D_C, T_RPC },
	{ 0b0001010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0001011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0001100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0001101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0001110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0001111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0010000, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010001, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010010, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010011, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010100, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010101, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010110, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0010111, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b0011000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0011111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0100111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0101111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0110111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b0111111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1000000, "RKY", false, NULL, NULL, D_C, T_RKY },
	{ 0b1000001, "ZRB", false, NULL, NULL, D_C, T_ZRB },
	{ 0b1000010, "SXL", false, NULL, NULL, D_C, T_SXL },
	{ 0b1000011, "NGC", false, NULL, NULL, D_C, T_NGC },
	{ 0b1000100, "SVZ", false, NULL, NULL, D_C, T_SVZ },
	{ 0b1000101, "SVY", false, NULL, NULL, D_C, T_SVY },
	{ 0b1000110, "SVX", false, NULL, NULL, D_C, T_SVX },
	{ 0b1000111, "SRX", false, NULL, NULL, D_C, T_SRX },
	{ 0b1001000, "SRZ", false, NULL, NULL, D_C, T_SRZ },
	{ 0b1001001, "LPC", false, NULL, NULL, D_C, T_LPC },
	{ 0b1001010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1001011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1001100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1001101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1001110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1001111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1010001, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010000, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010010, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010011, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010100, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010101, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010110, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1010111, "SHC", false, NULL, NULL, D_SHC, T_SHC },
	{ 0b1011000, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011001, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011010, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011011, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011100, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011101, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011110, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1011111, NULL,  false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1100111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1101111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1110111, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111000, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111001, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111010, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111011, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111100, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111101, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111110, NULL, false, NULL, NULL, DT_ILL, DT_ILL },
	{ 0b1111111, NULL, false, NULL, NULL, DT_ILL, DT_ILL }
};

static struct dasm_opdef dasm_iset_73[] = {
	{ 0b0000000, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000001, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000010, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000011, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000100, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000101, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000110, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b0000111, "HLT", false, NULL, NULL, D_HLT, T_HLT },

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
	{ 0b0010100, "SINT", false, NULL, NULL, D_S, T_SINT },
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

	{ 0b0101000, "CRON",false, NULL, NULL, D_S, T_CRON },
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

	{ 0b1000000, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000001, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000010, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000011, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000100, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000101, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000110, "HLT", false, NULL, NULL, D_HLT, T_HLT },
	{ 0b1000111, "HLT", false, NULL, NULL, D_HLT, T_HLT },

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
	{ 0b1010100, "SIND", false, NULL, NULL, D_S, T_SIND },
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

static struct dasm_opdef dasm_iset_74[] = {
	{ 0, "UJ", true, NULL, NULL, D_J, T_UJ },
	{ 1, "JL", true, NULL, NULL, D_J, T_JL },
	{ 2, "JE", true, NULL, NULL, D_J, T_JE },
	{ 3, "JG", true, NULL, NULL, D_J, T_JG },
	{ 4, "JZ", true, NULL, NULL, D_J, T_JZ },
	{ 5, "JM", true, NULL, NULL, D_J, T_JM },
	{ 6, "JN", true, NULL, NULL, D_J, T_JN },
	{ 7, "LJ", true, NULL, NULL, D_J, T_LJ }
};

static struct dasm_opdef dasm_iset_75[] = {
	{ 0, "LD", true, NULL, NULL, D_L, T_LD },
	{ 1, "LF", true, NULL, NULL, D_L, T_LF },
	{ 2, "LA", true, NULL, NULL, D_L, T_LA },
	{ 3, "LL", true, NULL, NULL, D_L, T_LL },
	{ 4, "TD", true, NULL, NULL, D_L, T_TD },
	{ 5, "TF", true, NULL, NULL, D_L, T_TF },
	{ 6, "TA", true, NULL, NULL, D_L, T_TA },
	{ 7, "TL", true, NULL, NULL, D_L, T_TL }
};

static struct dasm_opdef dasm_iset_76[] = {
	{ 0, "RD", true, NULL, NULL, D_G, T_RD },
	{ 1, "RF", true, NULL, NULL, D_G, T_RF },
	{ 2, "RA", true, NULL, NULL, D_G, T_RA },
	{ 3, "RL", true, NULL, NULL, D_G, T_RL },
	{ 4, "PD", true, NULL, NULL, D_G, T_PD },
	{ 5, "PF", true, NULL, NULL, D_G, T_PF },
	{ 6, "PA", true, NULL, NULL, D_G, T_PA },
	{ 7, "PL", true, NULL, NULL, D_G, T_PL }
};

static struct dasm_opdef dasm_iset_77[] = {
	{ 0, "MB", true, NULL, NULL, D_BN, T_MB },
	{ 1, "IM", true, NULL, NULL, D_BN, T_IM },
	{ 2, "KI", true, NULL, NULL, D_BN, T_KI },
	{ 3, "FI", true, NULL, NULL, D_BN, T_FI },
	{ 4, "SP", true, NULL, NULL, D_BN, T_SP },
	{ 5, "MD", true, NULL, NULL, D_BN, T_MD },
	{ 6, "RZ", true, NULL, NULL, D_BN, T_RZ },
	{ 7, "IB", true, NULL, NULL, D_BN, T_IB }
};

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

	{ 037, NULL, false, dt_extcode_37, dasm_iset_37, NULL, NULL },

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
	{ 062, "IRB", false, NULL, NULL, D_KA1R, T_IRB },
	{ 063, "DRB", false, NULL, NULL, D_KA1R, T_DRB },
	{ 064, "CWT", false, NULL, NULL, D_KA1, T_CWT },
	{ 065, "LWT", false, NULL, NULL, D_KA1, T_LWT },
	{ 066, "LWS", false, NULL, NULL, D_KA1R, T_LWS },
	{ 067, "RWS", false, NULL, NULL, D_KA1R, T_RWS },

	{ 070, NULL, false, dt_extcode_70, dasm_iset_70, NULL, NULL },
	{ 071, NULL, false, dt_extcode_71, dasm_iset_71, NULL, NULL },
	{ 072, NULL, false, dt_extcode_72, dasm_iset_72, NULL, NULL },
	{ 073, NULL, false, dt_extcode_73, dasm_iset_73, NULL, NULL },
	{ 074, NULL, false, dt_extcode_74, dasm_iset_74, NULL, NULL },
	{ 075, NULL, false, dt_extcode_75, dasm_iset_75, NULL, NULL },
	{ 076, NULL, false, dt_extcode_76, dasm_iset_76, NULL, NULL },
	{ 077, NULL, false, dt_extcode_77, dasm_iset_77, NULL, NULL }
};

// vim: tabstop=4 shiftwidth=4 autoindent
