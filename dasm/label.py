#!/usr/bin/python

import sys
from m400lib import r40, wload

ifile = sys.argv[1]
isector = int(sys.argv[2])

data = wload(ifile, isector, 17)

print " Disk image    : %s" % ifile
print " LABEL sector  : %i" % isector
print "-----------------------------------------------"
print " Label         : %s" % r40(data[0])
print " DICDIC offset : %i" % data[1]
print " FILDIC offset : %i" % data[2]
print " MAP offset    : %i" % data[3]
print " MAP end       : %i" % data[4]
print " Disk end      : %i" % data[5]
print " Disk name     : %s%s" % (r40(data[6]), r40(data[7]))
print " Init date     : %i-%i-%i" % (data[8], data[9], data[10])
print " Init date/time: %i-%i-%i %i:%i:%i" % (data[11], data[12], data[13], data[14], data[15], data[16])

