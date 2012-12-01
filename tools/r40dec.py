#!/usr/bin/python

import sys
from m400lib import r40

infile = sys.argv[1]

f = open(infile, "r")
a = 0
buf = ""
while f:
	c1 = ord(f.read(1))
	c2 = ord(f.read(1))
	c = (c1 << 8) | c2
	if a%24 == 0:
		buf += "%4x: " % a
	buf += r40(c)
	if a%24 == 22:
		print buf
		buf = ""
	a += 2

f.close()
