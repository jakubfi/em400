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


# ------------------------------------------------------------------------
class Samples:

    # --------------------------------------------------------------------
    def __init__(self, wds_file):
        print "Loading wds file: %s" % wds_file
        wds_f = open(wds_file, "r")
        self.data = bytearray(wds_f.read())
        wds_f.close()
        print "Bytes loaded: %d" % len(self.data)

        self.byte_pos = 0
        self.bit_pos = 7

    # --------------------------------------------------------------------
    def __iter__(self):
        self.byte_pos = 0
        self.bit_pos = 7
        return self

    # --------------------------------------------------------------------
    def __len__(self):
        return len(self.data)*8

    # --------------------------------------------------------------------
    def next(self):
        if self.byte_pos >= len(self.data):
            raise StopIteration
        else:
            v = (self.data[self.byte_pos] >> self.bit_pos) & 1

            if self.bit_pos == 0:
                self.bit_pos = 7
                self.byte_pos += 1
            else:
                self.bit_pos -= 1

            return v


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
