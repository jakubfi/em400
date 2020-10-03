#!/usr/bin/env python3

import random
import re
import subprocess
import sys
import serial
import struct
import os
import time
import signal
import socket
import argparse
import struct

# ------------------------------------------------------------------------
class KZ():
    def __init__(self, s, debug=False, echo_cancel=False):
        try:
            s.flushInput()
            s.flushOutput()
        except AttributeError:
            pass
        self.s = s
        self.debug = debug
        self.echo_cancel = echo_cancel

    def _read(self, cnt):
        if self.debug:
            print("Waiting for {} bytes of data".format(cnt))
        try:
            data = self.s.recv(cnt, socket.MSG_WAITALL)
        except AttributeError:
            data = self.s.read(cnt)
        return data

    def _write(self, data):
        try:
            res = self.s.sendall(data, socket.MSG_WAITALL)
        except AttributeError:
            res = self.s.write(data)
        if self.echo_cancel:
            dcheck = self._read(len(data))
            if data != dcheck:
                raise Exception("Echo mismatch! wrote {}, got {}".format(data, dcheck))
        return res

    def read_word(self):
        res = self._read(2)
        x = (res[0]<<8) + res[1]
        if self.debug:
            print("Read word: {}".format(x))
        return x

    def read_dword(self):
        res = self._read(4)
        x = (res[0]<<24) + (res[1]<<16) + (res[2]<<8) + res[3]
        if self.debug:
            print("Read dword: {}".format(x))
        return x

    def read_byte(self):
        x = ord(self._read(1))
        if self.debug:
            print("Read byte: {}".format(x))
        return x

    def read_bytes(self, num):
        return self._read(num)

    def write_byte(self, x):
        if self.debug:
            print("Write byte: {}".format(x))
        data = bytes([x & 255])
        return self._write(data)

    def write_word(self, x):
        if self.debug:
            print("Write word: {}".format(x))
        data = bytes([(x>>8) & 255, x & 255])
        return self._write(data)

    def write_dword(self, x):
        if self.debug:
            print("Write dword: {}".format(x))
        data = bytes([(x>>24) & 255, (x>>16) & 255, (x>>8) & 255, x & 255])
        return self._write(data)

    def write_bytes(self, data):
        if self.debug:
            print("Write bytes: {}".format([x for x in data]))
        return self._write(data)


# ------------------------------------------------------------------------
def bencode(val, octets):
    return bytes(reversed(
        [(val >> (8*i)) & 0xff for i in range(0, octets)]
    ))

# ------------------------------------------------------------------------
class EMAS:
    def __init__(self, binary="emas"):
        self.emas = binary

    # --------------------------------------------------------------------
    def process(self, source):
        args = [self.emas, "-Oraw", "-o-" , source]
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        p.wait()
        o, e = p.communicate()
        if p.returncode == 0:
            return o
        else:
            raise RuntimeError(e.decode('ascii'))

# ------------------------------------------------------------------------
class Seq:
    def __init__(self, datatype, sequence):
        self.datatype = datatype
        self.sequence = sequence
        self.infinite = False
        self.current = None
        self.reset()

    def reset(self):
        self.i = iter(self.sequence)

    def advance(self):
        self.current = next(self.i)

    def __str__(self):
        return str(self.datatype(self.current))

    def __bytes__(self):
        return bencode(self.current, self.datatype.size*2)

# ------------------------------------------------------------------------
class Rnd:
    def __init__(self, datatype, start, end, bitmask=-1):
        self.datatype = datatype
        self.bitmask = bitmask
        self.start = start
        self.end = end
        self.infinite = True
        self.current = None

    def reset(self):
        pass

    def advance(self):
        self.current = random.randrange(self.start, self.end) & self.bitmask

    def __str__(self):
        return str(self.datatype(self.current))

    def __bytes__(self):
        return bencode(self.current, self.datatype.size*2)

