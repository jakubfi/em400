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
em400 = "../build/src/em400"

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, tests):
        self.tests = tests

    def run(self):
        for test_file in self.tests:
            result = {}
            result_avg = 0
            count = 0
            test = Benchmark(test_file)
            for i in range(0, 10):
                res, details = test.run()
                result[i] = float(res)
                if details:
                    break;
            result_max = -1
            for i in range(0, 10):
                if result[i] > result_max:
                    result_max = result[i]

            print("%-40s : %.3f%s" % (test_file.replace("./", ""), result_max/1000000, details))

# ------------------------------------------------------------------------
class Benchmark:

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
            phase = "Run"
            result = self.execute()
            phase = ""
        except Exception as e:
            result = "FAILED"
            details = " at %s: %s" % (phase, str(e))

        return result, details

    # ------------------------------------------------------------------------
    def assembly(self):
        self.output = "/tmp/out.bin"
        args = [assem, "-Oraw", "-o" + self.output, self.source]
        subprocess.check_output(args)

    # ------------------------------------------------------------------------
    def execute(self):
        args = [em400, "-c", self.config, "-p", self.output, "-be"]

        o = subprocess.check_output(args).decode("utf-8")

        tres = re.findall("IPS: ([0-9]+) .*\n", o)

        if not tres:
            raise Exception("No benchmark result found")

        return tres[0].strip(" ")


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
