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

HLT_PASS = "hlt 077"

# ------------------------------------------------------------------------
def normalize_expr(expr):
    return re.sub(r"\s", "", expr).lower()

# ------------------------------------------------------------------------
class EmulatorDied(Exception):
    pass

# ------------------------------------------------------------------------
class EM400:

    # --------------------------------------------------------------------
    def __init__(self, binary, add_args, polldelay=0.01, timeout=5):
        self.polldelay = polldelay
        self.timeout = timeout

        args = [ binary, "-u", "cmd" ] + add_args
        self.p = subprocess.Popen(args, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=1, universal_newlines=True)

    # --------------------------------------------------------------------
    def close(self):
        try:
            self.quit()
        except EmulatorDied:
            return
        self.p.wait()

    # --------------------------------------------------------------------
    def kill(self):
        self.p.kill()
        self.p.wait()
        for f in (self.p.stdin, self.p.stdout):
            try:
                f.close()
            except BrokenPipeError:
                pass

    # --------------------------------------------------------------------
    def __send(self, command):
        try:
            self.p.stdin.write("%s\n" % command)
            resp = self.p.stdout.readline()
        except BrokenPipeError:
            resp = ""
        if not resp:
            self.kill()
            raise EmulatorDied("em400 died (exit code %i)" % self.p.returncode)
        return resp

    # --------------------------------------------------------------------
    def cmd_raw(self, command):
        return self.__send(command).strip()

    # --------------------------------------------------------------------
    def cmd(self, command):
        if DEBUG: print("--> %s" % command)
        resp = self.__send(command)
        if DEBUG: print("<-- %s" % resp)

        ret = R_UNK
        if resp.startswith("OK"):
            ret = R_OK
        elif resp.startswith("ERR"):
            ret = R_ERR

        if ret != R_OK:
            raise RuntimeError(re.sub("[A-Za-z]+: ", "", resp))
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
    def wait_for_finish(self, max_runtime=0):
        started = time.monotonic()
        hung_since = None
        while True:
            hung = False
            s = self.state()
            if s == "STOP":
                return "stop"
            if s == "WAIT":
                ir = self.reg("ir")
                if self.ips() == 0:
                    # HLT with an argument >= 0o40 means "test finished"
                    if (ir & 0b1111110111000000) == 0b1110110000000000 and (ir & 0b0000000000111111) >= 0o40:
                        return "hlt %03o" % (ir & 0b0000000000111111)
                    hung = True
            if not hung:
                hung_since = None
            elif hung_since is None:
                hung_since = time.monotonic()
            elif time.monotonic() - hung_since >= self.timeout:
                raise TimeoutError("halted with no activity for %gs" % self.timeout)
            if max_runtime and time.monotonic() - started >= max_runtime:
                raise TimeoutError("still running after %gs" % max_runtime)
            if self.polldelay:
                time.sleep(self.polldelay)

    # --------------------------------------------------------------------
    def wait_for_stop(self):
        deadline = time.monotonic() + self.timeout
        while True:
            state = self.state()
            if state == "STOP":
                break
            if time.monotonic() >= deadline:
                raise TimeoutError("no stop within %gs" % self.timeout)
            if self.polldelay:
                time.sleep(self.polldelay)

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

    PASS, FAIL, ERROR, TIMEOUT, BENCH = range(5)

    # --------------------------------------------------------------------
    def __init__(self, name):
        self.name = name
        self.status = None
        self.checks = []
        self.error = None
        self.ips = None
        self.ips_percent = None
        self.failcmds = []
        self.elapsed = None

    # --------------------------------------------------------------------
    def failed(self):
        return self.status not in (self.PASS, self.BENCH)

    # --------------------------------------------------------------------
    def add_check(self, expr, expected, got):
        self.checks += [(expr, expected, got)]
        if expected != got:
            self.status = self.FAIL
        elif self.status is None:
            self.status = self.PASS

    # --------------------------------------------------------------------
    def __elapsed(self):
        if self.elapsed is None:
            return ""
        return "  [%.3fs]" % self.elapsed

    # --------------------------------------------------------------------
    def __str__(self):
        if self.status == self.ERROR:
            return "%-60s %s%s" % (self.name, self.error, self.__elapsed())

        if self.status == self.TIMEOUT:
            return "%-60s \033[91mTIMEOUT\033[0m %s%s" % (self.name, self.error, self.__elapsed())

        if self.status == self.BENCH:
            if self.ips_percent:
                pc = "(%+.1f%%)" % self.ips_percent
            else:
                pc = ""
            return "%-60s %7.3f %s" % (self.name, self.ips, pc)

        if self.status in (self.PASS, self.FAIL):
            pf = { self.FAIL: "\033[91mFAILED\033[0m", self.PASS: "\033[92mPASSED\033[0m" }
            ret = "%-60s %s" % (self.name, pf[self.status])
            for expr, expected, got in self.checks:
                if expected != got:
                    ret += " %s=%s!=%s" % (expr, got, expected)
            return ret + self.__elapsed()

        return "%-60s no result%s" % (self.name, self.__elapsed())

