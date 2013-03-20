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
#include <strings.h>

#include "ops.h"
#include "elements.h"

int mnemo_sel = 0;

struct op_t ops[] = {

{ {"LW", "LO"  }, N_2ARG, 0b0100000000000000 },
{ {"TW", "LOB" }, N_2ARG, 0b0100010000000000 },
{ {"LS", "LOM" }, N_2ARG, 0b0100100000000000 },
{ {"RI", "LOS" }, N_2ARG, 0b0100110000000000 },
{ {"RW", "ST"  }, N_2ARG, 0b0101000000000000 },
{ {"PW", "STB" }, N_2ARG, 0b0101010000000000 },
{ {"RJ", "JPAR"}, N_2ARG, 0b0101100000000000 },
{ {"IS", "IS"  }, N_2ARG, 0b0101110000000000 },
{ {"BB", "CLBO"}, N_2ARG, 0b0110000000000000 },
{ {"BM", "BM"  }, N_2ARG, 0b0110010000000000 },
{ {"BS", "CLMO"}, N_2ARG, 0b0110100000000000 },
{ {"BC", "BC"  }, N_2ARG, 0b0110110000000000 },
{ {"BN", "BN"  }, N_2ARG, 0b0111000000000000 },
{ {"OU", "OU"  }, N_2ARG, 0b0111010000000000 },
{ {"IN", "IN"  }, N_2ARG, 0b0111100000000000 },

{ {"AD", "ADD"}, N_FD, 0b0111110000000000 },
{ {"SD", "SD" }, N_FD, 0b0111110001000000 },
{ {"MW", "MW" }, N_FD, 0b0111110010000000 },
{ {"DW", "DW" }, N_FD, 0b0111110011000000 },
{ {"AF", "ADF"}, N_FD, 0b0111110100000000 },
{ {"SF", "SBF"}, N_FD, 0b0111110101000000 },
{ {"MF", "MLF"}, N_FD, 0b0111110110000000 },
{ {"DF", "DVF"}, N_FD, 0b0111110111000000 },

{ {"AW", "AD" }, N_2ARG, 0b1000000000000000 },
{ {"AC", "ADC"}, N_2ARG, 0b1000010000000000 },
{ {"SW", "SU" }, N_2ARG, 0b1000100000000000 },
{ {"CW", "CO" }, N_2ARG, 0b1000110000000000 },
{ {"OR", "OR" }, N_2ARG, 0b1001000000000000 },
{ {"OM", "OM" }, N_2ARG, 0b1001010000000000 },
{ {"NR", "AND"}, N_2ARG, 0b1001100000000000 },
{ {"NM", "NM" }, N_2ARG, 0b1001110000000000 },
{ {"ER", "ORN"}, N_2ARG, 0b1010000000000000 },
{ {"EM", "EM" }, N_2ARG, 0b1010010000000000 },
{ {"XR", "XR" }, N_2ARG, 0b1010100000000000 },
{ {"XM", "XM" }, N_2ARG, 0b1010110000000000 },
{ {"CL", "CL" }, N_2ARG, 0b1011000000000000 },
{ {"LB", "LB" }, N_2ARG, 0b1011010000000000 },
{ {"RB", "RB" }, N_2ARG, 0b1011100000000000 },
{ {"CB", "CB" }, N_2ARG, 0b1011110000000000 },

{ {"AWT", "ADT" }, N_KA1, 0b1100000000000000 },
{ {"TRB", "ADOT"}, N_KA1, 0b1100010000000000 },
{ {"IRB", "ADJT"}, N_KA1, 0b1100100000000000 },
{ {"DRB", "DRB"},  N_KA1, 0b1100110000000000 },
{ {"CWT", "COT" }, N_KA1, 0b1101000000000000 },
{ {"LWT", "LOT" }, N_KA1, 0b1101010000000000 },
{ {"LWS", "LTS" }, N_KA1, 0b1101100000000000 },
{ {"RWS", "STS" }, N_KA1, 0b1101110000000000 },

{ {"UJS", "JPT" }, N_JS, 0b1110000000000000 },
{ {"JLS", "JPTL"}, N_JS, 0b1110000001000000 },
{ {"JES", "JPTE"}, N_JS, 0b1110000010000000 },
{ {"JGS", "JPTG"}, N_JS, 0b1110000011000000 },
{ {"JVS", "JPTV"}, N_JS, 0b1110000100000000 },
{ {"JXS", "JPTX"}, N_JS, 0b1110000101000000 },
{ {"JYS", "JPTY"}, N_JS, 0b1110000110000000 },
{ {"JCS", "JCS" }, N_JS, 0b1110000111000000 },

{ {"BLC", "BLC"}, N_KA2, 0b1110010000000000 },
{ {"EXL", "EXL"}, N_KA2, 0b1110010100000000 },
{ {"BRC", "BRC"}, N_KA2, 0b1110011000000000 },
{ {"NRF", "NLZ"}, N_KA2, 0b1110011100000000 },

{ {"RIC", "RIC" }, N_C, 0b1110100000000000 },
{ {"ZLB", "ZLB" }, N_C, 0b1110100000000001 },
{ {"SXU", "STXA"}, N_C, 0b1110100000000010 },
{ {"NGA", "NEGA"}, N_C, 0b1110100000000011 },
{ {"SLZ", "SHL" }, N_C, 0b1110100000000100 },
{ {"SLY", "SHLY"}, N_C, 0b1110100000000101 },
{ {"SLX", "SHLX"}, N_C, 0b1110100000000110 },
{ {"SRY", "SHRY"}, N_C, 0b1110100000000111 },
{ {"NGL", "NEG" }, N_C, 0b1110100000001000 },
{ {"RPC", "RPC" }, N_C, 0b1110100000001001 },
{ {"SHC", "SHC" }, N_SHC, 0b1110100000010000 },
{ {"RKY", "RKEY"}, N_C, 0b1110101000000000 },
{ {"ZRB", "ZRB" }, N_C, 0b1110101000000001 },
{ {"SXL", "STXZ"}, N_C, 0b1110101000000010 },
{ {"NGC", "NEC" }, N_C, 0b1110101000000011 },
{ {"SVZ", "SHV" }, N_C, 0b1110101000000100 },
{ {"SVY", "SHVY"}, N_C, 0b1110101000000101 },
{ {"SVX", "SHVX"}, N_C, 0b1110101000000110 },
{ {"SRX", "SHRX"}, N_C, 0b1110101000000111 },
{ {"SRZ", "SHR" }, N_C, 0b1110101000001000 },
{ {"LPC", "LPC" }, N_C, 0b1110101000001001 },

{ {"HLT", "STOP"}, N_HLT, 0b1110110000000000 },
{ {"MCL", "MCL" }, N_S, 0b1110110001000000 },
{ {"CIT", "CIT" }, N_S, 0b1110110010000000 },
{ {"SIL", "SIL" }, N_S, 0b1110110010000001 },
{ {"SIU", "SIU" }, N_S, 0b1110110010000010 },
{ {"SIT", "SIT" }, N_S, 0b1110110010000011 },
{ {"GIU", "GIU" }, N_S, 0b1110110011000000 },
{ {"LIP", "LIP" }, N_S, 0b1110110100000000 },
{ {"GIL", "GIL" }, N_S, 0b1110111011000000 },

// fake UJS 0
{ {"NOP", "NOP"}, N_S, 0b1110000000000000 },

{ {"UJ", "JP" }, N_J, 0b1111000000000000 },
{ {"JL", "JPL"}, N_J, 0b1111000001000000 },
{ {"JE", "JPE"}, N_J, 0b1111000010000000 },
{ {"JG", "JPG"}, N_J, 0b1111000011000000 },
{ {"JZ", "JZ" }, N_J, 0b1111000100000000 },
{ {"JM", "JM" }, N_J, 0b1111000101000000 },
{ {"JN", "JN" }, N_J, 0b1111000110000000 },
{ {"LJ", "JPR"}, N_J, 0b1111000111000000 },

{ {"LD", "LDD" }, N_L, 0b1111010000000000 },
{ {"LF", "LDF" }, N_L, 0b1111010001000000 },
{ {"LA", "LDR" }, N_L, 0b1111010010000000 },
{ {"LL", "LDM" }, N_L, 0b1111010011000000 },
{ {"TD", "LDDB"}, N_L, 0b1111010100000000 },
{ {"TF", "LDFB"}, N_L, 0b1111010101000000 },
{ {"TA", "LDRB"}, N_L, 0b1111010110000000 },
{ {"TL", "LDMB"}, N_L, 0b1111010111000000 },

{ {"RD", "STD" }, N_G, 0b1111100000000000 },
{ {"RF", "STF" }, N_G, 0b1111100001000000 },
{ {"RA", "STR" }, N_G, 0b1111100010000000 },
{ {"RL", "STM" }, N_G, 0b1111100011000000 },
{ {"PD", "STDB"}, N_G, 0b1111100100000000 },
{ {"PF", "STFB"}, N_G, 0b1111100101000000 },
{ {"PA", "STRB"}, N_G, 0b1111100110000000 },
{ {"PL", "STMB"}, N_G, 0b1111100111000000 },

{ {"MB", "MB"  }, N_BN, 0b1111110000000000 },
{ {"IM", "IM"  }, N_BN, 0b1111110001000000 },
{ {"KI", "KI"  }, N_BN, 0b1111110010000000 },
{ {"FI", "FI"  }, N_BN, 0b1111110011000000 },
{ {"SP", "SP"  }, N_BN, 0b1111110100000000 },
{ {"MD", "MOD" }, N_BN, 0b1111110101000000 },
{ {"RZ", "ZS"  }, N_BN, 0b1111110110000000 },
{ {"IB", "ADOS"}, N_BN, 0b1111110111000000 },

{ {NULL, NULL}, 0, 0 }

};

// -----------------------------------------------------------------------
struct op_t * get_op(char *opname)
{
	struct op_t *op = ops;

	while (op->mnemo[mnemo_sel]) {
		if (!strcasecmp(op->mnemo[mnemo_sel], opname)) {
			return op;
		}
		op++;
	}

	return NULL;
}

// vim: tabstop=4
