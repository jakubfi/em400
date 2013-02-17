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
        self.uid = (data[7] & 0b1111111111110000)
        self.flags = (data[7] & 0b0000000000001111)
        self.reserved = data[8]
        self.start = data[9]
        self.end = data[10]
        self.size = data[11]

        if self.did == 0 or self.uid == 0:
            raise ValueError

    # --------------------------------------------------------------------
    def __str__(self):
        return "%-6s.%-3s %5i/%-5i (%i-%i = %i) flags: %i, reserved: %i" % (self.name, self.ext, self.uid, self.did, self.start, self.end, self.size, self.flags, self.reserved)

# ------------------------------------------------------------------------
class C5FS:

    # --------------------------------------------------------------------
    def __init__(self, image, offset):
        self.image = image
        self.offset = offset

        self.read_label()
        self.read_dicdic()
        self.read_fildic()

    # --------------------------------------------------------------------
    def read_label(self):
        data = wload(self.image, self.offset, 17)
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
        pos = 8
        while pos+512 < dicdic_size:
            if data[pos] == 0:
                pass
            if data[pos] == 1:
                break
            else:
                entry_data = data[pos:pos+4] + data[256+pos:256+pos+4] + data[512+pos:512+pos+4]
                d = DDEntry(pos*4, entry_data)
                self.dicdic[pos*4] = d

            pos += 4
            if pos % 256 == 0:
                pos += 256 * 2

    # --------------------------------------------------------------------
    def read_fildic(self):
        fildic_size = 256 * (self.map_start - self.fildic_start)
        data = wload(self.image, self.offset + self.fildic_start, fildic_size)

        self.fildic = {}
        pos = 0
        while pos < fildic_size-1:
            entry_data = data[pos:pos+12]
            try:
                f = FDEntry(pos, entry_data)
                self.fildic[pos] = f
            except ValueError:
                pass

            pos += 12
            if pos % 256 == 252:
                pos += 4

    # --------------------------------------------------------------------
    def print_label(self):
        print "  Label         : %s" % self.label
        print "  Disk name     : %s" % self.disk_name
        print "  DICDIC start  : %i" % self.dicdic_start
        print "  FILDIC start  : %i" % self.fildic_start
        print "  MAP start/end : %i / %i" % (self.map_start, self.map_end)
        print "  Disk end      : %i" % self.disk_end
        print "  Disk size     : %i KB" % (self.disk_end/2)
        print "  Init date     : %s" % self.init_date
        print "  Init date/time: %s" % self.init_date_time

    # --------------------------------------------------------------------
    def print_dicdic(self):
        for i in self.dicdic:
            print self.dicdic[i]

    # --------------------------------------------------------------------
    def print_fildic(self):
        for i in self.fildic:
            print self.fildic[i]

    # --------------------------------------------------------------------
    def dump_file(self, did, pos):
        name = "%s.%s" % (self.fildic[pos].name, self.fildic[pos].ext)
        start = 512 * (self.offset + self.fildic[pos].start)
        size = 512 * (self.fildic[pos].size)

        print "  Dumping file: \"%s\" (%i bytes at %i)" % (name, size, start)

        fin = open(self.image, "r")
        fin.seek(start)
        data = fin.read(size)
        fin.close()

        fout = open(name, "w")
        fout.write(data)
        fout.close()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

