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

import pygame, sys, os
from pygame.locals import *
from pygame.gfxdraw import *
from crc_algorithms import Crc

# -----------------------------------------------------------------------
class mfm_track:

    # -------------------------------------------------------------------
    def __init__(self, fname):
        print "Loading track image: %s..." % fname
        f = open(fname, "r")
        self.data = bytearray(f.read())
        f.close()

        self.samples = []
        self.gaps = []
        self.gap_hist = {}
        self.clock = []
        self.a1 = []

        self.a1_mark = [0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1]
        self.a1_buf = []
        self.a1_clk = []
        self.a1_pos = 0

        self.crc = Crc(width = 16, poly = 0x1021, reflect_in = False, xor_in = 0xffff, reflect_out = False, xor_out = 0x0000);
        #self.crc = Crc(width = 16, poly = 0x140a0445, reflect_in = False, xor_in = 0xffffffff, reflect_out = False, xor_out = 0x0000);

        print "Unpacking..."
        self.explode()

    # -------------------------------------------------------------------
    def explode(self):
        for byte in self.data:
            for bit in [7, 6, 5, 4, 3, 2, 1, 0]:
                v = (byte >> bit) & 1
                # [value, clock, a1, cell, validdbit, bit, validbyte, byte, crcok]
                self.samples.append([v, 0, 0, 0, 0, 0, 0, 0, 0])

    # -------------------------------------------------------------------
    def clock_regen(self, clock_period, early_clock_margin, clock_offset=0):
        counter = 0
        ov = 1
        next_clock = 0

        while counter < len(self.samples):
            v = self.samples[counter][0]
            self.samples[counter] = [v, 0, 0, 0, 0, 0, 0, 0, 0]

            # each rising edge restarts clock
            if (ov == 0) and (v == 1):
                # clock cleanup - remove previously inserted early clock ticks
                if early_clock_margin and not clock_offset:
                    prev_clock_tick = self.clock[len(self.clock)-1]
                    if counter - prev_clock_tick <= early_clock_margin:
                        self.clock.pop()
                        self.samples[prev_clock_tick][1] = 0

                if not clock_offset:
                    next_clock = counter + clock_period
                    self.clock.append(counter)
                    self.samples[counter][1] = 1
                else:
                    next_clock = counter + clock_offset

            # is it time for next clock tick?
            if counter >= next_clock:
                self.samples[counter][1] = 1
                self.clock.append(counter)
                next_clock = counter + clock_period

            ov = v
            counter += 1

    # -------------------------------------------------------------------
    def a1_match(self, clock):
        bit = self.samples[clock][0]
        self.a1_buf.append(bit)
        self.a1_clk.append(clock)

        if len(self.a1_buf) == 16:
            if self.a1_buf == self.a1_mark:
                clk = self.a1_clk[0]
                self.a1_buf = []
                self.a1_clk = []
                return clk
            else:
                self.a1_buf.pop(0)
                self.a1_clk.pop(0)

        return 0

    # -------------------------------------------------------------------
    def a1_search(self):
        clk = 0
        while clk < len(self.clock):
            a1beg = self.a1_match(self.clock[clk])
            if a1beg:
                self.a1.append(clk)
                self.samples[a1beg][2] = 1
                self.samples[self.clock[clk+1]][2] = 2
            clk += 1

    # -------------------------------------------------------------------
    def calc_gaps(self):
        ls = 0
        pos = opos = 0
        for (s,c,a1,cell,valbit,bit,valbyte,byte,crcok) in self.samples:
            if (s != ls) and (s == 1):
                diff = pos-opos
                self.gaps.append(diff)
                if (self.gap_hist.has_key(diff)):
                    self.gap_hist[diff] += 1
                else:
                    self.gap_hist[diff] = 1
                opos = pos
            pos += 1
            ls = s

    # -------------------------------------------------------------------
    def read_bytes(self, clkpos, b):
        bit = 7
        char = 0
        data = [ 0xa1 ]
        cellmark = 1
        crcok = False
        crc = 0
        while b > 0:
            # prepare 0, +1 and +2 clocks
            clk0 = self.clock[clkpos]
            clk1 = self.clock[clkpos+1]
            clk2 = self.clock[clkpos+2]

            # mark cell start
            self.samples[clk0][3] = cellmark
            cellmark *= -1

            # mark bit end
            self.samples[clk2][4] = 1
            self.samples[clk2][5] = self.samples[clk1][0]

            # shift bit value into the byte
            char |= self.samples[clk1][0] << bit
            bit -= 1

            if bit < 0:
                # append only data, not CRC
                if b > 2:
                    data.append(char)
                elif b == 2:
                    crc = self.crc.table_driven(''.join([chr(x) for x in data]))
                    print "%x" % crc
                    if char == (crc & 0xff00) >> 8:
                        self.samples[clk2][8] = 1
                        crcok = True
                    else:
                        self.samples[clk2][8] = 2
                        crcok = False
                elif b == 1:
                    if char == crc & 0xff:
                        self.samples[clk2][8] = 1
                        crcok &= True
                    else:
                        self.samples[clk2][8] = 2
                        crcok = False

                # mark and store byte
                self.samples[clk2][6] = 1
                self.samples[clk2][7] = char

                bit = 7
                b -= 1
                char = 0

            clkpos += 2
        self.samples[clk2][3] = 2
        return data, crcok

    # -------------------------------------------------------------------
    def analyze(self, clock, margin, offset):

        self.gaps = []
        self.gap_hist = {}
        self.clock = []
        self.a1 = []

        print "Regenerating clock..."
        self.clock_regen(clock, margin, offset)

        print "Analyzing signal gaps..."
        self.calc_gaps()

        print "Looking for sector header/data marks..."
        self.a1_search()
        print "%i A1 marks found." % len(self.a1)

        print "Analyzing sectors..."
        count = 0
        while count < len(self.a1):
            try:
                data, crcok = self.read_bytes(self.a1[count]+1, 6)
                print ''.join([chr(x) for x in data])
                print "---- CRC: %s --------------------------------------------------------" % str(crcok)
            except Exception, e:
                print str(e)
                pass
            count += 1
            try:
                data, crcok = self.read_bytes(self.a1[count]+1, 512 + 5)
                print ''.join([chr(x) for x in data])
                print "---- CRC: %s --------------------------------------------------------" % str(crcok)
            except Exception, e:
                print str(e)
                pass
            count += 1

