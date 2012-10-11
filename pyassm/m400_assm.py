#  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

import re

# LW  r1, r2
# LW  r1, r2+r3
# LW  r1, 3
# LW  r1, 3+r3
#
# LW  r1, [r2]
# LW  r1, [r2+r3]
# LW  r1, [3]
# LW  r1, [3+r1]

re_narg = r'(\[)?(r([0-9]+))?([0-9]+)?\]?(\+r([0-9]+))?\]?'

def m400_assm_2argn(op, line):
    op_bin = [m400_assm[op][1]]
    res = re.match(r'^([a-zA-Z]+)[ \t]+r([0-9]+)[ \t]*,[ \t]*' + re_narg + '$', line)
    n_a = res.group(2)
    n_d = res.group(3)
    n_c = res.group(5)
    n_m = res.group(6)
    n_b = res.group(8)
    op_bin[0] |= (int(n_a)<<6)
    if n_d:
        op_bin[0] |= 0b0000001000000000
    if n_c:
        op_bin[0] |= (int(n_c)<<3)
    if n_m:
        op_bin.append(int(n_m))
    if n_b:
        op_bin[0] |= int(n_b)
    return op_bin

def m400_assm_no1argn(op, line):
    print "no1arg2"

def m400_assm_ka2arg(op, line):
    print "ka2arg"

def m400_assm_ka(op, line):
    print "ka"

def m400_assm_barg(op, line):
    print "byte arg"

def m400_assm_1arg(op, line):
    print "1arg"

def m400_assm_shc(op, line):
    print "shc"

def m400_assm_noarg(op, line):
    print "noarg"

def m400_assm_data(op, line):
    res = re.match(r'.data[ \t]([0-9]+)', line)
    return [int(res.group(1))]

