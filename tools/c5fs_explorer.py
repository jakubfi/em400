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
    def __init__(self, image = ""):
        self.quit = False
        self.image = ""
        self.fs = []
        self.fscnt = 0
        self.cur_label = -1

        self.cur_dir = 0
        self.path = []

        if image:
            self.load_image(image)

        self.methods = dict(inspect.getmembers(self, predicate=inspect.ismethod))

    # --------------------------------------------------------------------
    def switch_label(self, label_no):
        self.cur_label = label_no
        self.cur_dir = 8*4
        self.path = [ self.cur_dir ]
        print "  Switched to label %i: \"%s\"" % (self.cur_label, self.fs[self.cur_label].label)

    # --------------------------------------------------------------------
    def load_image(self, image):
        self.image = image
        print "  Searching image '%s' for labels..." % image
        for offset in self.find_labels(image):
            self.fs.append(C5FS(image, offset))
            print "  Label %i: \"%s\" at sector %i" % (self.fscnt, self.fs[self.fscnt].label, offset)
            self.fscnt += 1

        if self.fscnt > 0:
            self.switch_label(0)
        else:
            print "  No labels found"

    # --------------------------------------------------------------------
    def find_labels(self, image):
        # maybe some other time ;-)
        return [64, 9888]

    # --------------------------------------------------------------------
    def label_ok(self):
        if self.image != "":
            if self.cur_label >= 0:
                return True
            else:
                print "  No label selected"
                return False
        else:
            print "  No image loaded"
            return False

    # --------------------------------------------------------------------
    def cmd_quit(self, args):
        """Leave C5FS explorer"""
        self.quit = True
    cmd_exit = cmd_quit

    # --------------------------------------------------------------------
    def cmd_load(self, args):
        self.load_image(args[0])

    # --------------------------------------------------------------------
    def cmd_label(self, args):
        """Print contents of currently selected label"""
        if self.label_ok():
            c5fs = self.fs[self.cur_label]
            print "  Label         : %s" % c5fs.label
            print "  Disk name     : %s" % c5fs.disk_name
            print "  DICDIC start  : %i" % c5fs.dicdic_start
            print "  FILDIC start  : %i" % c5fs.fildic_start
            print "  MAP start/end : %i / %i" % (c5fs.map_start, c5fs.map_end)
            print "  Disk end      : %i" % c5fs.disk_end
            print "  Disk size     : %i KB" % (c5fs.disk_end/2)
            print "  Init date     : %s" % c5fs.init_date
            print "  Init date/time: %s" % c5fs.init_date_time

    # --------------------------------------------------------------------
    def cmd_dicdic(self, args):
        """Print DICDIC for currently selected partition"""
        if self.label_ok():
            dicdic = self.fs[self.cur_label].dicdic
            for i in dicdic:
                print dicdic[i]

    # --------------------------------------------------------------------
    def cmd_fildic(self, args):
        """Print FILDIC for currently selected partition"""
        if self.label_ok():
            fildic = self.fs[self.cur_label].fildic
            for i in fildic:
                print fildic[i]

    # --------------------------------------------------------------------
    def cmd_use(self, args):
        """Change current partition"""
        if self.label_ok():
            nlabel = int(args[0])
            if nlabel >= 0 and nlabel < self.fscnt:
                self.cur_label = nlabel
                self.cur_dir = 8*4
                self.path = [ 8*4 ]
                print "  Switched to label %i: \"%s\"" % (self.cur_label, self.fs[self.cur_label].label)
            else:
                print "  No such label: %i" % nlabel

    # --------------------------------------------------------------------
    def cmd_help(self, args):
        """Print help"""
        for i in sorted(self.methods.keys()):
            if i.startswith("cmd_"):
                print "%10s : %s" % (i.replace("cmd_", ""), self.methods[i].__doc__)

    # --------------------------------------------------------------------
    def cmd_ls(self, args):
        dicdic = self.fs[self.cur_label].dicdic
        diclist = sorted([ (dicdic[x].name, dicdic[x]) for x in dicdic ])

        for d in diclist:
            if d[1].topid == self.cur_dir:
                print "  %-6s <DIR>  %6s %7i" % (d[1].name, dicdic[d[1].topid].name, d[1].subdirs)

        fildic = self.fs[self.cur_label].fildic
        fillist = sorted([ (("%s.%s" % (fildic[x].name, fildic[x].ext)), fildic[x]) for x in fildic ])
        for f in fillist:
            if f[1].did == self.cur_dir:
                print "  %-10s    %6s %7i" % ("%s.%s" % (f[1].name, f[1].ext), dicdic[f[1].uid].name, f[1].size*512)

    # --------------------------------------------------------------------
    def cmd_dump(self, args):
        if len(args) == 0:
            return

        name = args[0].upper()
        name_split = name.split(".")

        fildic = self.fs[self.cur_label].fildic
        for i in fildic:
            if fildic[i].did == self.cur_dir and fildic[i].name == name_split[0] and fildic[i].ext == name_split[1]:
                data = self.fs[self.cur_label].read_file(fildic[i].did, fildic[i].pos)

                print "  Dumping file: %s" % name
                fout = open(name, "w")
                fout.write(data)
                fout.close()

                return

        print "  Could not find file: %s" % name

    # --------------------------------------------------------------------
    def cmd_cd(self, args):
        if len(args) == 0:
            return

        dirname = args[0]
        dicdic =  self.fs[self.cur_label].dicdic

        if dirname == ".":
            pass
        elif dirname == "/":
            self.cur_dir = 8*4
            self.path = [ 8*4 ]
        elif dirname == "..":
            topdir = dicdic[self.cur_dir].topid
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

            if self.cur_label >= 0 and self.cur_label < self.fscnt:
                label = self.fs[self.cur_label].label
            else:
                label = '?'

            self.directory = "/".join([ self.fs[self.cur_label].dicdic[x].name for x in self.path ])

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

if len(sys.argv) > 1:
    c5e = C5FSExplorer(sys.argv[1])
else:
    c5e = C5FSExplorer()

c5e.run()


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