# ------------------------------------------------------------------------
class ArgProduct:
    def __init__(self, limit, iterators):
        self.iterators = iterators
        self.limit = limit

    def len(self):
        return sum([x.datatype.size for x in self.iterators])

    def advance(self):
        if self.limit is not None and self.limit <= 0:
            raise StopIteration

        if not self.iterators:
            raise StopIteration

        result = []
        need_next = True
        finite_seqs = False
        for i in reversed(self.iterators):  # start from least significant
            if i.infinite:
                i.advance()
            else:
                finite_seqs = True
                if i.current is None or need_next:
                    try:
                        i.advance()
                        need_next = False
                    except StopIteration:
                        i.reset()
                        i.advance()
                        need_next = True
        if need_next and finite_seqs:
            raise StopIteration
        if self.limit is not None:
            self.limit -= 1

    def __str__(self):
        data = ', '.join([str(i) for i in self.iterators])
        return data

    def __bytes__(self):
        return b''.join([bytes(i) for i in self.iterators])

# ------------------------------------------------------------------------
class Word:
    size = 1

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_bytes(cls, b):
        val = struct.unpack(">H", b)[0]
        return cls(val)

    def __str__(self):
        return "{:04x}".format(self.val)

# ------------------------------------------------------------------------
class Int:
    size = 1

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_bytes(cls, b):
        val = struct.unpack(">h", b)[0]
        return cls(val)

    def __str__(self):
        return str(self.val)

# ------------------------------------------------------------------------
class Dword:
    size = 2

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_bytes(cls, b):
        v = struct.unpack(">I", b)[0]
        return cls(v)

    def __str__(self):
        return "{:08x}".format(self.val)

# ------------------------------------------------------------------------
class Dint:
    size = 2

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_bytes(cls, b):
        v = struct.unpack(">i", b)[0]
        return cls(v)

    def __str__(self):
        return str(self.val)

# ------------------------------------------------------------------------
class Flags:
    size = 1

    def __init__(self, val):
        self.val = val

    @classmethod
    def from_bytes(cls, b):
        v = struct.unpack(">H", b)[0]
        return cls(v)

    def __str__(self):
        flags = "ZMVCLEGYX1234567"
        out = ""
        for i in range(0, 16):
            if (self.val >> (15-i)) & 1:
                out += flags[i]
        if out == "":
            out = "none"
        return "{}".format(out)

# ------------------------------------------------------------------------
class Float:
    size = 3

    def __init__(self, val):
        self.val = val
        w1 = val & 0xffff
        w2 = (val>>16) & 0xffff
        w3 = (val>>32) & 0xffff
        self.m = (w1<<24) | (w2<<8) | (w3 >> 8) & 0xff
        self.e = w3 & 0xff

    @classmethod
    def from_bytes(cls, b):
        t = struct.unpack(">HHH", b)
        v = (t[0] << 32) | (t[1] << 16) | (t[2])
        return cls(v)

    def __str__(self):
        return ".{:010x} * 2^{:02x}".format(self.m, self.e)

