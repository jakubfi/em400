#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
from track import *

# ------------------------------------------------------------------------
class DiskImage:

    # --------------------------------------------------------------------
    def __init__(self, session_name, cylinders, heads, sectors):
        self.tracks = []
        print "Preparing disk image from session: %s (%d/%d/%d)" % (session_name, cylinders, heads, sectors)
        for cylinder in range(cylinders):
            for head in range(heads):
                wds_file = "%s--1--%03d--%d.wds" % (session_name, cylinder, head)
                self.tracks.append(Track(wds_file, sectors))

    # --------------------------------------------------------------------
    def write_image(self):
        for t in self.tracks:
            t.get_data()



# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

track = Track("dump--1--000--0.wds", 17)
#disk = DiskImage("dump", 615, 4, 17)

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
