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
import tempfile

DEBUG = 0

R_OK = 0
R_ERR = 1
R_UNK = 2

# ------------------------------------------------------------------------
class EM400:

    # --------------------------------------------------------------------
    def __init__(self, binary, add_args, polldelay=0.01):
        self.polldelay = polldelay

        args = [ binary, "-u", "cmd" ] + add_args
        self.p = subprocess.Popen(args, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=1, universal_newlines=True)

    # --------------------------------------------------------------------
    def close(self):
        self.quit()
        self.p.wait()

    # --------------------------------------------------------------------
    def cmd_raw(self, command):

        self.p.stdin.write("%s\n" % command)
        return self.p.stdout.readline().strip()

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
        val = self.cmd("REG %s" % name)[0]
        return int(val, 0)

    # --------------------------------------------------------------------
    def state(self):
        state = self.cmd("STATE")[0]
        return state

    # --------------------------------------------------------------------
    def eval(self, expr):
        val = self.cmd("EVAL %s" % expr)[0]
        return int(val, 0)

    # --------------------------------------------------------------------
    def wait_for_finish(self):
        while True:
            s = self.state()
            if s == "WAIT":
                ir = self.reg("ir")
                if (ir & 0b1111110111000000) == 0b1110110000000000 and (ir & 0b0000000000111111) >= 0o40:
                    if self.ips() == 0:
                        break
            elif s == "STOP":
                break
            if self.polldelay:
                time.sleep(self.polldelay)

    # --------------------------------------------------------------------
    def wait_for_stop(self):
        while True:
            state = self.state()
            if state != "STOP":
                if self.polldelay:
                    time.sleep(self.polldelay)
            else:
                break

    # --------------------------------------------------------------------
    def clear(self):
        self.cmd("CLEAR")
        self.wait_for_stop()

    # --------------------------------------------------------------------
    def start(self):
        self.cmd("START")
        self.ips()

    # --------------------------------------------------------------------
    def stop(self):
        self.cmd("STOP")
        self.wait_for_stop()

    # --------------------------------------------------------------------
    def quit(self):
        self.cmd("QUIT")

# ------------------------------------------------------------------------
class TestResult:

    # --------------------------------------------------------------------
    def __init__(self, name):
        self.name = None
        self.passed = None
        self.checks = []
        self.error = None
        self.ips = None
        self.ips_percent = None
        self.failcmds = []

    # --------------------------------------------------------------------
    def add_check(self, t, x, r):
        self.checks += [(t, x, r)]
        if self.passed is None:
            self.passed = (x == r)
        else:
            self.passed &= (x == r)

    # --------------------------------------------------------------------
    def __str__(self):
        # error
        if self.error:
            return "%-60s %s" % (t, self.error)

        # benchmark
        if self.ips:
            if self.ips_percent:
                pc = "(%+.1f%%)" % self.ips_percent
            else:
                pc = ""
            return "%-60s %7.3f %s" % (t, self.ips, pc)

        # pass/fail test
        if self.passed is not None:
            pf = [ "\033[91mFAILED\033[0m", "\033[92mPASSED\033[0m" ]
            ret = "%-60s %s" % (t, pf[self.passed])
            for f in self.checks:
                if f[1] != f[2]:
                    ret += " %s=%i!=%i" % (f[0], f[2], f[1])
            return ret

        return "no result"

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, emas, binary, blfile, benchmark_duration=0.5, failcmd=None, log="", fpga=0, options=[]):
        self.emas = emas
        self.binary = binary
        self.failcmd = failcmd
        self.benchmark_duration = benchmark_duration
        self.e = None
        self.add_opts = None
        self.default_config = "configs/minimal.ini"
        self.bl = self.baseline(blfile)
        self.log = log
        self.fpga = fpga
        self.options = options

    # --------------------------------------------------------------------
    def close(self):
        if self.e:
            self.e.close()

    # --------------------------------------------------------------------
    def __runemu(self, add_opts):
        if self.log:
            add_opts += ["-l", self.log]
        if self.fpga:
            add_opts += ["-F"]
        if self.options:
            for o in self.options:
                add_opts += ["-O", o]

        if self.e is None:
            if DEBUG:
                print("Spawning fresh EM400: %s %s" % (self.binary, " ".join(add_opts)))
            self.e = EM400(self.binary, add_opts, polldelay=0.01)
        else:
            if self.add_opts != add_opts:
                if DEBUG:
                    print("Spawning EM400 with new options: %s %s" % (self.binary, " ".join(add_opts)))
                self.e.close()
                self.e = EM400(self.binary, add_opts, polldelay=0.01)
            else:
                if DEBUG:
                    print("Reusing existing EM400 instance")
        self.add_opts = add_opts

    # --------------------------------------------------------------------
    def baseline(self, bfile):
        if not bfile:
            return None

        print("Using baseline: %s" % bfile)
        baseline = {}
        with open(bfile) as f:
            for line in f:
                t = line.split()
                if len(t) == 2:
                    baseline[t[0].strip()] = float(t[1].strip())

        return baseline

    # --------------------------------------------------------------------
    def __assembly(self, source):
        aout = tempfile.gettempdir() + "/" + os.path.basename(source) + ".bin"
        args = [self.emas, "-D", "EM400", "-I", "include", "-O", "raw", "-o", aout, source]
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        p.wait()
        if p.returncode == 0:
            return aout
        else:
            o, e = p.communicate()
            raise RuntimeError(o.decode('ascii'))

    # --------------------------------------------------------------------
    def __gerparams(self, source):
        opts = []
        xpct = []
        precmd = []
        postcmd = []
        for l in open(source, "r"):
            # get OPTS directive
            if "OPTS" in l:
                try:
                    popts = re.findall(";[ \t]*OPTS[ \t]+(.*)", l)[0].split()
                    opts += popts
                except:
                    raise Exception("Malformed OPTS: %s" % l)
            # get test result conditions
            if "XPCT" in l:
                try:
                    pxpct = re.findall(";[ \t]*XPCT[ \t]+(.+):(.+)\n", l)
                    expr = pxpct[0][0].strip()
                    val = int(pxpct[0][1], 0) & 0xffff
                    xpct += [(expr, val)]
                except:
                    raise Exception("Malformed XPCT: %s" % l)
            # get pre-run commands
            if "PRECMD" in l:
                try:
                    pprecmd = re.findall(";[ \t]*PRECMD[ \t]+(.*)", l)
                    precmd += pprecmd
                except:
                    raise Exception("Malformed PRECMD: %s" % l)
            # get post-run commands
            if "POSTCMD" in l:
                try:
                    ppostcmd = re.findall(";[ \t]*POSTCMD[ \t]+(.*)", l)
                    postcmd += ppostcmd
                except:
                    raise Exception("Malformed POSTCMD: %s" % l)

        return opts, xpct, precmd, postcmd

    # --------------------------------------------------------------------
    def run(self, source):
        result = TestResult(source)

        try:
            opts, xpct, precmd, postcmd = self.__gerparams(source)
            aout = self.__assembly(source)
            self.__runemu(["-c", self.default_config] + opts)
            self.e.wait_for_stop()
            self.e.clear()
            self.e.load(0, 0, aout)
            self.e.cmd("CLOCK OFF");
            self.e.cmd("REG IC 0");
            if precmd:
                for c in precmd:
                    self.e.cmd(c)

            if xpct:
                self.__passfail(result, xpct)
            else:
                self.__benchmark(result, source)

            if postcmd:
                for c in postcmd:
                    self.e.cmd(c)

        except Exception as e:
            result.passed = 0
            result.error = str(e).rstrip()

        if result.passed == 0:
            if self.failcmd:
                for cmd in self.failcmd:
                    result.failcmds += [(cmd, self.e.cmd_raw(cmd))]

        return result

    # --------------------------------------------------------------------
    def __passfail(self, result, xpct):
        self.e.start()
        self.e.wait_for_finish()
        self.e.stop()
        for x in xpct:
            result.add_check(x[0], x[1], self.e.eval(x[0]))

    # --------------------------------------------------------------------
    def __benchmark(self, result, source):
        self.e.start()
        time.sleep(0.05)
        self.e.ips()
        time.sleep(self.benchmark_duration)
        ips = self.e.ips()
        self.e.stop()
        result.ips = ips/1000000.0
        result.passed = 1

        if self.bl and source in self.bl:
            diff = result.ips - self.bl[t]
            result.ips_percent = (diff*100.0) / self.bl[t]

