#!/usr/bin/python
# -*- coding: UTF-* -*-

import sys
import re

trans_table = {
"_a" : "ą",
"_c" : "ć",
"_e" : "ę",
"_l" : "ł",
"_n" : "ń",
"_o" : "ó",
"_s" : "ś",
"_z" : "ż",
"_x" : "ż",
"_A" : "Ą",
"_C" : "Ć",
"_E" : "Ę",
"_L" : "Ł",
"_N" : "Ń",
"_O" : "Ó",
"_S" : "Ś",
"_Z" : "Ż",

"_\*" : "",
"_'" : "",
"_!![ZPWAOGXSTLQ][0-9]+;" : "",
"_>" : "",
"_0" : "",
"_1" : " ",
"_2" : "  ",
"_3" : "   ",
"_4" : "    ",
"_5" : "    ",
"_6" : "    ",
"_7" : "    ",
"_8" : "    ",
"_\." : ".",
"_\+" : "",
"_\," : ",",
"_\(" : "",
#"_\ " : " ",
"_\:" : ":",
"_\?" : "?",
"_\[" : "[",
"_\]" : "]",

"__" : "_",
}

def trans_line(line, table):
    for rule in table:
        line, count = re.subn(rule, table[rule], line)
    return line

# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

in_file = sys.argv[1]

f = open(in_file, "r")

limit = -1

for line in f:
    if limit == 0:
        break
    print trans_line(line, trans_table),
    limit -= 1

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

