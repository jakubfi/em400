#!/usr/bin/python

#  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

import sys
from m400_instr import *
from m400_utils import *

class M400dasm:

    # ------------------------------------------------------------------------
    def __init__(self, f, mode):
        ifile = open(f, "r")
        self.img = bytearray(ifile.read())
        ifile.close()
        self.mode = mode
        self.ic = -1
        self.pos = 0

    # ------------------------------------------------------------------------
    def m400_decode_norm(self, i, group, d, a, b, c):

        # argument is in next word
        if c == 0:
            m = self.m400_fetch()
            rc = "0x%x" % m
        # use rC as the argument
        else:
            rc = "r%i" % c
            m = 0
        # B-modification
        if b != 0:
            rb = "+r%s" % b
        else:
            rb = ""

        # NORM1 and NORM2 are decoded here, but group 037 (NORM1) uses A for opcode extension
        if group == OP_NORM2:
            ra = "r%i, " % a
        else:
            ra = ""

        # D-modification
        if d == 0:
            args = "%s%s%s" % (ra, rc, rb)
        else:
            args = "%s[%s%s]" % (ra, rc, rb)

        # for group 076 we have 2-level addressing
        if i == 076:
            args = "[%s]" % (args)

        return args

    # ------------------------------------------------------------------------
    def m400_decode_short2_arg(self, i, d, a, b, c):
        # B and C stores the value
        t = (b<<3) | c

        # D stores the sign
        if d == 1:
            t = -t
        args = "r%i, %i" % (a, t)
        return args

    # ------------------------------------------------------------------------
    def m400_decode_short1_arg(self, i, d, a, b, c):
        t = (b<<3) | c
        if d == 1:
            t = -t
        args = "%i" % (t)
        return args

    # ------------------------------------------------------------------------
    def m400_decode_byte_arg(self, i, d, a, b, c):
        b = ((a&0b011)<<6) | (b<<3) | c
        args = "%i" % (b)
        return args

    # ------------------------------------------------------------------------
    def m400_decode_no_arg(self, i, d, a, b, c):
        return ""

    # ------------------------------------------------------------------------
    def m400_decode_no2_arg(self, i, d, a, b, c):
        if (b == 0b010): # SHC
            T = (d<<3) | (c)
            args = "r%i, %i" % (a, T)
        else:
            args = "r%i" % (a)
        return args

    # ------------------------------------------------------------------------
    def m400_decode_sin(self, i, d, a, b, c):
        args = "%i" % c
        return args

    # ------------------------------------------------------------------------
    def m400_decode(self, word):

        i = (word & 0b1111110000000000) >> 10
        d = (word & 0b0000001000000000) >> 9
        a = (word & 0b0000000111000000) >> 6
        b = (word & 0b0000000000111000) >> 3
        c = (word & 0b0000000000000111) >> 0

        addr = self.ic

        code, group, desc = m400_get_opcode(i, d, a, b, c, self.mode)

        # 2-arg opcode with normal arg
        if group == OP_NORM2:
            args = self.m400_decode_norm(i, group, d, a, b, c)

        # 1-arg opcode with normal arg
        elif group == OP_NORM1:
            args = self.m400_decode_norm(i, group, d, a, b, c)

        # opcode with A arg and 7-bit arg
        elif group == OP_SHORT2:
            args = self.m400_decode_short2_arg(i, d, a, b, c)

        # opcode with 7-bit arg only
        elif group == OP_SHORT1:
            args = self.m400_decode_short1_arg(i, d, a, b, c)
            # those are jumps, calculate destination address for convenience
            args = "%-3s -> 0x%04x" % (args, addr + 1 + int(args))

        # opcode with 8-bit arg
        elif group == OP_BYTE:
            args = self.m400_decode_byte_arg(i, d, a, b, c)

        # opcode with no arg
        elif group == OP_NO:
            # HLT (A=0) uses T as the halt reason, just as SHORT1
            if a == 0:
                args = self.m400_decode_short1_arg(i, d, a, b, c)
            else:
                args = self.m400_decode_no_arg(i, d, a, b, c)

        # opcode with no second arg
        elif group == OP_NO2:
            args = self.m400_decode_no2_arg(i, d, a, b, c)

        # sin is treated as a special case
        elif group == OP_SIN:
            args = self.m400_decode_sin(i, d, a, b, c)

        # unknown group
        else:
            args = "%04x" % (word)

        print "0x%04x: %-5s %-15s    # 0x%04x  %-30s" % (addr, code, args, word, desc)
            
    # --------------------------------------------------------------------
    def m400_fetch(self):
        b1 = self.img[self.pos]
        b2 = self.img[self.pos+1]
        word = (b1 << 8) | b2
        self.pos += 2
        self.ic += 1
        return word

    # --------------------------------------------------------------------
    def go(self):
        while self.pos < len(self.img):
            self.m400_decode(self.m400_fetch())
 

# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

binfile = sys.argv[1]

#dasm = M400dasm(binfile, MODE_K202)
dasm = M400dasm(binfile, MODE_MERA400)
dasm.go()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
