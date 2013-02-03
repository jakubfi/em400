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

#head = int(sys.argv[1])

#for i in range(615):
#    track = Track("dump--1--%03d--%d.wds" % (i, head), 17, 512)

track = Track("dump--1--000--3.wds", 17, 512)
for sector in track:
    print sector

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
