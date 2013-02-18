#  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

from m400lib import r40, wload

# ------------------------------------------------------------------------
class DDEntry:

    # --------------------------------------------------------------------
    def __init__(self, pos, data):
        self.pos = pos
        self.name = "%s%s" % (r40(data[0]), r40(data[1]))
        self.id = data[2]
        self.subdirs = data[3]
        self.password = "%s" % r40(data[4])
        self.budget = data[5]
        self.topid = data[6]
        self.rights = data[7]
        self.suspended = True if data[7] & (1<<15) else False
        self.type = "User" if data[7] & (1<<14) else "Dir"
        self.osl = "%s%s" % (r40(data[8]), r40(data[9]))
        self.osl_arg = "%s%s" % (r40(data[10]), r40(data[11]))

        # 0 = empty entry, skip
        if data[0] == 0:
            raise ValueError
        # 1 = DICDIC end
        elif data[0] == 1:
            raise EOFError

    # --------------------------------------------------------------------
    def __str__(self):
        return "%4s [%5i]: %6s [%3s] (id:%-5i top:%-5i) subdirs: %i, budget: %i, OSL: %s (%s)" % (self.type, self.pos, self.name, self.password, self.id, self.topid, self.subdirs, self.budget, self.osl, self.osl_arg)

# ------------------------------------------------------------------------
class FDEntry:

    # --------------------------------------------------------------------
    def __init__(self, pos, data):
        self.pos = pos
        self.name = "%s%s" % (r40(data[0]), r40(data[1]))
        self.ext = "%s" % r40(data[3])
        self.did = data[2]
        self.param1 = data[4]
        self.param2 = data[5]
        self.rights = (data[6] & 0b1111100000000000) >> 11
        self.attrs = (data[6] & 0b0000011111100000) >> 5
        self.mem = data[6] & 0b0000000000011111
        self.uid = (data[7] & 0b1111111111110000)
        self.flags = (data[7] & 0b0000000000001111)
        self.reserved = data[8]
        self.start = data[9]
        self.end = data[10]
        self.size = data[11]

        # skip GLOBAL in filesystem contents
        if self.name.startswith("GLOBAL"):
            raise ValueError

        # no top directory, ignore
        if self.did == 0 or 
            raise ValueError

        # show deleted files with "DELETED_" prefix
        if data[0] == 0:
            self.name = "DELETED_%s" % self.name

        # shouldn't happen, but as long as .did is set, it's ok, display the file with "ZEROUID_" prefix
        if self.uid == 0:
            self.name = "ZEROUID_%s" % self.name

    # --------------------------------------------------------------------
    def __str__(self):
        return "%-6s.%-3s %5i/%-5i (%i-%i = %i) flags: %i, reserved: %i, rights: %i attrs: %i mem: %i" % (self.name, self.ext, self.uid, self.did, self.start, self.end, self.size, self.flags, self.reserved, self.rights, self.attrs, self.mem)

# ------------------------------------------------------------------------
class C5FS:

    # --------------------------------------------------------------------
    def __init__(self, image, offset, label_offset):
        self.image = image
        self.offset = offset
        self.label_offset = label_offset

        self.read_label()
        self.read_dicdic()
        self.read_fildic()
        self.read_map()

    # --------------------------------------------------------------------
    def read_label(self):
        data = wload(self.image, self.offset+self.label_offset, 17)
        self.label = r40(data[0])
        self.dicdic_start = data[1]
        self.fildic_start = data[2]
        self.map_start = data[3]
        self.map_end = data[4]
        self.disk_end = data[5]
        self.disk_name = "%s%s" % (r40(data[6]), r40(data[7]))
        self.init_date = "%i-%i-%i" % (data[8], data[9], data[10])
        self.init_date_time = "%i-%i-%i %i:%i:%i" % (data[11], data[12], data[13], data[14], data[15], data[16])

    # --------------------------------------------------------------------
    def read_dicdic(self):
        dicdic_size = 256 * (self.fildic_start - self.dicdic_start)
        data = wload(self.image, self.offset + self.dicdic_start, dicdic_size)

        self.dicdic = {}
        # DICDIC entries start at 8th word in first DICDIC sector
        pos = 8
        while pos+512 < dicdic_size:

            # each DICDIC entry spans across 3 consecutive sectors, 4 words per sector
            entry_data = data[pos:pos+4] + data[256+pos:256+pos+4] + data[512+pos:512+pos+4]

            try:
                d = DDEntry(pos*4, entry_data)
                self.dicdic[pos*4] = d
            except ValueError:
                pass
            except EOFError:
                return

            # next DICDIC entry starts in +4 words
            pos += 4

            # end of sector, skip to next one
            if pos % 256 == 0:
                pos += 256 * 2

    # --------------------------------------------------------------------
    def read_fildic(self):
        fildic_size = 256 * (self.map_start - self.fildic_start)
        data = wload(self.image, self.offset + self.fildic_start, fildic_size)

        self.fildic = {}
        # FILDIC starts at the beginning of first sector
        pos = 0
        while pos < fildic_size-1:

            entry_data = data[pos:pos+12]

            try:
                f = FDEntry(pos, entry_data)
                self.fildic[pos] = f
            except ValueError:
                pass

            # 12 words each FILEDIC entry
            pos += 12
            # last 4 words in a sector are reserved for hash
            if pos % 256 == 252:
                pos += 4

    # --------------------------------------------------------------------
    def read_map(self):
        map_size = 512 * ((self.disk_end/8 + 512) / 512)
        map_offset = 512 * (self.map_start + self.offset)

        f = open(self.image, "r")
        f.seek(map_offset)
        data = f.read(map_size)
        f.close()

        self.map = []
        for byte in data:
            for bit in [7, 6, 5, 4, 3, 2, 1, 0]:
                x = (ord(byte) >> 7) & 1
                self.map.append(x)

    # --------------------------------------------------------------------
    def get_map(self, start, length):
        filemap = self.map[start:start+length]
        m = ''.join(["%i" % x for x in filemap])
        return m

    # --------------------------------------------------------------------
    def get_file(self, did, pos):
        name = "%s.%s" % (self.fildic[pos].name, self.fildic[pos].ext)
        start = 512 * (self.offset + self.fildic[pos].start)
        size = 512 * (self.fildic[pos].size)

        fin = open(self.image, "r")
        fin.seek(start)
        data = fin.read(size)
        fin.close()
        return data


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

