#!/usr/bin/env python

import sys

# Fill winchester image with sector addresses

f = open(sys.argv[1], "rb+")
f.seek(0x1a + 1*4*16*512)

for sect in range(0, 614*4*16):
    f.write(256 * ("%c%c" % (sect/256, sect%256)))

f.close()