# ------------------------------------------------------------------------
class WDA:

    # -------------------------------------------------------------------
    def __init__(self, fname):
        self.fname = fname
        self.quit = 0
        self.win_w = 1200
        self.win_h = 297

        self.clk = 12
        self.clk_margin = 2
        self.clk_offset = 0
        self.track = mfm_track(self.fname)
        self.track.analyze(self.clk, self.clk_margin, self.clk_offset)

        pygame.init()
        pygame.font.init()
        window = pygame.display.set_mode((self.win_w, self.win_h))
        pygame.display.set_caption('WDA')
        self.screen = pygame.display.get_surface()
        pygame.mouse.set_visible(1)
        self.f = {}
        self.f[12] = pygame.font.Font(pygame.font.match_font("tahoma"), 12)
        self.f[10] = pygame.font.Font(pygame.font.match_font("tahoma"), 10)

    # -------------------------------------------------------------------
    def write(self, text, pos, color, size=12, bold=False, bg=(0,0,0)):
        self.f[size].set_bold(bold)
        s = self.f[size].render(text, True, color, bg)
        self.screen.blit(s, pos)
        
    # -------------------------------------------------------------------
    def draw_info(self, x, y, w, h):
        pygame.draw.rect(self.screen, (0,0,0), (x, x, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, x, w, h), 1)

        self.write("File:", (x+5, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write(self.fname, (x+40, y+3), (0xFF, 0xFE, 0xE0), 12, True)

        self.write("Clock period/margin/offset:", (x+300, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write("%i/%i/%i" % (self.clk, self.clk_margin, self.clk_offset), (x+480, y+3), (0xFF, 0xFE, 0xE0), 12, True)

        self.write("A1s:", (x+545, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write("%i" % (len(self.track.a1)), (x+580, y+3), (0xFF, 0xFE, 0xE0), 12, True)

    # -------------------------------------------------------------------
    def draw_hist(self, x, y, w, h):
        pygame.draw.rect(self.screen, (0,0,0), (x, y, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, y, w, h), 1)

        xpos = 5

        for p in self.track.gap_hist.keys():
            self.write("%i" % p, (xpos, y+h-15), (255,255,255), 10, False)
            v = self.track.gap_hist[p]/150
            if v > h-20:
                v = h-20
            pygame.draw.rect(self.screen, (0xFF, 0x47, 0x75), (xpos, y+h-15-v, 8, v))
            xpos += 15

    # -------------------------------------------------------------------
    def draw_wave(self, offset, x, y, w, h):
        if not self.track:
            return

        pygame.draw.rect(self.screen, (0,0,0), (x, y, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, y, w, h), 1)

        ox = 0
        oy = 0
        sx = 0
        a1 = False
        cell = 0

        while sx < w-x-10:
            pos = offset + sx

            # draw a1 marks
            if self.track.samples[pos][2] == 1:
                a1 = True
            elif self.track.samples[pos][2] == 2:
                a1 = False
            if a1:
                line(self.screen, x+sx+5, y+23, x+sx+5, y+62, (0xA6, 0x1B, 0x9A))

            # draw bit cells
            if self.track.samples[pos][3] == 1:
                cell = 1
            if self.track.samples[pos][3] == -1:
                cell = 2
            if self.track.samples[pos][3] == 2:
                cell = 0
            if cell:
                cc = 50 * (cell%2) + 0x50
                line(self.screen, x+sx+5, y+23, x+sx+5, y+62, (cc,cc,cc))

            # draw clock ticks
            if self.track.samples[pos][1]:
                if ((pos/10)%10) == 0:
                    line(self.screen, x+sx+5, y+11, x+sx+5, y+21, (0x29, 0xFF, 0xEA))
                    self.write("%i" % (pos/10), (x+sx+5, y+5), (255,255,255), 10, False)
                line(self.screen, x+sx+5, y+58, x+sx+5, y+61, (0x29, 0xFF, 0xEA))
                self.write("%i" % self.track.samples[pos][0], (x+sx+3, y+62), (255,255,255), 10, False)

            # draw bit values
            if self.track.samples[pos][4]:
                line(self.screen, x+sx+5, y+74, x+sx+5, y+91, (0xFF, 0xf5, 0x70))
                self.write("%i" % self.track.samples[pos][5], (x+sx-6, y+77), (255,255,255), 12, False)

            # draw byte values
            if self.track.samples[pos][6]:
                color = (0xFF, 0xf5, 0x70)
                if self.track.samples[pos][8] == 1:
                    color = (0x00, 0xff, 0x00)
                if self.track.samples[pos][8] == 2:
                    color = (0xff, 0x00, 0x00)
                line(self.screen, x+sx+5, y+74, x+sx+5, y+111, color)
                if self.track.samples[pos][7] != 0:
                    ch = chr(self.track.samples[pos][7])
                else:
                    ch = " "
                self.write("0x%02x %s" % (self.track.samples[pos][7], ch), (x+sx-46, y+97), (255,255,255), 12, True)

            # draw waveform
            sy = 30 + self.track.samples[pos][0]*-30
            if (ox!=0) or (oy!=0):
                line(self.screen, x+ox+5, y+oy+26, x+sx+5, y+sy+26, (255,255,255))

            ox = sx+1
            oy = sy

            sx += 1

    # -------------------------------------------------------------------
    def draw_controls(self, x, y, w, h):
        pygame.draw.rect(self.screen, (0,0,0), (x, y, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, y, w, h), 1)

        pygame.draw.rect(self.screen, (0x70, 0x9D, 0xFF), (x+2, y+2, 50, h-4))
        pygame.draw.rect(self.screen, (0x70, 0x9D, 0xFF), (x+54, y+2, 50, h-4))
        self.write("CLK-", (x+12, y+8), (0,0,0), 12, True, (0x70, 0x9D, 0xFF))
        self.write("CLK+", (x+64, y+8), (0,0,0), 12, True, (0x70, 0x9D, 0xFF))

        pygame.draw.rect(self.screen, (0x8d, 0x7D, 0xFF), (x+106, y+2, 50, h-4))
        pygame.draw.rect(self.screen, (0x8d, 0x7D, 0xFF), (x+158, y+2, 50, h-4))
        self.write("MRG-", (x+116, y+8), (0,0,0), 12, True, (0x8d, 0x7D, 0xFF))
        self.write("MRG+", (x+168, y+8), (0,0,0), 12, True, (0x8d, 0x7D, 0xFF))

        pygame.draw.rect(self.screen, (0xba, 0x70, 0xFF), (x+210, y+2, 50, h-4))
        pygame.draw.rect(self.screen, (0xba, 0x70, 0xFF), (x+262, y+2, 50, h-4))
        self.write("OFS-", (x+220, y+8), (0,0,0), 12, True, (0xba, 0x70, 0xFF))
        self.write("OFS+", (x+272, y+8), (0,0,0), 12, True, (0xba, 0x70, 0xFF))

        pygame.draw.rect(self.screen, (0xE2, 0x70, 0xFF), (x+314, y+2, 100, h-4))
        self.write("UPDATE", (x+334, y+8), (0,0,0), 12, True, (0xE2, 0x70, 0xFF))

    # -------------------------------------------------------------------
    def draw_nav(self, offset, x, y, w, h):
        pygame.draw.rect(self.screen, (0,0,0), (x, y, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, y, w, h), 1)

        scale = len(self.track.samples) / (w-x-10)

        for a1 in self.track.a1:
            xpos = self.track.clock[a1] / scale
            line(self.screen, x+5+xpos, y+1, x+5+xpos, y+h-2, (0xBB, 0x59, 0xD4))
        line(self.screen, x+5+offset/scale, y+1, x+5+offset/scale, y+h-2, (0xFF, 0xf5, 0x70))

    # -------------------------------------------------------------------
    def run(self):
        drag = 0
        offset = 0

        self.draw_info(1, 1, self.win_w-2, 20)
        self.draw_hist(1, 22, self.win_w-2, 100)
        self.draw_wave(offset, 1, 123, self.win_w-2, 120)
        self.draw_nav(offset, 1, 244, self.win_w-2, 20)
        self.draw_controls(1, 265, self.win_w-2, 30)
        while not self.quit:
            ev = pygame.event.wait()
            # mouse down
            if ev.type == 5:
                # wave drag
                if ev.pos[1] > 123 and ev.pos[1] < 243:
                    drag = 1
                if ev.pos[1] > 244 and ev.pos[1] < 264:
                    offset = (ev.pos[0]-6) * (len(self.track.samples) / (self.win_w-2-1-10))
                    self.draw_wave(offset, 1, 123, self.win_w-2, 120)
                    self.draw_nav(offset, 1, 244, self.win_w-2, 20)
                # controls
                if ev.pos[1] > 265 and ev.pos[1] < 295 and ev.pos[0] < 414:
                    if ev.pos[0] > 2 and ev.pos[0] < 50:
                        self.clk -= 1
                    if ev.pos[0] > 54 and ev.pos[0] < 104:
                        self.clk += 1
                    if ev.pos[0] > 106 and ev.pos[0] < 156:
                        self.clk_margin -= 1
                    if ev.pos[0] > 158 and ev.pos[0] < 208:
                        self.clk_margin += 1
                    if ev.pos[0] > 210 and ev.pos[0] < 260:
                        self.clk_offset -= 1
                    if ev.pos[0] > 262 and ev.pos[0] < 312:
                        self.clk_offset += 1
                    if ev.pos[0] > 314 and ev.pos[0] < 412:
                        self.track.analyze(self.clk, self.clk_margin, self.clk_offset)
                    self.draw_info(1, 1, self.win_w-2, 20)
                    self.draw_wave(offset, 1, 123, self.win_w-2, 120)
                    self.draw_nav(offset, 1, 244, self.win_w-2, 20)
            # mouse up
            if ev.type == 6:
                drag = 0
            # mouse pos
            if ev.type == 4 and drag:
                offset -= ev.rel[0]
                if offset < 0:
                    offset = 0
                self.draw_wave(offset, 1, 123, self.win_w-2, 120)
                self.draw_nav(offset, 1, 244, self.win_w-2, 20)
            # key down
            if ev.type == 2:
                if ev.key == 292:
                    self.quit = 1
                if ev.key == 275:
                    for a1 in self.track.a1:
                        if self.track.clock[a1] > offset:
                            offset = self.track.clock[a1]
                            break;
                if ev.key == 276:
                    apos = len(self.track.a1)-1
                    while apos>0 and self.track.clock[self.track.a1[apos]] >= offset:
                        apos -= 1
                    if apos >= 0:
                        offset = self.track.clock[self.track.a1[apos]]
                if ev.key == 278:
                    offset = 0
                if ev.key == 279:
                    offset = len(self.track.samples) - self.win_w

                self.draw_wave(offset, 1, 123, self.win_w-2, 120)
                self.draw_nav(offset, 1, 244, self.win_w-2, 20)

            #print ev
            pygame.display.flip()


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

filename = ""

try:
    filename = sys.argv[1]
except Exception, e:
    print "Usage: wda.py track_image"
    sys.exit(1)

try:
    wda = WDA(filename)
    wda.run()
except Exception, e:
    print "Cannot perform the analysis: %s" % str(e)


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
