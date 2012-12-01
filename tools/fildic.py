#!/usr/bin/python

import sys
from m400lib import r40, wload

def fil_print(data, o):
	fname = "%s%s" % (r40(data[o+0]), r40(data[o+1]))
	dircode = "%i" % data[o+2]
	ftype = "%s" % r40(data[o+3])
	ftypei = "%s" % data[o+3]
	arg1 = "%i" % data[o+4]
	arg2 = "%i" % data[o+5]
	iacc  = (data[o+6] & 0b1111110000000000) >> 10
	if (iacc == 000):
		acc = "RES"
	elif (iacc == 040):
		acc = "OR"
	elif (iacc == 060):
		acc = "OW"
	elif (iacc == 050):
		acc = "LR"
	elif (iacc == 074):
		acc = "LW"
	elif (iacc == 052):
		acc = "AR"
	elif (iacc == 077):
		acc = "AW"
	else:
		acc = "%o" % iacc
	attr = "%i" % ((data[o+6] & 0b0000001111110000) >> 4)
	mem  = "%i" % ((data[o+6] & 0b0000000000001111) >> 0)
	user = "%i" % data[o+7]
	res = "%i" % data[o+8]
	sbeg = "%i" % data[o+9]
	send = "%i" % data[o+10]
	slen = "%i" % data[o+11]

	print "%6s %5s %3s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s" % (fname, dircode, ftype, ftypei, arg1, arg2, acc, attr, mem, user, res, sbeg, send, slen)

ifile = sys.argv[1]
sstart = int(sys.argv[2])
send = int(sys.argv[3])

data = wload(ifile, sstart, 512*(send-sstart+1))

print " Disk image    : %s" % ifile
print " FILDIC start  : %i" % sstart
print " FILDIC end    : %i" % send
print " data len      : %i" % len(data)
print "-------------------------------------------------------------"

print "%6s %5s %3s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s" % ("name", "dir", "typ", "typei", "arg1", "arg2", "acc", "attr", "mem", "user", "res", "beg", "end", "len")

o = 0
sect = 0

while 1:
	fil_print(data, sect*256 + o)
	o += 12
	if o >= 256:
		o = 0
		sect +=1

