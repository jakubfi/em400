# ------------------------------------------------------------------------
class State:
    FAILED = 0
    COOKING = 1
    DONE = 2
    LOOP_END = 3

# ------------------------------------------------------------------------
class A1Finder:

    # -------------------------------------------------------------------
    def __init__(self, max_ticks):
        self.a1_mark = [0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1]
        self.a1_buf = []
        self.a1_clk = []
        self.max_ticks = max_ticks
        self.start_tick = 0

    # --------------------------------------------------------------------
    def feed(self, (t, v)):
        self.a1_buf.append(v)

        # search starts, store current clock
        if self.start_tick == 0:
            self.start_tick = t

        if (t - self.start_tick) > self.max_ticks:
            self.start_tick = 0
            return State.FAILED

        if len(self.a1_buf) == 16:
            if self.a1_buf == self.a1_mark:
                self.a1_buf = []
                self.start_tick = 0
                return State.DONE
            else:
                self.a1_buf.pop(0)

        return State.COOKING


# ------------------------------------------------------------------------
class Sector:

    # --------------------------------------------------------------------
    def __init__(self):
        self.a1f_short = A1Finder(8000)
        self.a1f_long = A1Finder(100000)

        self.phase = 0
        self.layout = [
            self.a1f_long.feed,
            self.a1f_short.feed,
            self.loop
        ]

    # --------------------------------------------------------------------
    def loop(self, s):
        return State.LOOP_END

    # --------------------------------------------------------------------
    def feed(self, s):
        result = self.layout[self.phase](s)

        if result == State.DONE:
            self.phase += 1
            return State.COOKING

        elif result == State.FAILED:
            self.phase = 0
            return State.FAILED

        elif result == State.LOOP_END:
            phase = 0
            return State.DONE