# ------------------------------------------------------------------------
class TestBed:

    # --------------------------------------------------------------------
    def __init__(self, emas, binary, blfile, benchmark_duration=0.5, failcmd=None, log="", options=None, timeout=5, max_runtime=20, config=None):
        self.emas = emas
        self.binary = binary
        self.failcmd = failcmd
        self.benchmark_duration = benchmark_duration
        self.timeout = timeout
        self.max_runtime = max_runtime
        self.e = None
        self.add_opts = None
        self.default_config = "configs/minimal.ini"
        self.config_override = config
        self.bl = self.baseline(blfile)
        self.log = log
        self.options = options

    # --------------------------------------------------------------------
    def close(self):
        if self.e:
            self.e.close()

    # --------------------------------------------------------------------
    def __runemu(self, add_opts):
        if self.log:
            add_opts += ["-l", self.log]
        if self.options:
            for o in self.options:
                add_opts += ["-O", o]

        if self.e is None:
            if DEBUG:
                print("Spawning fresh EM400: %s %s" % (self.binary, " ".join(add_opts)))
            self.e = EM400(self.binary, add_opts, polldelay=0.01, timeout=self.timeout)
        else:
            if self.add_opts != add_opts:
                if DEBUG:
                    print("Spawning EM400 with new options: %s %s" % (self.binary, " ".join(add_opts)))
                self.e.close()
                self.e = EM400(self.binary, add_opts, polldelay=0.01, timeout=self.timeout)
            else:
                if DEBUG:
                    print("Reusing existing EM400 instance")
        self.add_opts = add_opts

    # --------------------------------------------------------------------
    def __killemu(self):
        if self.e:
            self.e.kill()
            self.e = None

    # --------------------------------------------------------------------
    def baseline(self, bfile):
        if not bfile:
            return None

        print("Using baseline: %s" % bfile)
        baseline = {}
        with open(bfile) as f:
            for line in f:
                t = line.split()
                if len(t) >= 2:
                    try:
                        baseline[os.path.normpath(t[0])] = float(t[1])
                    except ValueError:
                        pass

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
    def __getparams(self, source):
        opts = []
        benchmark = False
        xpct = []
        precmd = []
        postcmd = []
        max_runtime = self.max_runtime
        for l in open(source, "r"):
            if re.match(r"[ \t]*;[ \t]*BENCHMARK[ \t]*$", l):
                benchmark = True
                continue
            m = re.match(r"[ \t]*;[ \t]*(CONFIG|XPCT|PRECMD|POSTCMD|TIMEOUT)[ \t]+(.+?)[ \t]*$", l)
            if not m:
                continue
            directive, arg = m.groups()
            if directive == "CONFIG":
                if not self.config_override:
                    opts += ["-c", arg]
            elif directive == "TIMEOUT":
                try:
                    t = float(arg)
                    if t <= 0:
                        raise ValueError
                except ValueError:
                    raise SyntaxError("Malformed TIMEOUT: %s" % arg)
                if self.max_runtime:
                    max_runtime = t
            elif directive == "XPCT":
                # split on the last colon, the expression itself may contain ':' ("[seg:addr]")
                expr, sep, val = arg.rpartition(":")
                try:
                    if not sep:
                        raise ValueError
                    xpct += [(expr.strip(), int(val, 0) & 0xffff)]
                except ValueError:
                    raise SyntaxError("Malformed XPCT: %s" % arg)
            elif directive == "PRECMD":
                precmd += [arg]
            elif directive == "POSTCMD":
                postcmd += [arg]

        if benchmark and xpct:
            raise SyntaxError("BENCHMARK with XPCT")

        return opts, benchmark, xpct, precmd, postcmd, max_runtime

    # --------------------------------------------------------------------
    def run(self, source):
        result = TestResult(source)
        started = time.monotonic()

        try:
            opts, benchmark, xpct, precmd, postcmd, max_runtime = self.__getparams(source)
            aout = self.__assembly(source)
            self.__runemu(["-c", self.config_override or self.default_config] + opts)
            self.e.wait_for_stop()
            self.e.cmd("CLOCK OFF")
            self.e.clear()
            self.e.load(0, 0, aout)
            self.e.cmd("REG IC 0")
            if precmd:
                for c in precmd:
                    self.e.cmd(c)

            if benchmark:
                self.__benchmark(result, source)
            else:
                self.__passfail(result, xpct + self.__health_checks(xpct), max_runtime)

            if postcmd:
                for c in postcmd:
                    self.e.cmd(c)

        except TimeoutError as e:
            result.status = TestResult.TIMEOUT
            result.error = str(e)
            # em400 state is unknown at this point, don't reuse the instance
            self.__killemu()
        except EmulatorDied as e:
            result.status = TestResult.ERROR
            result.error = str(e)
            self.e = None
        except Exception as e:
            result.status = TestResult.ERROR
            result.error = str(e).rstrip()

        result.elapsed = time.monotonic() - started

        if result.failed() and self.failcmd and self.e:
            for cmd in self.failcmd:
                result.failcmds += [(cmd, self.e.cmd_raw(cmd))]

        return result

    # --------------------------------------------------------------------
    # Checks that every test should pass (except when explicitely set otherwise by the test)
    def __health_checks(self, xpct):
        declared = {normalize_expr(expr) for expr, val in xpct}
        checks = []
        # no alarm
        if "alarm" not in declared:
            checks += [("alarm", 0)]
        # no pending illegal instruction interrupt
        if not declared & {"rz", "rz[6]"}:
            checks += [("rz[6]", 0)]
        return checks

    # --------------------------------------------------------------------
    def __passfail(self, result, xpct, max_runtime):
        self.e.start()
        finished = self.e.wait_for_finish(max_runtime)
        # pressing STOP clears the alarm
        alarm = self.e.eval("alarm")
        self.e.stop()
        if finished != "stop":
            result.add_check("finish", HLT_PASS, finished)
        for expr, expected in xpct:
            got = alarm if normalize_expr(expr) == "alarm" else self.e.eval(expr)
            result.add_check(expr, expected, got)

    # --------------------------------------------------------------------
    def __benchmark(self, result, source):
        self.e.start()
        time.sleep(0.05)
        self.e.ips()
        time.sleep(self.benchmark_duration)
        ips = self.e.ips()
        self.e.stop()
        result.ips = ips/1000000.0
        result.status = TestResult.BENCH

        if self.bl and source in self.bl:
            diff = result.ips - self.bl[source]
            result.ips_percent = (diff*100.0) / self.bl[source]

