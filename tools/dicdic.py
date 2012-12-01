#!/usr/bin/python

import sys
from m400lib import r40, wload

def dic_print(data, o):
	dname = "%s%s" % (r40(data[o+0]), r40(data[o+1]))
	code = "%i" % data[o+2]
	sectors = "%i" % data[o+3]

	password = "%s" % r40(data[256+o+0])
	budget = "%i" % data[256+o+1]
	top = "%i" % data[256+o+2]
	rights = "%i" % data[256+o+3]

	osl = "%s%s" % (r40(data[512+o+0]), r40(data[512+o+1]))
	oslarg = "%s%s" % (r40(data[512+o+2]), r40(data[512+o+3]))

	print "%6s %5s %5s %3s %5s %5s %5s %6s %6s" % (dname, code, sectors, password, budget, top, rights, osl, oslarg)

ifile = sys.argv[1]
sstart = int(sys.argv[2])
send = int(sys.argv[3])

data = wload(ifile, sstart, 512*(send-sstart+1))

print " Disk image    : %s" % ifile
print " DICDIC start  : %i" % sstart
print " DICDIC end    : %i" % send
print " data len      : %i" % len(data)
print "-------------------------------------------------------------"

print "%6s %5s %5s %3s %5s %5s %5s %6s %6s" % ("Name", "Code", "Sect.", "Pas", "Budg.", "Top", "Acc.", "OSL", "OSLarg")
pos = 8
block = 0

while 1:
	o = block*256 + pos
	if data[o] == 0:
		print "EMPTY"
	elif data[o] == 1:
		break
	else:
		dic_print(data, o)
	pos += 4
	if pos>=256:
		pos = 0
		block += 1

