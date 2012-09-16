#!/usr/bin/python

#  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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
#   along with this program; if not, write to the Free Software
#  Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

import sys
from m400_instr import *
from m400_utils import *

class M400dasm:

    # ------------------------------------------------------------------------
    def __init__(self, f, p):
        ifile = open(f, "r")
        self.img = bytearray(ifile.read())
        ifile.close()
        self.ic = -1 + p;
        self.pos = 0 + 2*p;

    # ------------------------------------------------------------------------
    def m400_decode_norm(self, i, group, d, a, b, c):
        if c == 0:
            m = self.m400_fetch()
            rc = "%i" % m
        else:
            rc = "r%i" % c
            m = 0

        if b != 0:
            rb = "+r%s" % b
        else:
            rb = ""

        if group == OP_NORM2:
            ra = "r%i, " % a
        else:
            ra = ""

        debug = "m=%-4x" % (m)

        if d == 0:
            args = "%s%s%s" % (ra, rc, rb)
        else:
            args = "%s[%s%s]" % (ra, rc, rb)

        if i == 076:
            args = "[%s]" % (args)

        return args, debug


    # ------------------------------------------------------------------------
    def m400_decode_short2_arg(self, i, d, a, b, c):
        t = (b<<3) | c
        if d == 1:
            t = -t
        debug = "t=%-4x" % (t)
        args = "r%i, %i" % (a, t)
        return args, debug

    # ------------------------------------------------------------------------
    def m400_decode_short1_arg(self, i, d, a, b, c):
        t = (b<<3) | c
        if d == 1:
            t = -t
        debug = "t=%-4x" % (t)
        args = "%i" % (t)
        return args, debug

    # ------------------------------------------------------------------------
    def m400_decode_byte_arg(self, i, d, a, b, c):
        b = ((a&0b011)<3) | (b<<3) | c
        debug = "byte=%-4x" % (b)
        args = "%i" % (b)
        return args, debug

    # ------------------------------------------------------------------------
    def m400_decode_no_arg(self, i, d, a, b, c):
        return "", ""

    # ------------------------------------------------------------------------
    def m400_decode_no2_arg(self, i, d, a, b, c):
        if (b == 0b010): # SHC
            T = (d<<3) | (c)
            args = "r%i, %i" % (a, T)
        else:
            args = "r%i" % (a)
        return args, ""

    # ------------------------------------------------------------------------
    def m400_decode(self, word):

        i = (word & 0b1111110000000000) >> 10
        d = (word & 0b0000001000000000) >> 9
        a = (word & 0b0000000111000000) >> 6
        b = (word & 0b0000000000111000) >> 3
        c = (word & 0b0000000000000111) >> 0

        addr = self.ic
        debug_b = "# %i  %s %s %s (%i %i %i)" % (d, zbin(a,3), zbin(b,3), zbin(c,3), a, b, c)

        if not i in m400_opcodes:
            code = "---"
            args = zbin(i, 6)
            debug_b = "# %s %s" % (zbin(word>>8, 8), zbin(word&255, 8))
            desc = "# %2x %2x" % (word>>8, word&255)
            debug = "."

        else:
            code, group, desc = m400_get_opcode(i, d, a, b, c)
            desc = "# %s" % desc

            # 2-arg opcode with normal arg
            if group == OP_NORM2:
                args, debug = self.m400_decode_norm(i, group, d, a, b, c)

            # 1-arg opcode with normal arg
            elif group == OP_NORM1:
                args, debug = self.m400_decode_norm(i, group, d, a, b, c)

            # opcode with A arg and 7-bit arg
            elif group == OP_SHORT2:
                args, debug = self.m400_decode_short2_arg(i, d, a, b, c)

            # opcode with 7-bit arg only
            elif group == OP_SHORT1:
                args, debug = self.m400_decode_short1_arg(i, d, a, b, c)

            # opcode with 8-bit arg
            elif group == OP_BYTE:
                args, debug = self.m400_decode_byte_arg(i, d, a, b, c)

            # opcode with no arg
            elif group == OP_NO:
                args, debug = self.m400_decode_no_arg(i, d, a, b, c)

            # opcode with no second arg
            elif group == OP_NO2:
                args, debug = self.m400_decode_no2_arg(i, d, a, b, c)

            # unknown group
            else:
                code = "UNKNOWN"
                args = "GROUP"
                desc = ""
                debug = ""

        print "0x%04x: %-7s %-15s %-25s %-10s %-30s" % (addr, code, args, debug_b, debug, desc)
            
    # --------------------------------------------------------------------
    def m400_fetch(self):
        b1 = self.img[self.pos]
        b2 = self.img[self.pos+1]
        self.pos += 2
        self.ic += 1
        word = (b1 << 8) | b2
        return word

    # --------------------------------------------------------------------
    def go(self):
        while True:
            self.m400_decode(self.m400_fetch())
 

# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

M400dasm(sys.argv[1], int(sys.argv[2])).go()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
