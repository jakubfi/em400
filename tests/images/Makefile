IMAGE_0=winchester0.e4i
IMAGE_1=winchester1.e4i

all:	$(IMAGE_0) $(IMAGE_1)

$(IMAGE_0):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_0)
	./fillimage.py $(IMAGE_0)
$(IMAGE_1):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_1)

clean:
	rm -f $(IMAGE_0) $(IMAGE_1)