# ------------------------------------------------------------------------
def collect_tests(i):
    tests = []

    if os.path.isfile(i) and i.endswith(".asm"):
        tests.append(i)
    elif os.path.isdir(i):
        for path, dirs, files in os.walk(i):
            for f in files:
                if f.endswith(".asm"):
                    tests.append("{}/{}".format(path, f))
    elif os.path.isfile(i) and i.endswith(".set"):
        with open(i) as f:
            for line in f:
                line = line.strip()
                if line.startswith((";", "#")):
                    continue
                tests += collect_tests(line)
    else:
        print("Skipping: {}".format(i))

    return tests

# ------------------------------------------------------------------------
# --- MAIN ---------------------------------------------------------------
# ------------------------------------------------------------------------

parser = argparse.ArgumentParser()
parser.add_argument("-b", "--baseline", help="baseline test results")
parser.add_argument("-e", "--emulator", help="emulator binary to run", default="../build/em400")
parser.add_argument("-f", "--failcmd", help="command to run when test fails", action='append')
parser.add_argument("-l", "--log", help="configure em400 logging", default="")
parser.add_argument("-F", "--fpga", help="use FPGA CPU backend in em400", action="store_const", const=1, default=0)
parser.add_argument("-O", "--option", help="add the following option when running em400", action='append')
parser.add_argument("-v", "--verbose", help="be verbose", action="store_const", const=1, default=0)
parser.add_argument('test', nargs='*', help='Test to run (asm source, directory or test set). Default set is run when no tests are provided.')
args = parser.parse_args()

DEBUG = args.verbose

# enumerate tests
if args.test:
    tests = []
    for i in args.test:
        tests.extend(collect_tests(i))
else:
    tests = collect_tests("sets/default.set")

tests.sort()

# run tests
total = 0
failed = 0
tb = TestBed("emas", args.emulator, args.baseline, benchmark_duration=0.5, failcmd=args.failcmd, log=args.log, fpga=args.fpga, options=args.option)
for t in tests:
    if not DEBUG:
        if sys.stdout.isatty():
            print("%-60s ..." % t, end="", flush=True)
    else:
        print("Starting test: %s" % t)

    result = tb.run(t)
    if not DEBUG:
        if sys.stdout.isatty():
            print("\r", end="", flush=True)
    print(result)
    total += 1
    if result.passed != 1:
        failed += 1
        if result.failcmds:
            for f in result.failcmds:
                print("   +++ %s: %s" % (f[0], f[1]))

tb.close()

print("----------------------------------------------------------------------")
print("Tests run: %i, failed: %i" % (total, failed))

sys.exit(failed)

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
