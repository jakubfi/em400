#!/usr/bin/env python3

import sys

# Fill a disk image with sector addresses

image = sys.argv[1]
initial_offset_bytes = int(sys.argv[2])
sect_offset = int(sys.argv[3])
cylinders = int(sys.argv[4])
heads = int(sys.argv[5])
spt = int(sys.argv[6])
bytes_per_sector = int(sys.argv[7])
words_per_sector = bytes_per_sector // 2

f = open(image, "rb+")
f.seek(initial_offset_bytes)

filler = bytes([x & 0xff for x in range(2, bytes_per_sector)])

for sect in range(0, cylinders * heads * spt):
    sector_num = sect_offset + sect
    word = bytes([(sector_num >> 8) & 0xff, sector_num & 0xff])
    f.write(word)
    f.write(filler)

f.close()
