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

#include "nodes.h"
#include "keywords.h"
#include "parser_classic.h"

int mnemo_sel = MERA400;

struct var_t extracodes[] = {
{ 0200, "ASG"},
{ 0201, "CASG"},
{ 0202, "SETP"},
{ 0203, "LOAP"},
{ 0204, "TMEM"},
{ 0205, "NASG"},
{ 0206, "ERF"},
{ 0207, "ERS"},
{ 0210, "ERAS"},
{ 0211, "FBOF"},
{ 0212, "INPR"},
{ 0213, "PRIR"},
{ 0214, "PINP"},
{ 0215, "PRIN"},
{ 0216, "EOF"},
{ 0217, "FEOF"},
{ 0220, "INAM"},
{ 0221, "INUM"},
{ 0222, "WADR"},
{ 0223, "OVL"},
{ 0224, "REAP"},
{ 0225, "WRIP"},
{ 0226, "READ"},
{ 0227, "WRIT"},
{ 0230, "OES"},
{ 0231, "ERR"},
{ 0232, "CORE"},
{ 0233, "CPRF"},
{ 0234, "JUMP"},
{ 0235, "SDIR"},
{ 0236, "TDIR"},
{ 0237, "CDIR"},
{ 0240, "DEFP"},
{ 0241, "DELP"},
{ 0242, "SREG"},
{ 0243, "TREG"},
{ 0244, "RUNP"},
{ 0245, "HANG"},
{ 0246, "TERR"},
{ 0247, "WAIT"},
{ 0250, "STOP"},
{ 0251, "RELD"},
{ 0252, "DATE"},
{ 0253, "TIME"},
{ 0255, "CHPI"},
{ 0256, "WAIS"},
{ 0257, "SIGN"},
{ 0260, "TLAB"},
{ 0261, "PINF"},
{ 0262, "CSUM"},
{ 0263, "CSYS"},
{ 0264, "UNL"},
{ 0265, "LOD"},
{ 0266, "TAKS"},
{ 0267, "RELS"},
{ 0270, "GMEM"},
{ 0271, "RMEM"},
{ 0272, "LRAM"},
{ 0275, "OPPI"},
{ 0276, "WFPI"},
{ 0277, "CAMAC"},
{ 0300, "RWMT"},
{ 0301, "FBMT"},
{ 0302, "FFMT"},
{ 0303, "BBMT"},
{ 0304, "BFMT"},
{ 0305, "FMMT"},
{ 0306, "REMT"},
{ 0307, "WRMT"},
{ 0371, "SCON"},
{ 0372, "TCON"},
{ 0373, "END"},
{ 0374, "BACK"},
{ 0375, "ABO"},
{ 0376, "KILL"},
{ 0377, "EOSL"},

{ 0, NULL}
};

