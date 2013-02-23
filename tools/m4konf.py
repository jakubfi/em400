#!/usr/bin/python
# -*- coding: UTF-8 -*-
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


import sys
from m400lib import *
from m400_utils import *

NIETAK = ["Nie", "Tak"]

# ------------------------------------------------------------------------
class WordConf:

    # --------------------------------------------------------------------
    def __init__(self, word, disableable = False):
        self.word = word
        self.disableable = disableable
        self.keys = []
        self.conf = {}
        self.pos = 0

    # --------------------------------------------------------------------
    def __iter__(self):
        self.pos = 0
        return self

    # --------------------------------------------------------------------
    def add(self, key, name, bit_start, bit_end, options = []):

        bitlen = bit_end - bit_start + 1

        if self.conf.has_key(key):
            raise KeyError("Key '%s' alredy defined" % key)
        if bit_end < bit_start:
            raise ValueError("Starting bit > ending bit")
        if len(options) != 0 and len(options) != 2**(bitlen):
            raise ValueError("Option list size != field capacity")

        mask = bitmask(bitlen)
        shift = 15 - bit_end
        value = (self.word >> shift) & mask

        self.keys.append(key)
        self.conf[key] = [name, value, mask, shift, options]

    # --------------------------------------------------------------------
    def set(self, key, value):
        if value > self.conf[key][2]:
            raise ValueError("Value %i won't fit in '%s' bit length" % (value, key))
        self.conf[key][1] = value

    # --------------------------------------------------------------------
    def get_word(self):
        word = 0
        for k in self.keys:
            word |= (self.conf[k][1] & self.conf[k][2]) << self.conf[k][3]
        return word

    # --------------------------------------------------------------------
    def is_disabled(self):
        if self.disableable == True and self.word == 65535:
            return True
        return False

    # --------------------------------------------------------------------
    def get(self, key):

        if self.is_disabled():
            raise ValueError("Device disabled")

        entry = self.conf[key]
        name = entry[0]
        value = entry[1]
        try:
            value_name = entry[4][entry[1]]
        except:
            value_name = "%s" % value

        return key, name, value, value_name

    # --------------------------------------------------------------------
    def get_val(self, key):
        return self.conf[key][1]

    # --------------------------------------------------------------------
    def get_by_id(self, key_id):
        return self.get(self.keys[key_id])

    # --------------------------------------------------------------------
    def next(self):
        if (self.pos > len(self.conf)-1):
            raise StopIteration

        self.pos += 1

        return self.get_by_id(self.pos-1)

 
