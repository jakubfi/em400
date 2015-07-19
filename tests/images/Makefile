IMAGE=winchester0.e4i

all:
	../../build/src/emitool --preset win20 --spt 16 --image $(IMAGE)
	./fillimage.py $(IMAGE)

clean:
	rm -f $(IMAGE)
