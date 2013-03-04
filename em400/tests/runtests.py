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
import re
import subprocess

assem = "../../assem/assem"
em400 = "../build/em400"

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, tests):
        self.tests = tests

    def run(self):
        for test_file in self.tests:
            test = Test(test_file)
            result, details = test.run()
            print "%-30s : %s%s" % (test.prog_name, result, details)

# ------------------------------------------------------------------------
class Test:

    # --------------------------------------------------------------------
    def __init__(self, source):
        self.source = source
        self.prog_name = source

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
        except Exception, e:
            result = "FAILED"
            details = " (at %s: %s)" % (phase, str(e))

        return result, details

    # ------------------------------------------------------------------------
    def assembly(self):
        self.output = "/tmp/out.bin"
        args = [assem, self.source, self.output]
        subprocess.check_output(args)

    # ------------------------------------------------------------------------
    def parse(self):
        self.pre = ""
        self.test_expr = ""
        self.expected = ""

        xpcts = []
        pre_tab = []

        for l in open(self.source):

            # get program name
            pname = re.findall(".program[ \t]+\"(.*)\"", l)
            if pname:
                self.prog_name = pname[0]

            # get PRE expressions
            if re.match(".*PRE.*", l):
                ppre = re.findall(";[ \t]*PRE[ \t]+(.*)", l)
                if ppre:
                    pre_tab.append(ppre[0])
                else:
                    raise Exception("Incomplete PRE found")

            # get test result conditions
            if re.match(".*XPCT.*", l):
                xpct = re.findall(";[ \t]*XPCT[ \t]+([^ \t:]+)[ \t]*:[ \t]*([^ \t]+.*)\n", l)
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
        args = [em400, "-p", self.output, "-t", self.test_expr]
        if self.pre:
            args += ["-x", self.pre]

        o = subprocess.check_output(args)

        tres = re.findall("TEST RESULT: (.*) \n", o)

        if not tres:
            raise Exception("No test result found")

        self.result = tres[0]

        if self.result != self.expected:
            raise Exception("Test failed, expected: %s got: %s" % (self.expected, self.result))


# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

if len(sys.argv) != 1:
    tests = sys.argv[1:]
else:
    tests = []
    for path, dirs, files in os.walk("."):
        for f in files:
            if f.endswith(".asm"):
                tests.append(os.path.join(path, f))
    tests.sort()

# run tests

t = TestBed(tests)
t.run()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
