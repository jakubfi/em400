#!/usr/bin/env python

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

import errno
import fuse
import stat
import time
import sys

from c5fs import *

fuse.fuse_python_api = (0, 2)

T_FILE = 1
T_DIR = 2
T_SPEC = 3

# ------------------------------------------------------------------------
class DirEntry:

    # --------------------------------------------------------------------
    def __init__(self, etype, peid, eid, name, st):
        self.etype = etype
        self.peid = peid
        self.eid = eid
        self.name = name
        self.st = st
        self.entries = []

        if self.etype == T_DIR:
            st = fuse.Stat()
            st.st_atime = int(time.time())
            st.st_mtime = st.st_atime
            st.st_ctime = st.st_atime
            st.st_mode = stat.S_IFDIR | 0755

            self.entries.append(DirEntry(T_SPEC, 0, 0, ".", st))
            self.entries.append(DirEntry(T_SPEC, 0, 0, "..", st))

    # --------------------------------------------------------------------
    def add_entry(self, e):
        #print "%s adding: %s" % (self.name, e.name)
        self.entries.append(e)

    # --------------------------------------------------------------------
    def find_entry(self, path):
        #print "%s searching for: %s" % (self.name, str(path))

        if len(path) == 0:
            return None

        if path[0] == self.name:
            if len(path) == 1:
                print "got it (direct): %s" % self.name
                return self
            else:
                path.pop(0)
                for i in self.entries:
                    ret = i.find_entry(path)
                    if ret is not None:
                        print "got it (search): %s" % ret.name
                        return ret
                return None
        else:
            return None

    # --------------------------------------------------------------------
    def list_entries(self):
        return [ x.name for x in self.entries ]

# ------------------------------------------------------------------------
class C5Fuse(fuse.Fuse):

    # --------------------------------------------------------------------
    def __init__(self, *args, **kw):
        fuse.Fuse.__init__(self, *args, **kw)
        self.c5fs = C5FS(image, offset)

        st = fuse.Stat()
        st.st_atime = int(time.time())
        st.st_mtime = st.st_atime
        st.st_ctime = st.st_atime
        st.st_mode = stat.S_IFDIR | 0755
        st.st_size = 1
        st.st_nlink = 2

        self.tree = DirEntry(T_DIR, 0, 8*4, "LIBRAR", st)
        self.buildtree(self.tree)

    # --------------------------------------------------------------------
    def buildtree(self, tree):
        for i in self.c5fs.dicdic:
            if self.c5fs.dicdic[i].topid == tree.eid:
                st = fuse.Stat()
                st.st_atime = int(time.time())
                st.st_mtime = st.st_atime
                st.st_ctime = st.st_atime
                st.st_mode = stat.S_IFDIR | 0755
                st.st_size = self.c5fs.dicdic[i].subdirs
                st.st_nlink = 2

                e = DirEntry(T_DIR, tree.eid, self.c5fs.dicdic[i].pos, "%s" % self.c5fs.dicdic[i].name, st)
                tree.add_entry(e)
                self.buildtree(e)

        for i in self.c5fs.fildic:
            if self.c5fs.fildic[i].did == tree.eid:
                st = fuse.Stat()
                st.st_atime = int(time.time())
                st.st_mtime = st.st_atime
                st.st_ctime = st.st_atime
                st.st_mode = stat.S_IFREG | 0755
                st.st_size = self.c5fs.fildic[i].size * 512
                st.st_nlink = 1

                e = DirEntry(T_FILE, tree.eid, self.c5fs.fildic[i].pos, "%s.%s" % (self.c5fs.fildic[i].name, self.c5fs.fildic[i].ext), st)
                tree.add_entry(e)
                
        return tree

    # --------------------------------------------------------------------
    def getattr(self, path):
        print "getattr : %s" % path

        path_split = ("LIBRAR%s" % path).strip("/").split("/")
        e = self.tree.find_entry(path_split)
        print "getattr has: %s" % e.name

        return e.st

    # --------------------------------------------------------------------
    def readdir(self, path, offset):
        print "readdir: %s" % path

        path = ("LIBRAR%s" % path).strip("/")
        e = self.tree.find_entry(path.split("/"))
        print "readdir has: %s" % e.name

        for i in e.list_entries():
            print "readdir yielding: %s" % i
            yield fuse.Direntry(i)

    # --------------------------------------------------------------------
    def read(self, path, size, offset):
        print "Reading file: %s" % path
        path_split = ("LIBRAR%s" % path).strip("/").split("/")
        e = self.tree.find_entry(path_split)

        if e.etype != T_FILE:
            return -errno.ENOENT
        else:
            data = self.c5fs.read_file(e.peid, e.eid)

        return data[offset:size]

# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

if len(sys.argv) < 3:
    print "Usage: c5fuse.py image:offset mountpoint "
    sys.exit(1)

o = sys.argv[1].split(':')
image = o[0]
offset = int(o[1])

fs = C5Fuse()
fs.parse(errex=1)
fs.main()


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

