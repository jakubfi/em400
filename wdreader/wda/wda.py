#!/usr/bin/python
# -*- coding: UTF-8 -*-

import pygame, sys, os
from pygame.locals import *
from pygame.gfxdraw import *

# -----------------------------------------------------------------------
class wds_track:

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

        print "Unpacking..."
        self.explode()

    # -------------------------------------------------------------------
    def explode(self):
        for byte in self.data:
            for bit in [7, 6, 5, 4, 3, 2, 1, 0]:
                v = (byte >> bit) & 1
                # [value, clock, a1, cell, validdbit, bit, validbyte, byte]
                self.samples.append([v, 0, 0, 0, 0, 0, 0, 0])

    # -------------------------------------------------------------------
    def clock_regen(self, clock_period, early_clock_margin):
        counter = 0
        ov = 1
        next_clock = 0

        while counter < len(self.samples):
            v = self.samples[counter][0]

            # each rising edge restarts clock
            if (ov == 0) and (v == 1):
                # clock cleanup - remove previously inserted early clock ticks 
                prev_clock_tick = self.clock[len(self.clock)-1]
                if counter - prev_clock_tick <= early_clock_margin:
                    self.clock.pop()
                    self.samples[prev_clock_tick][1] = 0

                next_clock = counter + clock_period
                self.clock.append(counter)
                self.samples[counter][1] = 1

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
        for (s,c,a1,cell,valbit,bit,valbyte,byte) in self.samples:
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
        data = []
        cellmark = 1
        while b > 0:
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
                bit = 7
                data.append(char)
                # mark and store byte
                self.samples[clk2][6] = 1
                self.samples[clk2][7] = char
                b -= 1
                char = 0

            clkpos += 2
        self.samples[clk2][3] = 2
        return data

    # -------------------------------------------------------------------
    def analyze(self, clock, margin):
        print "Regenerating clock..."
        self.clock_regen(clock, margin)

        print "Analyzing signal gaps..."
        self.calc_gaps()

        print "Looking for sector header/data marks..."
        self.a1_search()
        print "%i A1 marks found." % len(self.a1)

        print "Analyzing sectors..."
        count = 0
        while count < len(self.a1):
            data = self.read_bytes(self.a1[count]+1, 6)
            print ''.join(map(chr,data))
            print "---------"
            count += 1
            data = self.read_bytes(self.a1[count]+1, 512 + 3)
            print ''.join(map(chr,data))
            print "---------"
            count += 1

# ------------------------------------------------------------------------
class WDA:

    # -------------------------------------------------------------------
    def __init__(self, fname):
        self.fname = fname
        self.quit = 0
        self.win_w = 1200
        self.win_h = 400

        self.clk = 12
        self.clk_margin = 2
        self.track = wds_track(self.fname)
        self.track.analyze(self.clk, self.clk_margin)

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
    def write(self, text, pos, color, size=12, bold=False):
        self.f[size].set_bold(bold)
        s = self.f[size].render(text, True, color)
        self.screen.blit(s, pos)
        
    # -------------------------------------------------------------------
    def draw_info(self, x, y, w, h):
        pygame.draw.rect(self.screen, (0,0,0), (x, x, w, h))
        pygame.draw.rect(self.screen, (255,255,255), (x, x, w, h), 1)

        self.write("File:", (x+5, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write(self.fname, (x+40, y+3), (0xFF, 0xFE, 0xE0), 12, True)

        self.write("Clock period/margin:", (x+300, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write("%i/%i" % (self.clk, self.clk_margin), (x+435, y+3), (0xFF, 0xFE, 0xE0), 12, True)

        self.write("A1s:", (x+500, y+3), (0xff, 0xff, 0xff), 12, True)
        self.write("%i" % (len(self.track.a1)), (x+535, y+3), (0xFF, 0xFE, 0xE0), 12, True)

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
                line(self.screen, x+sx+5, y+3, x+sx+5, y+42, (0xA6, 0x1B, 0x9A))

            # draw bit cells
            if self.track.samples[pos][3] == 1:
                cell = 1
            if self.track.samples[pos][3] == -1:
                cell = 2
            if self.track.samples[pos][3] == 2:
                cell = 0
            if cell:
                cc = 50 * (cell%2) + 0x50
                line(self.screen, x+sx+5, y+3, x+sx+5, y+42, (cc,cc,cc))

            # draw clock ticks
            if self.track.samples[pos][1]:
                line(self.screen, x+sx+5, y+38, x+sx+5, y+41, (0x29, 0xFF, 0xEA))
                self.write("%i" % self.track.samples[pos][0], (x+sx+3, y+42), (255,255,255), 10, False)

            # draw bit values
            if self.track.samples[pos][4]:
                line(self.screen, x+sx+5, y+54, x+sx+5, y+71, (0xFF, 0xf5, 0x70))
                self.write("%i" % self.track.samples[pos][5], (x+sx-6, y+57), (255,255,255), 12, False)

            # draw byte values
            if self.track.samples[pos][6]:
                line(self.screen, x+sx+5, y+54, x+sx+5, y+91, (0xFF, 0xf5, 0x70))
                if self.track.samples[pos][7] != 0:
                    ch = chr(self.track.samples[pos][7])
                else:
                    ch = " "
                self.write("0x%02x %s" % (self.track.samples[pos][7], ch), (x+sx-46, y+77), (255,255,255), 12, True)

            # draw waveform
            sy = 30 + self.track.samples[pos][0]*-30
            if (ox!=0) or (oy!=0):
                line(self.screen, x+ox+5, y+oy+6, x+sx+5, y+sy+6, (255,255,255))

            ox = sx+1
            oy = sy

            sx += 1

    # -------------------------------------------------------------------
    def run(self):
        drag = 0
        offset = 0

        self.draw_info(1, 1, self.win_w-2, 20)
        self.draw_hist(1, 22, self.win_w-2, 100)
        self.draw_wave(offset, 1, 123, self.win_w-2, 100)
        while not self.quit:
            ev = pygame.event.wait()
            # mouse down
            if ev.type == 5:
                drag = 1
            # mouse up
            if ev.type == 6:
                drag = 0
            # mouse pos
            if ev.type == 4 and drag:
                offset -= ev.rel[0]
                if offset < 0:
                    offset = 0
                self.draw_wave(offset, 1, 123, self.win_w-2, 100)
            # key down
            if ev.type == 2:
                if ev.key == 292:
                    self.quit = 1
            #print ev
            pygame.display.flip()


# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

wda = WDA("dump--1--000--3.wds")
wda.run()


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