struct kw_t pragmas[] = {
{ {".prog",		"PROG*"},		P_PROG,			N_PROG },
{ {".finprog",	"FINPROG*"},	P_FINPROG,		N_FINPROG },
{ {".seg",		"SEG*"},		P_SEG,			N_SEG },
{ {".finseg",	"FINSEG*"},		P_FINSEG,		N_FINSEG },
{ {".macro",	"MACRO*"},		P_MACRO,		N_MACRO },
{ {".finmacro",	"FINMACRO*"},	P_FINMACRO,		N_FINMACRO },

{ {".data",		""},			P_DATA,			N_LABEL },
{ {".equ",		""},			P_EQU,			N_VAR },
{ {".res",		"RES*"},		P_RES,			N_RES },

{ {".s",		"S*"},			P_S,			N_SETIC },
{ {"",			"F*"},			P_F,			N_ERR },
{ {"@",			"ALL*"},		P_ALL,			N_ERR },
{ {"",			"NAME*"},		P_NAME,			N_ERR },
{ {"",			"BA*"},			P_BA,			N_ERR },
{ {"",			"INT*"},		P_INT,			N_ERR },
{ {"",			"OUT*"},		P_OUT,			N_ERR },
{ {"",			"LAB*"},		P_LAB,			N_ERR },
{ {"",			"NLAB*"},		P_NLAB,			N_ERR },
{ {"",			"MEM*"},		P_MEM,			N_ERR },
{ {"",			"OS*"},			P_OS,			N_ERR },
{ {".ifunk",	"IFUNK*"},		P_IFUNK,		N_IFUNK },
{ {"",			"IFUND*"},		P_IFUND,		N_IFUND },
{ {".ifdef",	"IFDEF*"},		P_IFDEF,		N_IFDEF },
{ {".fi",		"FI*"},			P_FI,			N_FI },
{ {".ovl",		"SS*"},			P_SS,			N_OVL },
{ {"",			"HS*"},			P_HS,			N_ERR },
{ {".max",		"MAX*"},		P_MAX,			N_ERR },
{ {".len",		"LEN*"},		P_LEN,			N_ERR },
{ {"",			"E*"},			P_E,			N_ERR },
{ {".file",		"FILE*"},		P_FILE,			N_ERR },
{ {".text",		"TEXT*"},		P_TEXT,			N_TEXT },
{ {NULL,		NULL},			0,				0 }
};

struct kw_t ops[] = {

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
{ {"RB", "RB" }, OP_2ARG, 0b1011100000000000 },
{ {"CB", "CB" }, OP_2ARG, 0b1011110000000000 },

{ {"AWT", "ADT" }, OP_KA1, 0b1100000000000000 },
{ {"TRB", "ADOT"}, OP_KA1, 0b1100010000000000 },
{ {"IRB", "ADJT"}, OP_KA1, 0b1100100000000000 },
{ {"DRB", "DRB"},  OP_KA1, 0b1100110000000000 },
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
{ {"JYS", "JPTY"}, OP_JS, 0b1110000110000000 },
{ {"JCS", "JCS" }, OP_JS, 0b1110000111000000 },

{ {"BLC", "BLC"}, OP_BLC, 0b1110010000000000 },
{ {"EXL", "EXL"}, OP_EXL, 0b1110010100000000 },
{ {"BRC", "BRC"}, OP_BRC, 0b1110011000000000 },
{ {"NRF", "NLZ"}, OP_S, 0b1110011100000000 },

{ {"RIC", "RIC" }, OP_C, 0b1110100000000000 },
{ {"ZLB", "ZLB" }, OP_C, 0b1110100000000001 },
{ {"SXU", "STXA"}, OP_C, 0b1110100000000010 },
{ {"NGA", "NEGA"}, OP_C, 0b1110100000000011 },
{ {"SLZ", "SHL" }, OP_C, 0b1110100000000100 },
{ {"SLY", "SHLY"}, OP_C, 0b1110100000000101 },
{ {"SLX", "SHLX"}, OP_C, 0b1110100000000110 },
{ {"SRY", "SHRY"}, OP_C, 0b1110100000000111 },
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
struct kw_t * get_op(int set, char *name)
{
	return get_kw(ops, set, name);
}

// -----------------------------------------------------------------------
struct kw_t * get_pragma(int set, char *name)
{
	return get_kw(pragmas, set, name);
}

// -----------------------------------------------------------------------
struct kw_t * get_kw(struct kw_t *dict, int set, char *name)
{
	struct kw_t *kw = dict;

	while (kw->mnemo[set]) {
		if (!strcasecmp(kw->mnemo[set], name)) {
			return kw;
		}
		kw++;
	}

	return NULL;
}

// -----------------------------------------------------------------------
struct var_t * get_pvar(struct var_t *d, char *name)
{
	struct var_t *pv = d;

	while (name && pv->name) {
		if (!strcasecmp(pv->name, name)) {
			return pv;
		}
		pv++;
	}

	return NULL;
}

// vim: tabstop=4
