#!/usr/bin/python

import pygame, sys, os
from pygame.locals import *
from pygame.gfxdraw import *

# -----------------------------------------------------------------------
class wda:

    # -------------------------------------------------------------------
    def __init__(self, fname):
        print "Loading track image: %s..." % fname
        f = open(fname, "r")
        self.data = bytearray(f.read())
        f.close()

        self.samples = []
        self.gaps = []
        self.clock = []
        self.a1 = []

        self.a1_mark = [0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1]
        self.a1_buf = []
        self.a1_pos = 0

        print "Unpacking, regenerating clock..."
        self.explode()
        print "Looking for sector header/data marks..."
        self.a1_search()
        print "%i A1 marks found." % len(self.a1)

    # -------------------------------------------------------------------
    def gfx_init(self):
        pygame.init()
        window = pygame.display.set_mode((1600, 200))
        pygame.display.set_caption('WDA')
        self.screen = pygame.display.get_surface()
        pygame.mouse.set_visible(1)

    # -------------------------------------------------------------------
    def explode(self):
        clock_period = 11 # 10MHz signal sampled at 100MHz = 10 samples period +2 for luck (just kidding, for clean output)
        ov = 1
        counter = 0
        next_clock = 0

        # unpack bytes into samples array
        for byte in self.data:
            for bit in [7, 6, 5, 4, 3, 2, 1, 0]:
                v = (byte & (1<<bit)) >> bit
                is_clock = 0

                # find rising edges
                if (ov == 0) and (v == 1):
                    is_clock = 1
                    # clock cleanup - remove inserted early clock ticks 
                    prev_clock_tick = self.clock[len(self.clock)-1]
                    if counter - prev_clock_tick < 3:
                        self.clock.pop()
                        self.samples[prev_clock_tick][1] = 0

                    next_clock = counter + clock_period
                    self.clock.append(counter)

                # is it time for next clock tick?
                if counter >= next_clock:
                    is_clock = 1
                    self.clock.append(counter)
                    next_clock = counter + clock_period

                self.samples.append([v, is_clock])

                ov = v
                counter += 1

    # -------------------------------------------------------------------
    def a1_match(self, b):
        self.a1_buf.append(b)

        if len(self.a1_buf) == 16:
            if self.a1_buf == self.a1_mark:
                self.a1_buf = []
                return True
            else:
                self.a1_buf.pop(0)

        return False

    # -------------------------------------------------------------------
    def a1_search(self):
        pa = 0
        for c in self.clock:
            ret = self.a1_match(self.samples[c][0])
            if ret:
                self.a1.append(c)
                pa = c

    # -------------------------------------------------------------------
    def calc_gaps(self):
        ls = 0
        pos = opos = 0
        for s,c in self.samples:
            if s != ls and s == 1:
                diff = pos-opos
                self.gaps.append(diff)
                opos = pos
            pos += 1
            ls = s

    # -------------------------------------------------------------------
    def hist(self):
        diffs = {}
        for g in self.gaps:
            if diffs.has_key(g):
                diffs[g] += 1
            else:
                diffs[g] = 1

        for i in diffs:
            print "%i -> %i" % (i, diffs[i])

    # -------------------------------------------------------------------
    def sector(self, s):
        data = 1
        c = 0
        bit = 7
        bcount = 0
        while bcount < 20:
            if self.samples[s][1]:
                data *= -1
                if data == 1:
                    c |= self.samples[s][0] << bit
                    bit -= 1

                if bit < 0:
                    bit = 7
                    print "%2x %s" % (c, chr(c))
                    bcount += 1
                    c = 0

            s += 1

    # -------------------------------------------------------------------
    def doit(self):
        self.sector(self.a1[0]+3)

    # -------------------------------------------------------------------
    def dump(self, offset, clock):
        self.screen.fill((0,0,0))
        i = 0
        ox = 0
        oy = 0
        while i < 1600:
            pos = offset + i
            if self.samples[pos][1]:
                line(self.screen, i, 0, i, 50, (0,255,0))
            x = i
            y = 40+self.samples[pos][0]*-30
            line(self.screen, ox, oy, x, y, (255,255,255))
            ox = x+1
            oy = y
            i += 1
        pygame.display.flip()

# ------------------------------------------------------------------------
# ---- MAIN --------------------------------------------------------------
# ------------------------------------------------------------------------

anal = wda("dump--1--000--3.wds")
#anal.calc_gaps()
#anal.hist()

anal.doit()

# ------------------------------------------------------------------------
drag = 0
offset = 0
anal.gfx_init()

while 1:
    anal.dump(offset, 18)
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


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

