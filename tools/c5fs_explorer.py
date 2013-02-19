#!/usr/bin/python

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

import os
import sys
import inspect
from c5fs import *

# ------------------------------------------------------------------------
class C5FSExplorer:

    # --------------------------------------------------------------------
    def __init__(self, image):
        self.quit = False
        self.image = image
        self.fs = None
        self.fscnt = 0
        self.offset = 0
        self.label_offset = 0

        self.cur_dir = 0
        self.path = []

        self.methods = dict(inspect.getmembers(self, predicate=inspect.ismethod))

    # --------------------------------------------------------------------
    def cmd_quit(self, args):
        """Leave C5FS explorer"""
        self.quit = True
    cmd_exit = cmd_quit

    # --------------------------------------------------------------------
    def cmd_label(self, args):
        """Print contents of currently selected label"""

        if self.fs is None:
            return

        print "  Label         : %s" % self.fs.label
        print "  Disk name     : %s" % self.fs.disk_name
        print "  DICDIC start  : %i" % self.fs.dicdic_start
        print "  FILDIC start  : %i" % self.fs.fildic_start
        print "  MAP start/end : %i / %i" % (self.fs.map_start, self.fs.map_end)
        print "  Disk end      : %i" % self.fs.disk_end
        print "  Disk size     : %i KB" % (self.fs.disk_end/2)
        print "  Init date     : %s" % self.fs.init_date
        print "  Init date/time: %s" % self.fs.init_date_time

    # --------------------------------------------------------------------
    def cmd_dicdic(self, args):
        """Print DICDIC for currently selected partition"""

        if self.fs is None:
            return

        for i in self.fs.dicdic:
            print self.fs.dicdic[i]

    # --------------------------------------------------------------------
    def cmd_fildic(self, args):
        """Print FILDIC for currently selected partition"""

        if self.fs is None:
            return

        for i in self.fs.fildic:
            print self.fs.fildic[i]
            # dump MAP contents for the file
            smap = self.fs.get_map(self.fs.fildic[i].start, self.fs.fildic[i].end-self.fs.fildic[i].start)
            print smap

    # --------------------------------------------------------------------
    def cmd_use(self, args):
        """Change current partition"""

        self.label_offset = 0

        if len(args) == 0:
            print "Usage: use offset [label_offset]"
            return
        elif len(args) == 1:
            self.offset = int(args[0])
        else:
            self.offset = int(args[0])
            self.label_offset = int(args[1])

        self.fs = None

        try:
            self.fs = C5FS(self.image, self.offset, self.label_offset)
            # we need at least two entries: LIBRAR and BOSS
            if self.fs.dicdic[8*4].name != "LIBRAR":
                raise ValueError("User LIBRAR not found")
            if self.fs.dicdic[12*4].name != "BOSS":
                raise ValueError("User BOSS not found")
        except Exception, e:
            print "  Seems there is no label at %i:%i. Error: %s" % (self.offset, self.label_offset, str(e))
            return

        self.cur_dir = 8*4
        self.path = [ self.cur_dir ]

        print "  Switching to label \"%s\" (%i:%i)" % (self.fs.label, self.offset, self.label_offset)

    # --------------------------------------------------------------------
    def cmd_help(self, args):
        """Print help"""
        for i in sorted(self.methods.keys()):
            if i.startswith("cmd_"):
                print "%10s : %s" % (i.replace("cmd_", ""), self.methods[i].__doc__)

    # --------------------------------------------------------------------
    def cmd_ls(self, args):

        if self.fs is None:
            return

        dicdic = self.fs.dicdic
        diclist = sorted([ (dicdic[x].name, dicdic[x]) for x in dicdic ])

        for d in diclist:
            if d[1].topid == self.cur_dir:
                print "  %-6s <DIR>  %6s %7i" % (d[1].name, dicdic[d[1].topid].name, d[1].subdirs)

        fildic = self.fs.fildic
        fillist = sorted([ (("%s.%s" % (fildic[x].name, fildic[x].ext)), fildic[x]) for x in fildic ])
        for f in fillist:
            if f[1].did == self.cur_dir:
                print "  %-10s    %6s %7i" % ("%s.%s" % (f[1].name, f[1].ext), dicdic[f[1].uid].name, f[1].size*512)

    # --------------------------------------------------------------------
    def cmd_dump(self, args):

        if self.fs is None:
            return

        if len(args) == 0:
            return

        name = args[0].upper()
        name_split = name.split(".")

        fildic = self.fs.fildic
        for i in fildic:
            if fildic[i].did == self.cur_dir and fildic[i].name == name_split[0] and fildic[i].ext == name_split[1]:
                data = self.fs[self.cur_label].get_file(fildic[i].did, fildic[i].pos)

                print "  Dumping file: %s" % name
                fout = open(name, "w")
                fout.write(data)
                fout.close()

                return

        print "  Could not find file: %s" % name

    # --------------------------------------------------------------------
    def cmd_cd(self, args):

        if self.fs is None:
            return

        if len(args) == 0:
            return

        dirname = args[0]
        dicdic =  self.fs.dicdic

        if dirname == ".":
            pass
        elif dirname == "/":
            self.cur_dir = 8*4
            self.path = [ 8*4 ]
        elif dirname == "..":
            topdir = dicdic.topid
            if topdir == 0:
                print "  Already at top dir"
            else:
                self.path.pop()
                self.cur_dir = topdir
        else:
            for i in dicdic:
                if dicdic[i].name == dirname.upper():
                    self.cur_dir = dicdic[i].pos
                    self.path.append(self.cur_dir)
                    return
            print "  No such directory: %s" % dirname

    # --------------------------------------------------------------------
    def run(self):
        while not self.quit:

            if self.fs is None:
                label = '?'
            else:
                label = self.fs.label

            self.directory = "/".join([ self.fs.dicdic[x].name for x in self.path ])

            prompt = '\x1b[32;1m%s:%s/> \x1b[0m' % (label, self.directory)

            try:
                cmd = raw_input(prompt).split()
            except EOFError:
                self.quit = 1
                print ""
                continue

            if len(cmd) == 0:
                continue
            else:
                cmd_name = cmd[0]
                cmd_mthd = "cmd_%s" % cmd_name
                cmd_args = cmd[1:]

                if self.methods.has_key(cmd_mthd):
                    self.methods[cmd_mthd](cmd_args)
                else:
                    print "  Unknown command: %s" % cmd_name

# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

if len(sys.argv) != 2:
    print "Usage: c5fs_explorer.py image_file"
    sys.exit(1)

c5e = C5FSExplorer(sys.argv[1])
c5e.run()


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

