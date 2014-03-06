#!/usr/bin/python

import sys
from m400lib import r40

infile = sys.argv[1]

f = open(infile, "r")
a = 0
buf = ""
while f:
	if a%24 == 0:
		print "0x%04x:" % a,
	c1 = ord(f.read(1))
	c2 = ord(f.read(1))
	c = (c1 << 8) | c2
	print r40(c) + " |",
	a += 2
	if a%24 == 0:
		print ""

f.close()
