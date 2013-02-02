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
                print "Sector %d done" % (len(self.sectors)+1)
                self.sectors.append(sector)
                sector = Sector()
                if len(self.sectors) == sectors:
                    print "Got %d sectors, that's it!" % sectors
                    break

            elif res == State.FAILED:
                print "Cooking sector failed"
                break


    # --------------------------------------------------------------------
    def get_data(self):
        for s in self.sectors:
            s.get_data()