# ------------------------------------------------------------------------
def collect_tests(i):
    tests = []

    if os.path.isfile(i) and i.endswith(".asm"):
        tests.append(os.path.normpath(i))
    elif os.path.isdir(i):
        for path, dirs, files in os.walk(i):
            for f in files:
                if f.endswith(".asm"):
                    tests.append(os.path.normpath(os.path.join(path, f)))
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
parser.add_argument("-c", "--config", help="override config file for all tests, ignoring per-test CONFIG directives")
parser.add_argument("-e", "--emulator", help="emulator binary to run", default="../build/em400")
parser.add_argument("-f", "--failcmd", help="command to run when test fails", action='append')
parser.add_argument("-l", "--log", help="configure em400 logging", default="")
parser.add_argument("-O", "--option", help="add the following option when running em400", action='append')
parser.add_argument("-t", "--timeout", help="fail a test halted with no activity for this many seconds (default: 5)", type=float, default=5)
parser.add_argument("-T", "--max-runtime", help="fail a test still running after this many seconds, overridden by a test's TIMEOUT directive, 0 disables (default: 20)", type=float, default=20)
parser.add_argument("-x", "--exitfirst", help="stop after the first failed test", action="store_true")
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
tb = TestBed("emas", args.emulator, args.baseline, benchmark_duration=0.5, failcmd=args.failcmd, log=args.log, options=args.option, timeout=args.timeout, max_runtime=args.max_runtime, config=args.config)
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
    if result.failed():
        failed += 1
        if result.failcmds:
            for f in result.failcmds:
                print("   +++ %s: %s" % (f[0], f[1]))
        if args.exitfirst:
            break

tb.close()

print("----------------------------------------------------------------------")
print("Tests run: %i, failed: %i" % (total, failed))

sys.exit(failed)

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
