#!/usr/bin/python

import re
import sys
import inspect

ERROR =     1
NEXT_CMD =  2

# ------------------------------------------------------------------------
class Edit:

    # --------------------------------------------------------------------
    def __init__(self, efile, infile, outfile):
        self.fin = open(infile, "r")
        self.fout = open(outfile, "w")

        e = open(efile, "r")
        self.edits = [ l for l in e ]
        e.close()

        self.cmds = {k:v for k,v in inspect.getmembers(self, predicate=inspect.ismethod) if k.startswith("cmd_") }

        self.gl = []
        self.buf = None
        self.cmdpos = 0;
        self.cmd = None
        self.cmdline = None
        self.quit = 0
        self.loopcount = -1
        self.loopstart = -1
        self.process()
    
    # --------------------------------------------------------------------
    def cmd_IN(self):
        pass

    # --------------------------------------------------------------------
    def cmd_OU(self):
        pass

    # --------------------------------------------------------------------
    def cmd_GL(self):
        stop = self.cmdargs[0]
        self.next_cmd()
        while not self.cmdline.startswith(stop):
            print "GL: " + self.cmdline
            self.gl.append(self.cmdpos-1)
            self.next_cmd()
        print str(self.gl)

    # --------------------------------------------------------------------
    def cmd_FL(self):
        self.flush()
        string = self.cmdargs.rstrip("\n\r\f")
        print "FL: " + string
        self.get_line()
        while not self.buf.startswith(string):
            self.flush()
            self.get_line()

    # --------------------------------------------------------------------
    def cmd_FS(self):
        self.flush()
        string = self.cmdargs.rstrip("\n\f\r")
        print "FS: " + string
        self.get_line()
        while not string in self.buf:
            self.flush()
            self.get_line()

    # --------------------------------------------------------------------
    def cmd_IB(self):
        self.flush()
        stop = self.cmdargs[0]
        self.next_cmd()
        while not self.cmdline.startswith(stop):
            print "IB: " + self.cmdline
            self.fout.write(self.cmdline)
            self.next_cmd()

    # --------------------------------------------------------------------
    def cmd_AB(self):
        self.buf = "%s%s" % (self.cmdargs.rstrip("\n\r\f"), self.buf)

    # --------------------------------------------------------------------
    def cmd_AE(self):
        self.buf = "%s%s" % (self.buf.rstrip("\n\r\f"), self.cmdargs)

    # --------------------------------------------------------------------
    def cmd_DL(self):
        self.flush()
        string = self.cmdargs.rstrip("\n\f\r")
        print "DL: " + string
        self.get_line()
        while not self.buf.startswith(string):
            self.get_line()

    # --------------------------------------------------------------------
    def cmd_DS(self):
        self.flush()
        string = self.cmdargs.rstrip("\n\f\r")
        print "DS: " + string
        self.get_line()
        while not string in self.buf:
            self.get_line()

    # --------------------------------------------------------------------
    def cmd_DE(self):
        print "DE: (%s)" % self.buf
        self.buf = None

    # --------------------------------------------------------------------
    def cmd_RS(self):
        args = self.cmdargs[1:].rstrip("\n\r\f").split(self.cmdargs[0])
        pos = 0
        while pos < len(args):
            print "RS: %s -> %s" % (args[pos+0], args[pos+1])
            self.buf = re.sub(re.escape(args[pos+0]), args[pos+1], self.buf)
            pos += 2

    # --------------------------------------------------------------------
    def cmd_RE(self):
        self.flush()
        self.get_line()
        while self.buf:
            self.flush()
            self.get_line()
        self.quit = 1

    # --------------------------------------------------------------------
    def cmd_IL(self):
        print "IL: %s" % self.cmdargs.rstrip("\n\r\f")
        self.flush()
        self.buf = self.cmdargs

    # --------------------------------------------------------------------
    def cmd_LP(self):
        self.loopcount = int(self.cmdargs.rstrip("\n\r\f"))
        print "LP: %i" % self.loopcount
        self.loopstart = self.cmdpos

    # --------------------------------------------------------------------
    def cmd_JP(self):
        self.loopcount -= 1
        print "JP: %i" % self.loopcount
        if self.loopcount > 0:
            self.cmdpos = self.loopstart

    # --------------------------------------------------------------------
    def flush(self, gl=1):
        if self.buf is not None:
            ocmdpos = self.cmdpos-1
            for i in self.gl:
                self.get_cmd(i)
                print "gl: %i: %s" % (i, self.cmdline)
                self.run()
            self.fout.write(self.buf)
            self.buf = None
            self.get_cmd(ocmdpos)

    # --------------------------------------------------------------------
    def get_line(self):
        self.buf = self.fin.readline()
        if self.buf == "":
            self.quit = 1

    # --------------------------------------------------------------------
    def get_cmd(self, pos):
        self.cmdline = self.edits[pos]
        self.cmd = self.cmdline[0:2].rstrip("\n\r\f")
        self.cmdargs = self.cmdline[2:]

    # --------------------------------------------------------------------
    def next_cmd(self):
        self.get_cmd(self.cmdpos)
        self.cmdpos += 1

    # --------------------------------------------------------------------
    def run(self):
        if self.cmd == "":
            return
        return self.cmds["cmd_" + self.cmd]()

    # --------------------------------------------------------------------
    def process(self):
        while not self.quit:
            self.next_cmd()
            try:
                self.run()
            except KeyError:
                print "%s not implemented" % self.cmd
                sys.exit(1)


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

e = Edit("PC5P08.COR", "crook_n8.asm", "crook_p8.asm")

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
