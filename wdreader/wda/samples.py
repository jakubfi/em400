# ------------------------------------------------------------------------
class Samples:

    # --------------------------------------------------------------------
    def __init__(self, wds_file):
        print "Loading wds file: %s" % wds_file
        wds_f = open(wds_file, "r")
        self.data = bytearray(wds_f.read())
        wds_f.close()
        print "Bytes loaded: %d" % len(self.data)

        self.byte_pos = 0
        self.bit_pos = 7

    # --------------------------------------------------------------------
    def __iter__(self):
        self.byte_pos = 0
        self.bit_pos = 7
        return self

    # --------------------------------------------------------------------
    def __len__(self):
        return len(self.data)*8

    # --------------------------------------------------------------------
    def next(self):
        if self.byte_pos >= len(self.data):
            raise StopIteration
        else:
            v = (self.data[self.byte_pos] >> self.bit_pos) & 1

            if self.bit_pos == 0:
                self.bit_pos = 7
                self.byte_pos += 1
            else:
                self.bit_pos -= 1

            return v

