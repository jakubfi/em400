import crc_algorithms

# ------------------------------------------------------------------------
class State:
    FAILED = 0
    COOKING = 1
    DONE = 2
    LOOP_END = 3

# ------------------------------------------------------------------------
class Looper:

    # -------------------------------------------------------------------
    def __init__(self):
        self.name = "Loop End"

    # -------------------------------------------------------------------
    def feed(self, s):
        return State.LOOP_END

# ------------------------------------------------------------------------
class Skipper:

    # -------------------------------------------------------------------
    def __init__(self, name, hbit_count):
        self.name = name
        self.hbit_count = hbit_count
        self.counter = 0

    # -------------------------------------------------------------------
    def feed(self, s):
        self.counter += 1

        if self.counter >= self.hbit_count:
            self.counter = 0
            return State.DONE
        else:
            return State.COOKING

# ------------------------------------------------------------------------
class BitSeqFinder:

    # -------------------------------------------------------------------
    def __init__(self, name, hbit_seq, deadline, callback):
        self.name = name
        self.hbit_seq = hbit_seq
        self.deadline = deadline
        self.callback = callback

        self.hbit_buf = []
        self.clock_tick = 0

    # --------------------------------------------------------------------
    def feed(self, (t, v)):
        self.hbit_buf.append(v)

        # past the deadline
        if self.clock_tick > (self.deadline + len(self.hbit_seq)):
            self.hbit_buf = []
            self.clock_tick = 0
            return State.FAILED

        # buffer full
        if len(self.hbit_buf) == len(self.hbit_seq):
            # does it match?
            if self.hbit_buf == self.hbit_seq:
                self.hbit_buf = []
                self.clock_tick = 0
                self.callback(self.hbit_buf)
                return State.DONE
            else:
                self.hbit_buf.pop(0)

        self.clock_tick += 1

        return State.COOKING

# ------------------------------------------------------------------------
class ByteReader:

    # --------------------------------------------------------------------
    def __init__(self, name, byte_count, callback):
        self.name = name
        self.byte_count = byte_count
        self.callback = callback
        self.bytes = [0]
        self.byte_pos = 0
        self.bit_pos = 7
        self.bit_odd = 1

    # --------------------------------------------------------------------
    def feed(self, (t,v)):

        # ignore odd bits (clock)
        if self.bit_odd:
            self.bit_odd = 0
            return State.COOKING
        self.bit_odd = 1

        # shift even bits (data)
        self.bytes[self.byte_pos] |= (v << self.bit_pos)

        self.bit_pos -= 1

        # byte is done
        if self.bit_pos < 0:
            self.byte_pos += 1
            if self.byte_pos == self.byte_count:
                self.callback(self.bytes)
                self.byte_pos = 0
                self.bit_pos = 7
                self.bytes = [0]
                self.bit_odd = 1
                return State.DONE
            else:
                self.bit_pos = 7
                self.bytes.append(0)
                return State.COOKING

# ------------------------------------------------------------------------
class Sector:

    A1 = [0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1]
    SYNC = [1, 0] * (10*8)

    # --------------------------------------------------------------------
    def __init__(self):
        self.crc_head_buf = []
        self.crc_data_buf = []
        self.crc16_alg = crc_algorithms.Crc(width = 16, poly = 0x1021, reflect_in = False, xor_in = 0xffff, reflect_out = False, xor_out = 0x0000);
        self.crc32_alg = crc_algorithms.Crc(width = 32, poly = 0x140a0445, reflect_in = False, xor_in = 0xffffffff, reflect_out = False, xor_out = 0x0000);
        self.cylinder = 0
        self.head = 0
        self.sector = 0
        self.sector_size = 0
        self.bad = False

        self.head_crc_ok = False
        self.data_crc_ok = False

        self.data = []

        self.phase = 0
        self.layout = [
            BitSeqFinder("Head SYNC", Sector.SYNC, 18*8*2, self.callback_none),
            BitSeqFinder("Head A1", Sector.A1, 3*8*2, self.callback_head_a1),
            ByteReader("Head data", 4, self.callback_head_data),
            ByteReader("Head CRC", 2, self.callback_head_crc),
            Skipper("Gap", 3*8*2),
            BitSeqFinder("Data Sync", Sector.SYNC, 3*8*2, self.callback_none),
            BitSeqFinder("Data A1", Sector.A1, 3*8*2, self.callback_data_a1),
            ByteReader("Data marker", 1, self.callback_data_marker),
            ByteReader("Data", 512, self.callback_data_data),
            ByteReader("Data CRC", 4, self.callback_data_crc),
            Skipper("Gap", 16*8*2),
            Looper()
        ]

    # --------------------------------------------------------------------
    def callback_head_a1(self, arg):
        self.crc_head_buf = [ 0xa1 ]
        
    # --------------------------------------------------------------------
    def callback_head_data(self, arg):
        self.crc_head_buf += arg

        if arg[0] == 0xfe:
            cyl_msb = 0
        elif arg[0] == 0xff:
            cyl_msb = 256
        elif arg[0] == 0xfc:
            cyl_msb = 512
        elif arg[0] == 0xfd:
            cyl_msb = 768
        self.cylinder = cyl_msb + arg[1]

        self.head = arg[2] & 0b00000111
        self.sector_size = arg[2] & 0b01100000
        if arg[2] & 0b10000000:
            self.bad = True
        self.sector = arg[3]

    # --------------------------------------------------------------------
    def callback_head_crc(self, arg):
        crc_read = arg[0]*256 + arg[1]
        crc_computed = self.crc16_alg.table_driven(''.join(map(chr,self.crc_head_buf)))
        if crc_read == crc_computed:
            self.head_crc_ok = True

    # --------------------------------------------------------------------
    def callback_data_a1(self, arg):
        self.crc_data_buf = [ 0xa1 ]
        
    # --------------------------------------------------------------------
    def callback_data_marker(self, arg):
        self.crc_data_buf += arg

    # --------------------------------------------------------------------
    def callback_data_data(self, arg):
        self.crc_data_buf += arg
        self.data = arg
        
    # --------------------------------------------------------------------
    def callback_data_crc(self, arg):
        crc_read = arg[0]*16777216 + arg[1]*65536 + arg[2]*256 + arg[3]
        crc_computed = self.crc32_alg.table_driven(''.join(map(chr,self.crc_data_buf)))
        if crc_read == crc_computed:
            self.data_crc_ok = True
        
    # --------------------------------------------------------------------
    def callback_none(self, arg):
        pass

    # --------------------------------------------------------------------
    def feed(self, s):
        result = self.layout[self.phase].feed(s)

	# Phase is done, but we're still cooking
        if result == State.DONE:
            self.phase += 1
            return State.COOKING

	# Phase failed, cooking failed
        elif result == State.FAILED:
            print "Failed at: %s" % (self.layout[self.phase].name)
            self.phase = 0
            return State.FAILED

	# Last phase done, loop over and return success!
        elif result == State.LOOP_END:
            phase = 0
            return State.DONE


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
