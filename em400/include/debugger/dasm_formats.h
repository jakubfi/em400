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

#ifndef DASM_FORMATS_H
#define DASM_FORMATS_H

/*
	Disassembler/translator output formatting:

	%I - mnemonic
	%E - mnemonic suffix
	%A - reg. A number
	%B - reg. B number
	%C - reg. C number
	%T - short arg.
	%t - very short arg. (SHC only)
	%b - byte arg.
	%N - norm. arg. (disassembler syntax)
	%n - norm. arg. (translator syntax)
	%0 - bin representation of a word
	%x - hex representation of a word
	%d - dec representation of a word
*/

#define DT_ILL	"? %x %d"
#define D_2ARGN	"%I%E r%A, %N"
#define D_1ARGN	"%I%E %N"
#define D_FD	D_1ARGN
#define D_KA1	"%I r%A, %T"
#define D_JS	"%I %T"
#define D_KA2	"%I %b"
#define D_C		"%I r%A"
#define D_SHC	"%I r%A, %t"
#define D_S		"%I"
#define D_J		D_1ARGN
#define D_L		D_1ARGN
#define D_G		D_1ARGN
#define D_BN	D_1ARGN

#define T_LW "r%A = %n"
#define T_TW "r%A = NB:%n"
#define T_LS "r%A = (r%A & ~r7) | (%n & r7)"
#define T_RI "[r%A] = %n\n\tr%A++"
#define T_RW "[%n] = r%A"
#define T_PW "NB:[%n] = r%A"
#define T_RJ "r%A = IC\n\tIC = %n"
#define T_IS "if (NB:[%n] & r%A == r%A) then skip\n\telse NB:%n |= r%A"
#define T_BB "if (r%A & %n == %n) then skip"
#define T_BM "if (NB:%n & r%A == r%A) then skip"
#define T_BS "if (r%A & r7 == %n & r7) then skip"
#define T_BC "if (r%A & %n != %n then skip"
#define T_BN "if (r%A & %n == 0) then skip"
#define T_OU "output(%n, r%A)"
#define T_IN "input(%n, r%A)"

#define T_AW "r%A += %n"
#define T_AC "r%A += %n + C"
#define T_SW "r%A -= %n"
#define T_CW "r%A ?? %n"
#define T_OR "r%A |= %n"
#define T_OM "NB:%n |= r%A"
#define T_NR "r%A &= %n"
#define T_NM "NB:%n &= r%A"
#define T_ER "r%A &= ~%n"
#define T_EM "NB:%n &= ~r%A"
#define T_XR "r%A %%= %n"
#define T_XM "NB:%n %%= r%A"
#define T_CL "r%A ?? %n"
#define T_LB "r%A[8-15] = NBb:%n"
#define T_RB "NBb:%n = r%A[8-15]"
#define T_CB "r%A[8-15] ?? NBb:%n"

#define T_AWT "r%A += %T"
#define T_TRB "r%A += %T\n\tif (r%A == 0) then skip"
#define T_IRB "r%A++\n\tif (r%A != 0) then IC += %T"
#define T_DRB "r%A--\n\tif (r%A != 0) then IC += %T"
#define T_CWT "r%A ?? %T"
#define T_LWT "r%A = %T"
#define T_LWS "r%A = [IC+%T]"
#define T_RWS "[IC+%T] = r%A"

#define T_AD "(r1, r2) += ([%n], [%n+1])"
#define T_SD "(r1, r2) -= ([%n], [%n+1])"
#define T_MW "(r1, r2) = r2 * [%n]"
#define T_DW "r2 = (r1, r2) / [%n]\n\tr1 = modulo"
#define T_AF "(r1, r2, r3) += ([%n], [%n+1], [%n+2])"
#define T_SF "(r1, r2, r3) -= ([%n], [%n+1], [%n+2])"
#define T_MF "(r1, r2, r3) *= ([%n], [%n+1], [%n+2])"
#define T_DF "(r1, r2, r3) /= ([%n], [%n+1], [%n+2])"

#define T_UJS "IC += %T"
#define T_JLS "if < then IC += %T"
#define T_JES "if == then IC += %T"
#define T_JGS "if > then IC += %T"
#define T_JVS "if V then IC += %T\n\tV = 0"
#define T_JXS "if X then IC += %T"
#define T_JYS "if Y then IC += %T"
#define T_JCS "if C then IC += %T"

