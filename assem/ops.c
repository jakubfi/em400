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
#include "parser_modern.h"

int mnemo_sel = 0;

struct op_t ops[] = {

{ {"LW", "LO"  }, OP_2ARG, 0b0100000000000000 },
{ {"TW", "LOB" }, OP_2ARG, 0b0100010000000000 },
{ {"LS", "LOM" }, OP_2ARG, 0b0100100000000000 },
{ {"RI", "LOS" }, OP_2ARG, 0b0100110000000000 },
{ {"RW", "ST"  }, OP_2ARG, 0b0101000000000000 },
{ {"PW", "STB" }, OP_2ARG, 0b0101010000000000 },
{ {"RJ", "JPAR"}, OP_2ARG, 0b0101100000000000 },
{ {"IS", "IS"  }, OP_2ARG, 0b0101110000000000 },
{ {"BB", "CLBO"}, OP_2ARG, 0b0110000000000000 },
{ {"BM", "BM"  }, OP_2ARG, 0b0110010000000000 },
{ {"BS", "CLMO"}, OP_2ARG, 0b0110100000000000 },
{ {"BC", "BC"  }, OP_2ARG, 0b0110110000000000 },
{ {"BN", "BN"  }, OP_2ARG, 0b0111000000000000 },
{ {"OU", "OU"  }, OP_2ARG, 0b0111010000000000 },
{ {"IN", "IN"  }, OP_2ARG, 0b0111100000000000 },

{ {"AD", "ADD"}, OP_FD, 0b0111110000000000 },
{ {"SD", "SD" }, OP_FD, 0b0111110001000000 },
{ {"MW", "MW" }, OP_FD, 0b0111110010000000 },
{ {"DW", "DW" }, OP_FD, 0b0111110011000000 },
{ {"AF", "ADF"}, OP_FD, 0b0111110100000000 },
{ {"SF", "SBF"}, OP_FD, 0b0111110101000000 },
{ {"MF", "MLF"}, OP_FD, 0b0111110110000000 },
{ {"DF", "DVF"}, OP_FD, 0b0111110111000000 },

{ {"AW", "AD" }, OP_2ARG, 0b1000000000000000 },
{ {"AC", "ADC"}, OP_2ARG, 0b1000010000000000 },
{ {"SW", "SU" }, OP_2ARG, 0b1000100000000000 },
{ {"CW", "CO" }, OP_2ARG, 0b1000110000000000 },
{ {"OR", "OR" }, OP_2ARG, 0b1001000000000000 },
{ {"OM", "OM" }, OP_2ARG, 0b1001010000000000 },
{ {"NR", "AND"}, OP_2ARG, 0b1001100000000000 },
{ {"NM", "NM" }, OP_2ARG, 0b1001110000000000 },
{ {"ER", "ORN"}, OP_2ARG, 0b1010000000000000 },
{ {"EM", "EM" }, OP_2ARG, 0b1010010000000000 },
{ {"XR", "XR" }, OP_2ARG, 0b1010100000000000 },
{ {"XM", "XM" }, OP_2ARG, 0b1010110000000000 },
{ {"CL", "CL" }, OP_2ARG, 0b1011000000000000 },
{ {"LB", "LB" }, OP_2ARG, 0b1011010000000000 },
{ {"RB", "WRB"}, OP_2ARG, 0b1011100000000000 },
{ {"CB", "CB" }, OP_2ARG, 0b1011110000000000 },

{ {"AWT", "ADT" }, OP_KA1, 0b1100000000000000 },
{ {"TRB", "ADOT"}, OP_KA1, 0b1100010000000000 },
{ {"IRB", "ADJT"}, OP_KA1, 0b1100100000000000 },
{ {"DRB", "ZDRB"}, OP_KA1, 0b1100110000000000 },
{ {"CWT", "COT" }, OP_KA1, 0b1101000000000000 },
{ {"LWT", "LOT" }, OP_KA1, 0b1101010000000000 },
{ {"LWS", "LTS" }, OP_KA1, 0b1101100000000000 },
{ {"RWS", "STS" }, OP_KA1, 0b1101110000000000 },

{ {"UJS", "JPT" }, OP_JS, 0b1110000000000000 },
{ {"JLS", "JPTL"}, OP_JS, 0b1110000001000000 },
{ {"JES", "JPTE"}, OP_JS, 0b1110000010000000 },
{ {"JGS", "JPTG"}, OP_JS, 0b1110000011000000 },
{ {"JVS", "JPTV"}, OP_JS, 0b1110000100000000 },
{ {"JXS", "JPTX"}, OP_JS, 0b1110000101000000 },
{ {"JYS", "JTPY"}, OP_JS, 0b1110000110000000 },
{ {"JCS", "JCS" }, OP_JS, 0b1110000111000000 },

{ {"BLC", "BLC"}, OP_KA2, 0b1110010000000000 },
{ {"EXL", "EXL"}, OP_KA2, 0b1110010100000000 },
{ {"BRC", "BRC"}, OP_KA2, 0b1110011000000000 },
{ {"NRF", "NLZ"}, OP_KA2, 0b1110011100000000 },

{ {"RIC", "RIC" }, OP_C, 0b1110100000000000 },
{ {"ZLB", "ZLB" }, OP_C, 0b1110100000000001 },
{ {"SXU", "STXA"}, OP_C, 0b1110100000000010 },
{ {"NGA", "NEGA"}, OP_C, 0b1110100000000011 },
{ {"SLZ", "SLZ" }, OP_C, 0b1110100000000100 },
{ {"SLY", "SHLY"}, OP_C, 0b1110100000000101 },
{ {"SLX", "SHLX"}, OP_C, 0b1110100000000110 },
{ {"SRY", "SRY" }, OP_C, 0b1110100000000111 },
{ {"NGL", "NEG" }, OP_C, 0b1110100000001000 },
{ {"RPC", "RPC" }, OP_C, 0b1110100000001001 },
{ {"SHC", "SHC" }, OP_SHC, 0b1110100000010000 },
{ {"RKY", "RKEY"}, OP_C, 0b1110101000000000 },
{ {"ZRB", "ZRB" }, OP_C, 0b1110101000000001 },
{ {"SXL", "STXZ"}, OP_C, 0b1110101000000010 },
{ {"NGC", "NEC" }, OP_C, 0b1110101000000011 },
{ {"SVZ", "SHV" }, OP_C, 0b1110101000000100 },
{ {"SVY", "SHVY"}, OP_C, 0b1110101000000101 },
{ {"SVX", "SHVX"}, OP_C, 0b1110101000000110 },
{ {"SRX", "SHRX"}, OP_C, 0b1110101000000111 },
{ {"SRZ", "SHR" }, OP_C, 0b1110101000001000 },
{ {"LPC", "LPC" }, OP_C, 0b1110101000001001 },

{ {"HLT", "STOP"}, OP_HLT, 0b1110110000000000 },
{ {"MCL", "MCL" }, OP_S, 0b1110110001000000 },
{ {"CIT", "CIT" }, OP_S, 0b1110110010000000 },
{ {"SIL", "SIL" }, OP_S, 0b1110110010000001 },
{ {"SIU", "SIU" }, OP_S, 0b1110110010000010 },
{ {"SIT", "SIT" }, OP_S, 0b1110110010000011 },
{ {"GIU", "GIU" }, OP_S, 0b1110110011000000 },
{ {"LIP", "LIP" }, OP_S, 0b1110110100000000 },
{ {"GIL", "GIL" }, OP_S, 0b1110111011000000 },

// fake UJS 0
{ {"NOP", "NOP"}, OP_S, 0b1110000000000000 },

{ {"UJ", "JP" }, OP_J, 0b1111000000000000 },
{ {"JL", "JPL"}, OP_J, 0b1111000001000000 },
{ {"JE", "JPE"}, OP_J, 0b1111000010000000 },
{ {"JG", "JPG"}, OP_J, 0b1111000011000000 },
{ {"JZ", "JZ" }, OP_J, 0b1111000100000000 },
{ {"JM", "JM" }, OP_J, 0b1111000101000000 },
{ {"JN", "JN" }, OP_J, 0b1111000110000000 },
{ {"LJ", "JPR"}, OP_J, 0b1111000111000000 },

{ {"LD", "LDD" }, OP_L, 0b1111010000000000 },
{ {"LF", "LDF" }, OP_L, 0b1111010001000000 },
{ {"LA", "LDR" }, OP_L, 0b1111010010000000 },
{ {"LL", "LDM" }, OP_L, 0b1111010011000000 },
{ {"TD", "LDDB"}, OP_L, 0b1111010100000000 },
{ {"TF", "LDFB"}, OP_L, 0b1111010101000000 },
{ {"TA", "LDRB"}, OP_L, 0b1111010110000000 },
{ {"TL", "LDMB"}, OP_L, 0b1111010111000000 },

{ {"RD", "STD" }, OP_G, 0b1111100000000000 },
{ {"RF", "STF" }, OP_G, 0b1111100001000000 },
{ {"RA", "STR" }, OP_G, 0b1111100010000000 },
{ {"RL", "STM" }, OP_G, 0b1111100011000000 },
{ {"PD", "STDB"}, OP_G, 0b1111100100000000 },
{ {"PF", "STFB"}, OP_G, 0b1111100101000000 },
{ {"PA", "STRB"}, OP_G, 0b1111100110000000 },
{ {"PL", "STMB"}, OP_G, 0b1111100111000000 },

{ {"MB", "MB"  }, OP_BN, 0b1111110000000000 },
{ {"IM", "IM"  }, OP_BN, 0b1111110001000000 },
{ {"KI", "KI"  }, OP_BN, 0b1111110010000000 },
{ {"FI", "FI"  }, OP_BN, 0b1111110011000000 },
{ {"SP", "SP"  }, OP_BN, 0b1111110100000000 },
{ {"MD", "MOD" }, OP_BN, 0b1111110101000000 },
{ {"RZ", "ZS"  }, OP_BN, 0b1111110110000000 },
{ {"IB", "ADOS"}, OP_BN, 0b1111110111000000 },

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
