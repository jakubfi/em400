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
import os.path
import sys
import re
import subprocess

assem = "emas"
em400 = "../build/em400"

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, tests):
        self.tests = tests

    def run(self):
        total = 0
        failed = 0
        for test_file in self.tests:
            print("%-30s : RUNNING..." % test_file.replace("./", "")),
            sys.stdout.flush()
            test = Test(test_file)
            result, details = test.run()
            total += 1
            print("\r%-30s : %s%s      " % (test_file.replace("./", ""), result, details))
            if result != "PASSED":
                failed += 1
        print("--------------------------------------------")
        print("Tests run: %i, failed: %i" % (total, failed))

# ------------------------------------------------------------------------
class Test:

    # --------------------------------------------------------------------
    def __init__(self, source):
        self.source = source
        self.prog_name = source
        self.config = "configs/minimal.cfg"

    # ------------------------------------------------------------------------
    def run(self):
        details = ""
        result = "?"

        try:
            phase = "Assembly"
            self.assembly()
            phase = "Parse"
            self.parse()
            phase = "Run"
            self.execute()
            result = "PASSED"
            phase = ""
        except Exception as e:
            result = "FAILED"
            details = " at %s: %s" % (phase, str(e))

        return result, details

    # ------------------------------------------------------------------------
    def assembly(self):
        self.output = "/tmp/out.bin"
        args = [assem, "-Iinclude", "-Oraw", "-o" + self.output, self.source]
        subprocess.check_output(args)

    # ------------------------------------------------------------------------
    def parse(self):
        self.pre = ""
        self.test_expr = ""
        self.expected = ""

        xpcts = []
        pre_tab = []

        for l in open(self.source, "r"):
            # get program name
            pname = re.findall(".prog[ \t]+\"(.*)\"", l)
            if pname:
                self.prog_name = pname[0]

            # get CONFIG directive
            if "CONFIG" in l:
                pcfg = re.findall(";[ \t]*CONFIG[ \t]+(.*)", l)
                if pcfg:
                    if not os.path.isfile(pcfg[0]):
                        raise Exception("Config '%s' does not exist" % pcfg[0])
                    self.config = pcfg[0]
                else:
                    raise Exception("Incomplete CONFIG directive")

            # get PRE expressions
            if "PRE" in l:
                ppre = re.findall(";[ \t]*PRE[ \t]+(.*)", l)
                if ppre:
                    pre_tab.append(ppre[0])
                else:
                    raise Exception("Incomplete PRE found")

            # get test result conditions
            if "XPCT" in l:
                xpct = re.findall(";[ \t]*XPCT[ \t]+([^ \t]+)[ \t]*:[ \t]*([^ \t]+.*)\n", l)
                if xpct and len(xpct[0]) == 2:
                    xpcts.append(xpct[0])
                else:
                    raise Exception("Incomplete XPCT found")

        if pre_tab:
            self.pre = ','.join(pre_tab)

        if not xpcts:
            raise Exception("No XPCTs foud")

        self.test_expr = ', '.join([x[0] for x in xpcts])
        self.expected = ' '.join([x[1] for x in xpcts])

    # ------------------------------------------------------------------------
    def execute(self):
        args = [em400, "-c", self.config, "-p", self.output, "-t", self.test_expr]
        if self.pre:
            args += ["-x", self.pre]

        o = subprocess.check_output(args).decode("utf-8")

        tres = re.findall("TEST RESULT @ (.*, regs: .*): (.*)\n", o)

        if not tres:
            raise Exception("No test result found")

        self.result = tres[0][1].strip(" ")
        address = tres[0][0].strip(" ")

        if self.result != self.expected:
            raise Exception("@ %s\n%32s xpct: '%s'\n%32s  got: '%s'" % (address, "", self.expected, "", self.result))


# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

if len(sys.argv) != 1:
    tests = sys.argv[1:]
else:
    tests = []
    for path, dirs, files in os.walk("."):
        for f in files:
            if f.endswith(".asm") and "benchmark" not in path and "include" not in path:
                tests.append(os.path.join(path, f))
    tests.sort()

# run tests

t = TestBed(tests)
t.run()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
