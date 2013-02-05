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


from mfm import *
from sector import *

# ------------------------------------------------------------------------
class Track:

    # --------------------------------------------------------------------
    def __init__(self, wds_file, sectors_per_track, sector_size):
        self.sectors_per_track = sectors_per_track
        self.sector_size = sector_size
        self.sectors = {}
        self.iter_pos = 0

        self.data = MFMData(wds_file)

        sector = Sector(self.sector_size)
        for s in self.data:
            res = sector.feed(s)

            if res == State.DONE:
                if not sector.head_crc_ok or not sector.data_crc_ok:
                    print "Sector %2d: %3d/%d/%2d CRC header: %s, CRC data: %s, BAD: %s" % (len(self.sectors)+1, sector.cylinder, sector.head, sector.sector, str(sector.head_crc_ok), str(sector.data_crc_ok), str(sector.bad))
                self.sectors[sector.sector] = sector
                sector = Sector(self.sector_size)
                if len(self.sectors) == self.sectors_per_track:
                    break

            elif res == State.FAILED:
                print "Cooking sector failed"
                break

    # --------------------------------------------------------------------
    def __len__(self):
        return len(self.sectors)

    # --------------------------------------------------------------------
    def __iter__(self):
        self.iter_pos = 0
        return self

    # --------------------------------------------------------------------
    def next(self):
        if self.iter_pos >= self.sectors_per_track:
            raise StopIteration
        else:
            self.iter_pos += 1
            return self.sectors[self.iter_pos].data


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
