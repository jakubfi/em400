#!/usr/bin/python

import sys
from m400lib import r40, wload

def fil_print(data, o):
	fname = "%s%s" % (r40(data[o+0]), r40(data[o+1]))
	dircode = "%i" % data[o+2]
	ftype = "%i" % data[o+3]
	arg1 = "%i" % data[o+4]
	arg2 = "%i" % data[o+5]
	attr = "%i" % data[o+6]
	user = "%i" % data[o+7]
	res = "%i" % data[o+8]
	sbeg = "%i" % data[o+9]
	send = "%i" % data[o+10]
	slen = "%i" % data[o+11]

	print "%6s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s" % (fname, dircode, ftype, arg1, arg2, attr, user, res, sbeg, send, slen)

ifile = sys.argv[1]
sstart = int(sys.argv[2])
send = int(sys.argv[3])

data = wload(ifile, sstart, 512*(send-sstart+1))

print " Disk image    : %s" % ifile
print " FILDIC start  : %i" % sstart
print " FILDIC end    : %i" % send
print " data len      : %i" % len(data)
print "-------------------------------------------------------------"

print "%6s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s" % ("name", "dir", "type", "arg1", "arg2", "attr", "user", "res", "beg", "end", "len")

o = 0

while 1:
	fil_print(data, o)
	o += 12
	if o % 252 == 0:
		o += 4;

