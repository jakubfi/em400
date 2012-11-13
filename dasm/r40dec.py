#!/usr/bin/python

import sys


def r2a(i):
	if (i>=1) and (i<=26):
		return chr(i+64)
	if (i>=27) and (i<=36):
		return chr(i+21)
	if i==37:
		return "_"
	if i==38:
		return "%"
	if i==39:
		return ":"
	return "."

def r40(w):
	kz1 = r2a((w/1600) % 40)
	kz2 = r2a((w/40) % 40)
	kz3 = r2a(w % 40)
	return "%s%s%s" % (kz1, kz2, kz3)

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