# ------------------------------------------------------------------------
class ComparativeTest:
    def __init__(self, sys_a, sys_b, limit=None, verbose=0):
        self.result = []
        self.systems = [x for x in [sys_a, sys_b] if x is not None]
        self.global_limit = limit
        self.verbose = verbose
        self.emas = EMAS()
        self.reset()

    def reset(self):
        self.filename = None
        self.args = None
        self.limit = None
        self.output_size = None
        self.result = []

    def load(self, filename):
        self.reset()
        self.filename = filename
        self.get_params()
        self.assemble()

    def get_params(self):
        
        f = open(self.filename, "r")
        inputs = []

        for line in f:
            if "INPUT" in line:
                try:
                    s = re.search(";[ \t]*INPUT[ \t]+(.*)", line)
                    inputs.append(eval(s.group(1)))
                except:
                    raise Exception("Malformed INPUT: %s" % line)

            if "OUTPUT" in line:
                try:
                    s = re.search(";[ \t]*OUTPUT[ \t]+(.*)", line)
                    self.result.append(eval((s.group(1))))
                except:
                    raise Exception("Malformed OUTPUT: %s" % line)

            if "LIMIT" in line:
                try:
                    s = re.search(";[ \t]*LIMIT[ \t]+(.*)", line)
                    self.limit = int(s.group(1))
                except:
                    raise Exception("Malformed LIMIT: %s" % line)

        limit = self.global_limit if self.global_limit is not None else self.limit
        self.output_size = sum([x.size for x in self.result])
        self.args = ArgProduct(limit, inputs)
        f.close()

    def assemble(self):
        self.data = self.emas.process(self.filename)

    def send_test(self):
        print('Test "{}" ({} words): input {} words, output {} words. Limit: {}'
            .format(self.filename, len(self.data)//2, self.args.len(), self.output_size, self.args.limit)
        )

        for s in self.systems:
            s.write_word(len(self.data)//2)
            s.write_word(self.args.len())
            s.write_word(self.output_size)
            s.write_bytes(self.data)

    def run(self):
        self.send_test()
        failed = 0
        count = 0
        finished = False

        while not finished:
            if terminate:
                for s in self.systems:
                    s.write_byte(2)
                print("Test gracefully terminated")
                break

            try:
                self.args.advance()
            except StopIteration:
                if not self.args.len():
                    finished = True
                    pass
                else:
                    break

            for s in self.systems:
                s.write_byte(1)
                if self.args.len():
                    s.write_bytes(bytes(self.args))

            result = []
            for s in self.systems:
                system_result = []
                for i in self.result:
                    result_raw = s.read_bytes(i.size*2)
                    system_result.append(i.from_bytes(result_raw))
                result.append(system_result)

            count += 1;

            i  = str(self.args)
            o1 = ", ".join([str(x) for x in result[0]])
            if len(self.systems) == 2:
                o2 = ", ".join([str(x) for x in result[1]])

                if [x.val for x in result[0]] == [x.val for x in result[1]]:
                    if self.verbose:
                        print("OK: [{}]  ->  [{}]".format(i, o1))
                else:
                    print("FAIL: input: [{}]".format(i))
                    print("         HW: [{}]".format(o1))
                    print("        EMU: [{}]".format(o2))
                    failed += 1
            elif self.verbose:
                print("OK: {}  ->  {}".format(i, o1))

            if not self.verbose and count % 10 == 0:
                print("Processed: {}\r".format(count), end='', flush=True)

        for s in self.systems:
            s.write_byte(2)

        return (failed, count)

# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------

parser = argparse.ArgumentParser()
parser.add_argument("-l", "--limit", help="limit test runs", type=int, default=None)
parser.add_argument("-v", "--verbose", help="verbose test output", action="store_const", const=1, default=0)
parser.add_argument('test', nargs='*', help='test(s) to run')

args = parser.parse_args()

terminate = False

def signal_handling(signum, frame):
    global terminate
    if terminate:
        sys.exit(1)
    terminate = True

signal.signal(signal.SIGINT, signal_handling)

s = serial.Serial("/dev/ttyUSB0",
    baudrate = 9600,
    bytesize = serial.EIGHTBITS,
    parity = serial.PARITY_NONE,
    stopbits = serial.STOPBITS_ONE,
    timeout = None,
    xonxoff = False,
    rtscts = False,
    dsrdtr = False)

tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp.connect(("127.0.0.1", 32000))
tcp.setblocking(True)

kz_hw = KZ(s, echo_cancel=True, debug=False)
kz_emu = KZ(tcp, echo_cancel=False, debug=False)

c = ComparativeTest(kz_hw, kz_emu, limit=args.limit, verbose=args.verbose)
tests_failed = 0
tests_count = 0
for test in args.test:
    c.load(test)
    failed, count = c.run()
    tests_count += 1
    if failed:
        tests_failed += 1
        print("FAILED test vectors: {} of {}".format(failed, count))
if tests_failed:
    print("FAILED tests: {} of {}".format(tests_failed, tests_count))
else:
    print("All tests OK    ")
