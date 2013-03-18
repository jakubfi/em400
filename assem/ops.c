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

int mnemo_sel = 0;

struct op_t ops[] = {

{ {"LW", "LO"  }, O_2ARG, 0b0100000000000000 },
{ {"TW", "LOB" }, O_2ARG, 0b0100010000000000 },
{ {"LS", "LOM" }, O_2ARG, 0b0100100000000000 },
{ {"RI", "LOS" }, O_2ARG, 0b0100110000000000 },
{ {"RW", "ST"  }, O_2ARG, 0b0101000000000000 },
{ {"PW", "STB" }, O_2ARG, 0b0101010000000000 },
{ {"RJ", "JPAR"}, O_2ARG, 0b0101100000000000 },
{ {"IS", "IS"  }, O_2ARG, 0b0101110000000000 },
{ {"BB", "CLBO"}, O_2ARG, 0b0110000000000000 },
{ {"BM", "BM"  }, O_2ARG, 0b0110010000000000 },
{ {"BS", "CLMO"}, O_2ARG, 0b0110100000000000 },
{ {"BC", "BC"  }, O_2ARG, 0b0110110000000000 },
{ {"BN", "BN"  }, O_2ARG, 0b0111000000000000 },
{ {"OU", "OU"  }, O_2ARG, 0b0111010000000000 },
{ {"IN", "IN"  }, O_2ARG, 0b0111100000000000 },

{ {"AD", "ADD"}, O_FD, 0b0111110000000000 },
{ {"SD", "SD" }, O_FD, 0b0111110001000000 },
{ {"MW", "MW" }, O_FD, 0b0111110010000000 },
{ {"DW", "DW" }, O_FD, 0b0111110011000000 },
{ {"AF", "ADF"}, O_FD, 0b0111110100000000 },
{ {"SF", "SBF"}, O_FD, 0b0111110101000000 },
{ {"MF", "MLF"}, O_FD, 0b0111110110000000 },
{ {"DF", "DVF"}, O_FD, 0b0111110111000000 },

{ {"AW", "AD" }, O_2ARG, 0b1000000000000000 },
{ {"AC", "ADC"}, O_2ARG, 0b1000010000000000 },
{ {"SW", "SU" }, O_2ARG, 0b1000100000000000 },
{ {"CW", "CO" }, O_2ARG, 0b1000110000000000 },
{ {"OR", "OR" }, O_2ARG, 0b1001000000000000 },
{ {"OM", "OM" }, O_2ARG, 0b1001010000000000 },
{ {"NR", "AND"}, O_2ARG, 0b1001100000000000 },
{ {"NM", "NM" }, O_2ARG, 0b1001110000000000 },
{ {"ER", "ORN"}, O_2ARG, 0b1010000000000000 },
{ {"EM", "EM" }, O_2ARG, 0b1010010000000000 },
{ {"XR", "XR" }, O_2ARG, 0b1010100000000000 },
{ {"XM", "XM" }, O_2ARG, 0b1010110000000000 },
{ {"CL", "CL" }, O_2ARG, 0b1011000000000000 },
{ {"LB", "LB" }, O_2ARG, 0b1011010000000000 },
{ {"RB", "RB" }, O_2ARG, 0b1011100000000000 },
{ {"CB", "CB" }, O_2ARG, 0b1011110000000000 },

{ {"AWT", "ADT" }, O_KA1, 0b1100000000000000 },
{ {"TRB", "ADOT"}, O_KA1, 0b1100010000000000 },
{ {"IRB", "ADJT"}, O_KA1, 0b1100100000000000 },
{ {"DRB", "DRB"},  O_KA1, 0b1100110000000000 },
{ {"CWT", "COT" }, O_KA1, 0b1101000000000000 },
{ {"LWT", "LOT" }, O_KA1, 0b1101010000000000 },
{ {"LWS", "LTS" }, O_KA1, 0b1101100000000000 },
{ {"RWS", "STS" }, O_KA1, 0b1101110000000000 },

{ {"UJS", "JPT" }, O_JS, 0b1110000000000000 },
{ {"JLS", "JPTL"}, O_JS, 0b1110000001000000 },
{ {"JES", "JPTE"}, O_JS, 0b1110000010000000 },
{ {"JGS", "JPTG"}, O_JS, 0b1110000011000000 },
{ {"JVS", "JPTV"}, O_JS, 0b1110000100000000 },
{ {"JXS", "JPTX"}, O_JS, 0b1110000101000000 },
{ {"JYS", "JPTY"}, O_JS, 0b1110000110000000 },
{ {"JCS", "JCS" }, O_JS, 0b1110000111000000 },

{ {"BLC", "BLC"}, O_KA2, 0b1110010000000000 },
{ {"EXL", "EXL"}, O_KA2, 0b1110010100000000 },
{ {"BRC", "BRC"}, O_KA2, 0b1110011000000000 },
{ {"NRF", "NLZ"}, O_KA2, 0b1110011100000000 },

{ {"RIC", "RIC" }, O_C, 0b1110100000000000 },
{ {"ZLB", "ZLB" }, O_C, 0b1110100000000001 },
{ {"SXU", "STXA"}, O_C, 0b1110100000000010 },
{ {"NGA", "NEGA"}, O_C, 0b1110100000000011 },
{ {"SLZ", "SHL" }, O_C, 0b1110100000000100 },
{ {"SLY", "SHLY"}, O_C, 0b1110100000000101 },
{ {"SLX", "SHLX"}, O_C, 0b1110100000000110 },
{ {"SRY", "SHRY"}, O_C, 0b1110100000000111 },
{ {"NGL", "NEG" }, O_C, 0b1110100000001000 },
{ {"RPC", "RPC" }, O_C, 0b1110100000001001 },
{ {"SHC", "SHC" }, O_SHC, 0b1110100000010000 },
{ {"RKY", "RKEY"}, O_C, 0b1110101000000000 },
{ {"ZRB", "ZRB" }, O_C, 0b1110101000000001 },
{ {"SXL", "STXZ"}, O_C, 0b1110101000000010 },
{ {"NGC", "NEC" }, O_C, 0b1110101000000011 },
{ {"SVZ", "SHV" }, O_C, 0b1110101000000100 },
{ {"SVY", "SHVY"}, O_C, 0b1110101000000101 },
{ {"SVX", "SHVX"}, O_C, 0b1110101000000110 },
{ {"SRX", "SHRX"}, O_C, 0b1110101000000111 },
{ {"SRZ", "SHR" }, O_C, 0b1110101000001000 },
{ {"LPC", "LPC" }, O_C, 0b1110101000001001 },

{ {"HLT", "STOP"}, O_HLT, 0b1110110000000000 },
{ {"MCL", "MCL" }, O_S, 0b1110110001000000 },
{ {"CIT", "CIT" }, O_S, 0b1110110010000000 },
{ {"SIL", "SIL" }, O_S, 0b1110110010000001 },
{ {"SIU", "SIU" }, O_S, 0b1110110010000010 },
{ {"SIT", "SIT" }, O_S, 0b1110110010000011 },
{ {"GIU", "GIU" }, O_S, 0b1110110011000000 },
{ {"LIP", "LIP" }, O_S, 0b1110110100000000 },
{ {"GIL", "GIL" }, O_S, 0b1110111011000000 },

// fake UJS 0
{ {"NOP", "NOP"}, O_S, 0b1110000000000000 },

{ {"UJ", "JP" }, O_J, 0b1111000000000000 },
{ {"JL", "JPL"}, O_J, 0b1111000001000000 },
{ {"JE", "JPE"}, O_J, 0b1111000010000000 },
{ {"JG", "JPG"}, O_J, 0b1111000011000000 },
{ {"JZ", "JZ" }, O_J, 0b1111000100000000 },
{ {"JM", "JM" }, O_J, 0b1111000101000000 },
{ {"JN", "JN" }, O_J, 0b1111000110000000 },
{ {"LJ", "JPR"}, O_J, 0b1111000111000000 },

{ {"LD", "LDD" }, O_L, 0b1111010000000000 },
{ {"LF", "LDF" }, O_L, 0b1111010001000000 },
{ {"LA", "LDR" }, O_L, 0b1111010010000000 },
{ {"LL", "LDM" }, O_L, 0b1111010011000000 },
{ {"TD", "LDDB"}, O_L, 0b1111010100000000 },
{ {"TF", "LDFB"}, O_L, 0b1111010101000000 },
{ {"TA", "LDRB"}, O_L, 0b1111010110000000 },
{ {"TL", "LDMB"}, O_L, 0b1111010111000000 },

{ {"RD", "STD" }, O_G, 0b1111100000000000 },
{ {"RF", "STF" }, O_G, 0b1111100001000000 },
{ {"RA", "STR" }, O_G, 0b1111100010000000 },
{ {"RL", "STM" }, O_G, 0b1111100011000000 },
{ {"PD", "STDB"}, O_G, 0b1111100100000000 },
{ {"PF", "STFB"}, O_G, 0b1111100101000000 },
{ {"PA", "STRB"}, O_G, 0b1111100110000000 },
{ {"PL", "STMB"}, O_G, 0b1111100111000000 },

{ {"MB", "MB"  }, O_BN, 0b1111110000000000 },
{ {"IM", "IM"  }, O_BN, 0b1111110001000000 },
{ {"KI", "KI"  }, O_BN, 0b1111110010000000 },
{ {"FI", "FI"  }, O_BN, 0b1111110011000000 },
{ {"SP", "SP"  }, O_BN, 0b1111110100000000 },
{ {"MD", "MOD" }, O_BN, 0b1111110101000000 },
{ {"RZ", "ZS"  }, O_BN, 0b1111110110000000 },
{ {"IB", "ADOS"}, O_BN, 0b1111110111000000 },

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