m400_assm = {
#          ...,,,DaaaBBBccc
'.DATA': [ m400_assm_data, 0 ],
'LW':  [ m400_assm_2argn, 0b0100000000000000 ],
'TW':  [ m400_assm_2argn, 0b0100010000000000 ],
'LS':  [ m400_assm_2argn, 0b0100100000000000 ],
'RI':  [ m400_assm_2argn, 0b0100110000000000 ],
'RW':  [ m400_assm_2argn, 0b0101000000000000 ],
'PW':  [ m400_assm_2argn, 0b0101010000000000 ],
'RJ':  [ m400_assm_2argn, 0b0101100000000000 ],
'IS':  [ m400_assm_2argn, 0b0101110000000000 ],
'BB':  [ m400_assm_2argn, 0b0110000000000000 ],
'BM':  [ m400_assm_2argn, 0b0110010000000000 ],
'BS':  [ m400_assm_2argn, 0b0110100000000000 ],
'BC':  [ m400_assm_2argn, 0b0110110000000000 ],
'BN':  [ m400_assm_2argn, 0b0111000000000000 ],
'OU':  [ m400_assm_2argn, 0b0111010000000000 ],
'IN':  [ m400_assm_2argn, 0b0111100000000000 ],

#          ...,,,DaaaBBBccc
'AD':  [ m400_assm_no1argn, 0b0111110000000000 ],
'SD':  [ m400_assm_no1argn, 0b0111110001000000 ],
'MW':  [ m400_assm_no1argn, 0b0111110010000000 ],
'DW':  [ m400_assm_no1argn, 0b0111110011000000 ],
'AF':  [ m400_assm_no1argn, 0b0111110100000000 ],
'SF':  [ m400_assm_no1argn, 0b0111110101000000 ],
'MF':  [ m400_assm_no1argn, 0b0111110110000000 ],
'DF':  [ m400_assm_no1argn, 0b0111110111000000 ],

#          ...,,,DaaaBBBccc
'AW':  [ m400_assm_2argn, 0b1000000000000000 ],
'AC':  [ m400_assm_2argn, 0b1000010000000000 ],
'SW':  [ m400_assm_2argn, 0b1000100000000000 ],
'CW':  [ m400_assm_2argn, 0b1000110000000000 ],
'OR':  [ m400_assm_2argn, 0b1001000000000000 ],
'OM':  [ m400_assm_2argn, 0b1001010000000000 ],
'NR':  [ m400_assm_2argn, 0b1001100000000000 ],
'NM':  [ m400_assm_2argn, 0b1001110000000000 ],
'ER':  [ m400_assm_2argn, 0b1010000000000000 ],
'EM':  [ m400_assm_2argn, 0b1010010000000000 ],
'XR':  [ m400_assm_2argn, 0b1010100000000000 ],
'XM':  [ m400_assm_2argn, 0b1010110000000000 ],
'CL':  [ m400_assm_2argn, 0b1011000000000000 ],
'LB':  [ m400_assm_2argn, 0b1011010000000000 ],
'RB':  [ m400_assm_2argn, 0b1011100000000000 ],
'CB':  [ m400_assm_2argn, 0b1011110000000000 ],

#          ...,,,DaaaBBBccc
'AWT': [ m400_assm_ka2arg, 0b1100000000000000 ],
'TRB': [ m400_assm_ka2arg, 0b1100010000000000 ],
'IRB': [ m400_assm_ka2arg, 0b1100100000000000 ],
'DRB': [ m400_assm_ka2arg, 0b1100110000000000 ],
'CWT': [ m400_assm_ka2arg, 0b1101000000000000 ],
'LWT': [ m400_assm_ka2arg, 0b1101010000000000 ],
'LWS': [ m400_assm_ka2arg, 0b1101100000000000 ],
'RWS': [ m400_assm_ka2arg, 0b1101110000000000 ],

#          ...,,,DaaaBBBccc
'UJS': [ m400_assm_ka, 0b1110000000000000 ],
'JLS': [ m400_assm_ka, 0b1110000001000000 ],
'JES': [ m400_assm_ka, 0b1110000010000000 ],
'JGS': [ m400_assm_ka, 0b1110000011000000 ],
'JVS': [ m400_assm_ka, 0b1110000100000000 ],
'JXS': [ m400_assm_ka, 0b1110000101000000 ],
'JYS': [ m400_assm_ka, 0b1110000110000000 ],
'JCS': [ m400_assm_ka, 0b1110000111000000 ],

#          ...,,,DaaaBBBccc
'BLC': [ m400_assm_barg, 0b1110010000000000 ],
'EXL': [ m400_assm_barg, 0b1110010100000000 ],
'BRC': [ m400_assm_barg, 0b1110011000000000 ],
'NRF': [ m400_assm_barg, 0b1110011100000000 ],

#          ...,,,DaaaBBBccc
'RIC': [ m400_assm_1arg, 0b1110100000000000 ],
'ZLB': [ m400_assm_1arg, 0b1110100000000001 ],
'SXU': [ m400_assm_1arg, 0b1110100000000010 ],
'NGA': [ m400_assm_1arg, 0b1110100000000011 ],
'SLZ': [ m400_assm_1arg, 0b1110100000000100 ],
'SLY': [ m400_assm_1arg, 0b1110100000000101 ],
'SLX': [ m400_assm_1arg, 0b1110100000000110 ],
'SRY': [ m400_assm_1arg, 0b1110100000000111 ],
'NGL': [ m400_assm_1arg, 0b1110100000001000 ],
'RPC': [ m400_assm_1arg, 0b1110100000001001 ],
'SHC': [ m400_assm_shc, 0b1110100000010000 ],
'RKY': [ m400_assm_1arg, 0b1110101000000000 ],
'ZRB': [ m400_assm_1arg, 0b1110101000000001 ],
'SXL': [ m400_assm_1arg, 0b1110101000000010 ],
'NGC': [ m400_assm_1arg, 0b1110101000000011 ],
'SVZ': [ m400_assm_1arg, 0b1110101000000100 ],
'SVY': [ m400_assm_1arg, 0b1110101000000101 ],
'SVX': [ m400_assm_1arg, 0b1110101000000110 ],
'SRX': [ m400_assm_1arg, 0b1110101000000111 ],
'SRZ': [ m400_assm_1arg, 0b1110101000001000 ],
'LPC': [ m400_assm_1arg, 0b1110101000001001 ],

#          ...,,,DaaaBBBccc
'HLT': [ m400_assm_noarg, 0b1110110000000000 ],
'MCL': [ m400_assm_noarg, 0b1110110001000000 ],
'CIT': [ m400_assm_noarg, 0b1110110010000000 ],
'SIL': [ m400_assm_noarg, 0b1110110010000001 ],
'SIU': [ m400_assm_noarg, 0b1110110010000010 ],
'SIT': [ m400_assm_noarg, 0b1110110010000011 ],
'GIU': [ m400_assm_noarg, 0b1110110011000000 ],
'LIP': [ m400_assm_noarg, 0b1110110100000000 ],
'GIL': [ m400_assm_noarg, 0b1110111011000000 ],

#          ...,,,DaaaBBBccc
'UJ':  [ m400_assm_no1argn, 0b1111000000000000 ],
'JL':  [ m400_assm_no1argn, 0b1111000001000000 ],
'JE':  [ m400_assm_no1argn, 0b1111000010000000 ],
'JG':  [ m400_assm_no1argn, 0b1111000011000000 ],
'JZ':  [ m400_assm_no1argn, 0b1111000100000000 ],
'JM':  [ m400_assm_no1argn, 0b1111000101000000 ],
'JN':  [ m400_assm_no1argn, 0b1111000110000000 ],
'LJ':  [ m400_assm_no1argn, 0b1111000111000000 ],

#          ...,,,DaaaBBBccc
'LD':  [ m400_assm_no1argn, 0b1111010000000000 ],
'LF':  [ m400_assm_no1argn, 0b1111010001000000 ],
'LA':  [ m400_assm_no1argn, 0b1111010010000000 ],
'LL':  [ m400_assm_no1argn, 0b1111010011000000 ],
'TD':  [ m400_assm_no1argn, 0b1111010100000000 ],
'TF':  [ m400_assm_no1argn, 0b1111010101000000 ],
'TA':  [ m400_assm_no1argn, 0b1111010110000000 ],
'TL':  [ m400_assm_no1argn, 0b1111010111000000 ],

#          ...,,,DaaaBBBccc
'RD':  [ m400_assm_no1argn, 0b1111100000000000 ],
'RF':  [ m400_assm_no1argn, 0b1111100001000000 ],
'RA':  [ m400_assm_no1argn, 0b1111100010000000 ],
'RL':  [ m400_assm_no1argn, 0b1111100011000000 ],
'PD':  [ m400_assm_no1argn, 0b1111100100000000 ],
'PF':  [ m400_assm_no1argn, 0b1111100101000000 ],
'PA':  [ m400_assm_no1argn, 0b1111100110000000 ],
'PL':  [ m400_assm_no1argn, 0b1111100111000000 ],

#          ...,,,DaaaBBBccc
'MB':  [ m400_assm_no1argn, 0b1111110000000000 ],
'IM':  [ m400_assm_no1argn, 0b1111110001000000 ],
'KI':  [ m400_assm_no1argn, 0b1111110010000000 ],
'FI':  [ m400_assm_no1argn, 0b1111110011000000 ],
'SP':  [ m400_assm_no1argn, 0b1111110100000000 ],
'MD':  [ m400_assm_no1argn, 0b1111110101000000 ],
'RZ':  [ m400_assm_no1argn, 0b1111110110000000 ],
'IB':  [ m400_assm_no1argn, 0b1111110111000000 ],
'': []

}


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
