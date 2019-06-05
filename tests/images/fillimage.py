#!/usr/bin/env python3

import sys

# Fill winchester image with sector addresses

f = open(sys.argv[1], "rb+")
f.seek(0x1a + 1*4*16*512)

for sect in range(0, 614*4*16):
    word = bytes([(sect>>8) & 0xff, sect & 0xff])
    f.write(256 * word)

f.close()