#define T_BLC "if r0[0-7] & %b != %b then skip"
#define T_EXL "exl(%b)"
#define T_BRC "if r0[8-15] & %b != %b then skip"
#define T_NRF "normalize()"

#define T_RIC "r%A = IC"
#define T_ZLB "r%A[0-7] = 0"
#define T_SXU "X = r%A[0]"
#define T_NGA "r%A = -r%A"
#define T_SLZ "Y << r%A << 0"
#define T_SLY "Y << r%A << Y"
#define T_SLX "Y << r%A << X"
#define T_SRY "Y >> r%A >> Y"
#define T_NGL "r%A = ~r%A"
#define T_RPC "r%A = r0"
#define T_SHC "r%A >> r%A, %t >> r%A"
#define T_RKY "r%A = keys()"
#define T_ZRB "r%A[8-15] = 0"
#define T_SXL "X = r%A[15]"
#define T_NGC "r%A = ~(r%A + C)"
#define T_SVZ "YV << r%A << 0"
#define T_SVY "YV << r%A << Y"
#define T_SVX "YV << r%A << X"
#define T_SRX "X >> r%A >> Y"
#define T_SRZ "0 >> r%A >> Y"
#define T_LPC "r0 = r%A"

#define T_HLT "halt()"
#define T_MCL "master_clear()"
#define T_CIT "int_clear(SOFT_U | SOFT_L)"
#define T_SIL "int_set(SOFT_L)"
#define T_SIU "int_set(SOFT_U)"
#define T_SIT "int_set(SOFT_U | SOFT_L)"
#define T_GIU "RZ\"[3] = 1\n\tif OK then skip"
#define T_GIL "RZ\"[29] = 1\n\tif OK then skip"
#define T_LIP "pop (IC, r0, SR)"
#define T_SIX "int_set(EXTRA)"
#define T_CIX "int_clear(EXTRA)"

#define T_UJ "IC = %n"
#define T_JL "if < then IC = %n"
#define T_JE "if == then IC = %5"
#define T_JG "if > then IC = %n"
#define T_JZ "if Z then IC = %n"
#define T_JM "if <0 then IC = %n"
#define T_JN "if != then IC = %n"
#define T_LJ "[%n] = IC\n\tIC = %n+1"

#define T_LD "(r1, r2) = ([%n], [%n+1])"
#define T_LF "(r1, r2, r3) = ([%n], [%n+1], [%n+2])"
#define T_LA "(r1...r7) = ([%n]...[%n+6])"
#define T_LL "(r5, r6, r7) = ([%n], [%n+1], [%n+2])"
#define T_TD "(r1, r2) = (NB:[%n], NB:[%n+1])"
#define T_TF "(r1, r2, r3) = (NB:[%n], NB:[%n+1], NB:[%n+2])"
#define T_TA "(r1...r7) = (NB:[%n]...NB:[%n+6])"
#define T_TL "(r5, r6, r7) = (NB:[%n], NB:[%n+1], NB:[%n+2])"

#define T_RD "([%n], [%n+1]) = (r1, r2)"
#define T_RF "([%n], [%n+1], [%n+2]) = (r1, r2, r3)"
#define T_RA "([%n]...[%n+6]) = (r1...r7)"
#define T_RL "([%n], [%n+1], [%n+2]) = (r5, r6, r7)"
#define T_PD "(NB:[%n], NB:[%n+1]) = (r1, r2)"
#define T_PF "(NB:[%n], NB:[%n+1], NB:[%n+2]) = (r1, r2, r3)"
#define T_PA "(NB:[%n]...NB:[%n+6]) = (r1...r7)"
#define T_PL "(NB:[%n], NB:[%n+1], NB:[%n+2]) = (r5, r6, r7)"

#define T_MB "MB = [%n][10-15]"
#define T_IM "IM = [%n][0-9]"
#define T_KI "[%n] = RZ[0-11,28-31]"
#define T_FI "RZ[0-11,28-31] = [%n]"
#define T_SP "(IC, r0, SR) = (NB:[%n], NB:[%n+1], NB:[%n+2])"
#define T_MD "mod(%n)"
#define T_RZ "[%n] = 0"
#define T_IB "[%n]++\n\tif ([%n] == 0) then skip"

#endif

// vim: tabstop=4
