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

from samples import *

# ------------------------------------------------------------------------
class MFMData:

    # --------------------------------------------------------------------
    def __init__(self, wds_file):
        samples = Samples(wds_file)
        clock_finder = MFMClockGen(11, 2, 0)
        self.data = clock_finder.run(samples)
        self.pos = 0
        self.clock_period = float(self.data[len(self.data)-1][0]) / len(self.data)
        print "Total clock ticks: %s, average clock period: %f samples" % (len(self.data), self.clock_period)

    # --------------------------------------------------------------------
    def period(self):
        return self.clock_period

    # --------------------------------------------------------------------
    def __iter__(self):
        self.pos = 0
        return self

    # --------------------------------------------------------------------
    def __len__(self):
        return len(self.data)

    # --------------------------------------------------------------------
    def next(self):
        if self.pos >= len(self.data):
            raise StopIteration
        else:
            self.pos += 1
            return self.data[self.pos-1]


# ------------------------------------------------------------------------
class MFMClockGen:

    # --------------------------------------------------------------------
    def __init__(self, c_period, c_margin, c_offset):
        self.c_period = c_period
        self.c_margin = c_margin
        self.c_offset = c_offset

        self.last_clock = -100

        print "MFMClockGen: clock len: %d, margin: %d, offset: %d" % (self.c_period, self.c_margin, self.c_offset)

    # --------------------------------------------------------------------
    def add_clock(self, ticks, t, v):
        # remove early inserted clocks
        if (t - self.last_clock) <= self.c_margin:
            ticks.pop()

        ticks.append((t + self.c_offset, v))
        self.last_clock = t

    # --------------------------------------------------------------------
    def run(self, samples):
        print "Running clock analysis on MFM data (%d samples)" % (len(samples))

        ticks = []
        ov = 1
        t = 0

        for v in samples:

            # each rising edge restarts clock
            if (ov == 0) and (v == 1):
                self.add_clock(ticks, t, v)

            # if not rising edge, maybe it's time for next tick?
            elif t >= self.last_clock + self.c_period:
                self.add_clock(ticks, t, v)

            ov = v
            t += 1

        return ticks


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
