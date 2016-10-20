#!/usr/bin/env python3

#  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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
import time
import subprocess
import argparse

DEBUG = 0

R_OK = 0
R_ERR = 1
R_UNK = 2

# ------------------------------------------------------------------------
class EM400:

    # --------------------------------------------------------------------
    def __init__(self, binary, add_args, polldelay=0.05):
        self.polldelay = polldelay

        args = [ binary, "-u", "cmd" ] + add_args
        self.p = subprocess.Popen(args, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=1, universal_newlines=True)

    # --------------------------------------------------------------------
    def close(self):
        self.quit()
        self.p.wait()

    # --------------------------------------------------------------------
    def cmd(self, command):
        if DEBUG: print("--> %s" % command)
        self.p.stdin.write("%s\n" % command)
        resp = self.p.stdout.readline()
        if DEBUG: print("<-- %s" % resp)

        ret = R_UNK
        if resp.startswith("OK"):
            ret = R_OK
        elif resp.startswith("ERR"):
            ret = R_ERR

        if ret != R_OK:
            raise SystemError(re.sub("[A-Za-a]+: ", "", resp))
        else:
            return resp.split()[1:]

    # --------------------------------------------------------------------
    def load(self, seg, addr, filename):
        self.cmd("LOAD %i %i %s" % (seg, addr, filename))

    # --------------------------------------------------------------------
    def ips(self):
        ips = self.cmd("IPS")[0]
        return int(ips)

    # --------------------------------------------------------------------
    def reg(self, name):
        val = self.cmd("REG %s" % name)[0].split("=")[1]
        return int(val, 0)

    # --------------------------------------------------------------------
    def state(self):
        state = self.cmd("STATE")[0]
        return int(state, 0)

    # --------------------------------------------------------------------
    def is_running(self):
        return self.state() == 0

    # --------------------------------------------------------------------
    def wait_for_hlt(self):
        while not (self.state() & 2):
            time.sleep(self.polldelay)
        return self.reg("IR") & 0b111111

    # --------------------------------------------------------------------
    def clear(self):
        self.cmd("CLEAR")
        while self.is_running():
            time.sleep(self.polldelay)

    # --------------------------------------------------------------------
    def start(self):
        self.cmd("START")

    # --------------------------------------------------------------------
    def stop(self):
        self.cmd("STOP")

    # --------------------------------------------------------------------
    def quit(self):
        self.cmd("QUIT")

# ------------------------------------------------------------------------
class TestResult:

    # --------------------------------------------------------------------
    def __init__(self, name):
        self.name = None
        self.passfail = None
        self.error = None
        self.ips = None
        self.ips_percent = None

    # --------------------------------------------------------------------
    def __str__(self):
        # error
        if self.error:
            return "%-40s : %s" % (t, self.error)

        # benchmark
        if self.ips:
            if self.ips_percent:
                pc = "(%+.1f%%)" % self.ips_percent
            else:
                pc = ""
            return "%-40s : %7.3f %s" % (t, self.ips, pc)

        # pass/fail test
        if self.passfail:
            return "n/a"

        return "no result"

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, emas, binary, blfile, benchmark_duration=0.5):
        self.emas = emas
        self.binary = binary
        self.benchmark_duration = benchmark_duration
        self.e = None
        self.add_opts = None
        self.default_config = "configs/minimal.cfg"
        self.bl = self.baseline(blfile)

    # --------------------------------------------------------------------
    def close(self):
        if self.e:
            self.e.close()

    # --------------------------------------------------------------------
    def __runemu(self, add_opts):
        if self.e is None:
            self.e = EM400(self.binary, add_opts)
        else:
            if self.add_opts != add_opts:
                self.e.close()
                self.e = EM400(self.binary, add_opts)
        self.add_opts = add_opts

    # --------------------------------------------------------------------
    def baseline(self, bfile):
        if not bfile:
            return None

        print("Using baseline: %s" % bfile)
        baseline = {}
        with open(bfile) as f:
            for line in f:
                t = line.split(":")
                if len(t) == 2:
                    baseline[t[0].strip()] = float(t[1].strip())

        return baseline

    # --------------------------------------------------------------------
    def __assembly(self, source):
        aout = re.sub(".*/(.*).asm$", "/tmp/\\1.bin", source)
        args = [self.emas, "-I../tests/include", "-Oraw", "-o" + aout, source]
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        p.wait()
        if p.returncode == 0:
            return aout
        else:
            o, e = p.communicate()
            raise RuntimeError(o.decode('ascii'))

    # --------------------------------------------------------------------
    def __prepare(self, source, add_opts):
        aout = self.__assembly(source)
        self.__runemu(add_opts)

        self.e.clear()
        self.e.load(0, 0, aout)

    # --------------------------------------------------------------------
    def passfail(self, source):
        result = TestResult(source)
        try:
            self.__prepare(source, ["-c", self.default_config])
            self.e.start()
            self.e.stop()
        except Exception as e:
            result.error = str(e).rstrip()

        return result

    # --------------------------------------------------------------------
    def benchmark(self, source):
        result = TestResult(source)

        try:
            self.__prepare(source, ["-c", self.default_config])
            self.e.start()
            time.sleep(0.05)
            self.e.ips()
            time.sleep(self.benchmark_duration)
            ips = self.e.ips()
            self.e.stop()
            result.ips = ips/1000000.0

            if self.bl and source in self.bl:
                diff = result.ips - self.bl[t]
                result.ips_percent = (diff*100.0) / self.bl[t]

        except Exception as e:
            result.error = str(e).rstrip()

        return result

# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

parser = argparse.ArgumentParser()
parser.add_argument("-b", "--baseline", help="baseline test results")
parser.add_argument('test', nargs='*', help='selected test(s) to run')
args = parser.parse_args()

# enumerate tests
if args.test:
    tests = args.test
else:
    tests = []
    for path, dirs, files in os.walk("."):
        for f in files:
            if f.endswith(".asm"):
                tests.append(os.path.join(path, f).lstrip("./"))
    tests.sort()

# run tests
tb = TestBed("emas", "../build/src/em400", args.baseline, benchmark_duration=0.5)
for t in tests:
    result = tb.benchmark(t)
    print(result)

tb.close()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