# ------------------------------------------------------------------------
class M400Conf:

    # --------------------------------------------------------------------
    def __init__(self, image, offset):
        self.image = image
        self.offset = offset
        self.data = wload(self.image, self.offset, 64)

        self.magic_w0 = 0b1111000000000000 # UJ
        self.magic_w2 = 0b0000000000000000 # 0
        self.magic_w3 = 0b0001011000000000 # 0x1600

        if self.data[0] != self.magic_w0 or self.data[2] != self.magic_w2 or self.data[3] != self.magic_w3:
            raise ValueError("Image '%s' doesn't contain system at offset %i" % (self.image, self.offset))

        self.config = {}

        self.config["sys1"] = self.parse_sys1(self.data[04])
        self.config["multix"] = self.parse_multix(self.data[05])
        self.config["sys2"] = self.parse_sys2(self.data[06])
        self.config["sys3"] = self.parse_sys3(self.data[07])

        for i in range(0, 8):
           self.config["mem%i"%i] = self.parse_mem(self.data[010+i])

        for i in range(0, 16):
            self.config["disk%i"%i] = self.parse_disk(self.data[020+i])

        for i in range(0, 4):
            self.config["tape%i"%i] = self.parse_tapes(self.data[040+i])

        self.config["io1"] = self.parse_io1(self.data[044])
        self.config["io2"] = self.parse_io2(self.data[045])
        self.config["io3"] = self.parse_io3(self.data[046])
        self.config["io4"] = self.parse_io4(self.data[047])
        self.config["io5"] = self.parse_io5(self.data[050])
        self.config["io6"] = self.parse_io6(self.data[051])
        self.config["io7"] = self.parse_io7(self.data[052])
        self.config["io8"] = self.parse_io8(self.data[053])
        self.config["rtc"] = self.parse_rtc(self.data[056])
        self.config["mon"] = self.parse_mon(self.data[057])
        self.config["oprq"] = self.parse_oprq(self.data[060])

        for i in range(0, 15):
            self.config["char%i"%i] = self.parse_char(self.data[061+i])

    # --------------------------------------------------------------------
    def sections(self):
        return self.config.keys()

    # --------------------------------------------------------------------
    def parse_sys1(self, word):
        sys1 = WordConf(word)
        sys1.add("exlsem",  "Ekstrakody semaforowe", 0, 0, NIETAK)
        sys1.add("dw3",     "Obsługa DW3", 1, 1, NIETAK)
        sys1.add("lod",     "Ekstrakody LOD i UNL", 2, 2, NIETAK)
        sys1.add("timer",   "Szybki zegar", 3, 3, NIETAK)
        sys1.add("noswap",  "Bez trybu z wymianami", 4, 4, NIETAK)
        sys1.add("4kbuf",   "Używaj byforów 4k", 5, 5, NIETAK)
        sys1.add("res",     "Programy jako rezydujące", 6, 6, NIETAK)
        sys1.add("automx",  "Automatyczna obsługa MULTIX-a", 7, 7, NIETAK)
        sys1.add("sysbuf",  "Liczba buf. systemowych", 8, 15)
        return sys1

    # --------------------------------------------------------------------
    def parse_multix(self, word):
        mulx = WordConf(word)
        mulx.add("err",     "Obsługa błędów", 0, 0, NIETAK)
        mulx.add("nopar",   "Bez parzystości", 1, 1, NIETAK)
        mulx.add("par",     "Z nieparzystością", 2, 2, NIETAK)
        mulx.add("8bit",    "8 bitów", 3, 3, NIETAK)
        mulx.add("xonxoff", "XON/XOFF", 4, 4, NIETAK)
        mulx.add("bscan",   "Obsługuj BS i CAN", 5, 5, NIETAK)
        mulx.add("upper",   "Litery małe na duże", 6, 6, NIETAK)
        mulx.add("dumper",  "Dołącz dumper", 7, 7, NIETAK)
        mulx.add("strvec",  "Liczba tablic strumieni", 8, 15)
        return mulx

    # --------------------------------------------------------------------
    def parse_sys2(self, word):
        sys2 = WordConf(word)
        sys2.add("autoram", "Zbiory robocze w RAM", 0, 0, NIETAK)
        sys2.add("uservec", "Liczba tablic skorowidzów", 8, 15)
        return sys2
 
    # --------------------------------------------------------------------
    def parse_sys3(self, word):
        sys3 = WordConf(word)
        sys3.add("sysram",  "Bloki dla OS", 0, 3)
        sys3.add("buflen",  "Długość buforów końcówek", 8, 15)
        return sys3

    # --------------------------------------------------------------------
    def parse_mem(self, word):
        mem = WordConf(word)
        mem.add("silicon",  "Typ pamięci", 0, 0, ["Ferrytowa", "Półprzewodnikowa"])
        mem.add("mega",     "Rozmiar modułów", 1, 1, ["32k", "MEGA 64k"])
        mem.add("blocks",   "Liczba bloków", 2, 7)
        mem.add("blstart",  "Blok poczatkowy", 8, 11)
        mem.add("mdstart",  "Moduł poczatkowy", 12, 15)
        return mem
 
    # --------------------------------------------------------------------
    def parse_disk(self, word):
        disk = WordConf(word, disableable = True)
        disk.add("foreign", "Talerz", 0, 0, ["Własny", "Obcy"])
        disk.add("dtype",   "Typ", 1, 2, ['MERA 9425 w kanale pamięciowym', 'Winchester', 'Floppy', '9425 lub EC 5061 w PLIX-ie'])

        disk_type = disk.get_val('dtype')

        # MERA 9425 w kanale pamięciowym
        if disk_type == 0:
            disk.add("unit",    "Jednostka St.", 7, 9)
            disk.add("chan",    "Kanał", 10, 13)
            disk.add("fixed",   "Talerz", 15, 15, ["Wymienny", "Stały"])

        # Floppy
        elif disk_type == 2:
            disk.add("inch", "Rozmiar flopa", 3, 3, ['8"', '5.25"'])
            inch = disk.get_val('inch')

            if inch == 0:
                disk.add("number",  "Numer jednostki", 8, 12)
                disk.add("door",    "Drzwiczki stacji", 13, 15)

            else:
                disk.add("density", "Gęstość", 10, 12, ["SD", "DD", "HD", "--", "--", "--", "--", "--"])
                disk.add("number",  "Numer stacji", 13, 15)

        # Winchester
        elif disk_type == 1:
            disk.add("quant",   "Kwant startowy", 4, 9)
            disk.add("type",    "Typ Winchestera", 10, 12)
            disk.add("number",  "Numer Winchestera", 14, 15)

        # MERA 9425 lub EC 5061 w PLIX-ie
        elif disk_type == 3:
            disk.add("type", "Typ", 6, 7, ['EC-5061', 'MERA 9425 (talerz wymienny)', 'MERA 9425 (talerz stały)', 'MERA 9425 (cały dysk)'])
            disk.add("plix", "Numer pakiety PLIX-a", 8, 12)
            disk.add("number", "Numer urządzenia", 13, 15)

        return disk

    # --------------------------------------------------------------------
    def parse_tapes(self, word):
        tape = WordConf(word, disableable = True)
        tape.add("unit", "Numer jednostki ster.", 8, 10)
        tape.add("chan", "Numer kanał", 11, 14)

        return tape

    # --------------------------------------------------------------------
    def parse_io1(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("camac1", "Adres CAMAC 1", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io2(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("camac2", "Adres CAMAC 2", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io3(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("camac3", "Adres CAMAC 3/PI", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io4(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("camac4", "Adres CAMAC 3/IEC", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io5(self, word):
        wc = WordConf(word)
        wc.add("winch", "Linia sterownika Winchester", 2, 7)
        wc.add("plix", "Kanał PLIX", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io6(self, word):
        wc = WordConf(word)
        wc.add("winch", "Linia sterownika Floppy", 2, 7)
        wc.add("plix", "Kanał MULTIX", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io7(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("char1", "Kanał znakowy 1", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_io8(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("char2", "Kanał znakowy 2", 11, 14)
        return wc

    # --------------------------------------------------------------------
    def parse_rtc(self, word):
        wc = WordConf(word, disableable = True)
        wc.add("unit", "Urządzenie", 8, 10)
        wc.add("chan", "Kanał", 8, 10)
        return wc

    # --------------------------------------------------------------------
    def parse_mon(self, word):
        wc = WordConf(word)
        wc.add("mon", "Numer systemowy monitorów", 0, 15)
        return wc

    # --------------------------------------------------------------------
    def parse_oprq(self, word):
        wc = WordConf(word)
        wc.add("oprq", "Numer systemowy końcówki dla OPRQ", 0, 15)
        return wc

    # --------------------------------------------------------------------
    def parse_char(self, word, disableable = True):
        wc = WordConf(word, disableable = True)
        # MULTIX only
        if word >> 8 != 0:
            wc.add("dir", "Kierunek", 0, 2, ["--", "--", "Wejście", "--", "Wyjście", "", "Half-Duplex", "Full-Duplex"])
            wc.add("used", "Linia użyta", 3, 3, NIETAK)
            wc.add("type", "Typ linii", 4, 7, ["szeregowa", "równoległa", "synchroniczna"]+["--"]*13)
        wc.add("proto", "Protokół", 8, 10, ["czytnik taśmy", "drukarka, perforator", "monitor"]+["--"]*5)
        wc.add("count", "Liczba urządzeń", 11, 15)
        return wc


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

if len(sys.argv) == 3:
    image = sys.argv[1]
    offset = int(sys.argv[2])
elif len(sys.argv) == 2:
    image = sys.argv[1]
    offset = 0
else:
    print "Usage: m4konf.py <image> [offset]"
    sys.exit(1)

try:
    m4c = M400Conf(image, offset)
except Exception, e:
    print "Cannot load system configuration: %s" % str(e)
    sys.exit(1)

while True:
    try:
        command = raw_input("KONF> ").split()
        cmd = command[0]
        args = command[1:]
    except EOFError:
        break
    except KeyboardInterrupt:
        break
    except:
        continue

    if cmd == "quit" or cmd == "exit":
        break

    elif cmd == "sections":
        print sorted(m4c.sections())

    elif cmd == "print":
        try:
            section = args[0]
        except:
            print "  Use: print <section>|all"
            continue

        if section == 'all':
            sections = sorted([ x for x in m4c.config ])
        else:
            sections = [ section ]

        for s in sections:
            print "[%s]" % s
            try:
                for name, desc, val, dval in m4c.config[s]:
                    print "  %-7s = %-3i   # %s = %s" % (name, val, desc, dval)
            except ValueError:
                print "  Disabled"
                continue
            except:
                print "  No section: %s" % s

    elif cmd == "set":
        try:
            section = args[0]
            key = args[1]
            value = int(args[2])
        except:
            print "  Use: set <section> <key> <value>"
            continue
        try:
            m4c.config[section].set(key, value)
        except Exception, e:
            print "  Cannot set %s/%s = %i. Error: %s" % (section, key, value, str(e))

    elif cmd == "write":
        print "  Not implemented"

    else:
        print "  Unknown command"


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
