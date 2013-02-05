#!/usr/bin/python
# -*- coding: UTF-8 -*-

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


import sys
from track import *


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

try:
    name = sys.argv[1]
except Exception, e:
    print "Usage: wda2.py <filename.wds>|<session_name>"
    sys.exit(1)

if name.endswith(".wds"):
    print "Running analysis on single file: %s" % name
    try:
        track = Track(name, 17, 512)
        print "%s: samples: %d, MFM clocks: %d, clock period: %.5f, sectors: %d" % (name, len(track.data.samples), len(track.data.data), track.data.period(), len(track))
    except Exception, e:
        print "Cannot load track %s for analysis. Error: %s" % (name, str(e))
        sys.exit(1)
    for sector in track:
        pass
else:
    print "Running analysis on WDS session: %s" % name
    for cylinder in range(615):
        for head in range(4):
            track_name = "%s--1--%03d--%d.wds" % (name, cylinder, head)
            try:
                track = Track(track_name, 17, 512)
                print "%s: samples: %d, MFM clocks: %d, clock period: %.5f, sectors: %d" % (track_name, len(track.data.samples), len(track.data.data), track.data.period(), len(track))
                sys.stdout.flush()
            except Exception, e:
                print "Cannot run analysis on file: %s. Error: %s" % (track_name, str(e))
                sys.exit(1)
            for sector in track:
                pass

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
