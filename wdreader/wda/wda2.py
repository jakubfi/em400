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
import re
from track import *
from mfm import *

# ------------------------------------------------------------------------
def process_file(file_name):
    try:
        track = Track(file_name, MFMClockGen(11, 2, 0), SectorMERA, 17, 512)
        track.analyze()
    except Exception, e:
        print "Cannot load track %s for analysis. Error: %s" % (file_name, str(e))

    # write sector image to a file
    out_file = file_name.replace(".wds", ".img")
    outf = open(out_file, "w")
    for sector in track:
        outf.write(''.join([chr(x) for x in sector]))
    outf.close()
    print "%s: %d samples, %d clocks (period: %8.5f) %d sectors -> %s" % (re.sub(".*/", "", file_name), len(track.data.samples), len(track.data.data), track.data.period(), len(track), re.sub(".*/", "", out_file))


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

session_name = None
file_name = None
heads = [0, 1, 2, 3]

# parse command line

if len(sys.argv) < 2:
    print "Usage: wda2.py <filename.wds>|<session_name [head]>"
    sys.exit(1)
if sys.argv[1].endswith(".wds"):
    file_name = sys.argv[1]
else:
    session_name = sys.argv[1]
    if len(sys.argv) == 3:
        heads = [int(x) for x in sys.argv[2].split(',')]

# single-file run
if file_name is not None:
    print "Running analysis on single file: %s" % file_name
    process_file(file_name)

# session run
else:
    print "Running analysis on WDS session: %s, heads: %s" % (session_name, ','.join([str(x) for x in heads]))
    for cylinder in range(615):
        for head in heads:
            file_name = "%s--1--%03d--%d.wds" % (session_name, cylinder, head)
            process_file(file_name)


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
