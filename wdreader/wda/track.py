from mfm import *
from sector import *

# ------------------------------------------------------------------------
class Track:

    # --------------------------------------------------------------------
    def __init__(self, wds_file, sectors):
        self.data = MFMData(wds_file)

        self.sectors = []

	print "Cooking sectors..."

        sector = Sector()
        for s in self.data:
            res = sector.feed(s)

            if res == State.DONE:
                print "Sector %2d: %3d/%d/%2d CRC header: %s, CRC data: %s, BAD: %s" % (len(self.sectors)+1, sector.cylinder, sector.head, sector.sector, str(sector.head_crc_ok), str(sector.head_crc_ok), str(sector.bad))
                self.sectors.append(sector)
                sector = Sector()
                if len(self.sectors) == sectors:
                    break

            elif res == State.FAILED:
                print "Cooking sector failed"
                break


    # --------------------------------------------------------------------
    def get_data(self):
        for s in self.sectors:
            s.get_data()


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
