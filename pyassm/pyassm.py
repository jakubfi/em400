#!/usr/bin/python

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

import sys
import re
from m400_assm import *


# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

asmfile = sys.argv[1]

ifile = open(asmfile, "r")
lineno = 1

prog = []

for line in ifile:
    line = line.strip()
    if line == "":
        continue
    res = re.match(r'^([\.a-zA-Z]+).*$', line)
    if not res:
        print "Syntax errpr at line %i" % lineno
        sys.exit(1)
    op = res.group(1).upper()
    if not op:
        print "Syntax error at line %i" % lineno
        sys.exit(1)
    try:
        fun = m400_assm[op][0]
    except:
        print "Unknown instruction '%s' at line %i" % (op, lineno)
        sys.exit(1)
    prog += fun(op, line)
    lineno += 1

ifile.close()

binfile = asmfile.replace(".asm", ".bin")
ofile = open(binfile, "w")
for i in prog:
    ofile.write(chr((i&0b1111111100000000)>>8))
    ofile.write(chr(i&0b0000000011111111))
ofile.close()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
