IMAGE_W0=winchester0.e4i
IMAGE_W1=winchester1.e4i
IMAGE_F8_PREWRITE_0=flop8_prewrite_0.img
IMAGE_F8_PREWRITE_1=flop8_prewrite_1.img
IMAGE_F8_PREWRITE_2=flop8_prewrite_2.img
IMAGE_F8_PREWRITE_3=flop8_prewrite_3.img
IMAGE_F8_EMPTY_0=flop8_empty_0.img
IMAGE_F8_EMPTY_1=flop8_empty_1.img
IMAGE_F8_EMPTY_2=flop8_empty_2.img
IMAGE_F8_EMPTY_3=flop8_empty_3.img

all:	$(IMAGE_W0) $(IMAGE_W1) $(IMAGE_F8_PREWRITE_0) $(IMAGE_F8_PREWRITE_1) $(IMAGE_F8_PREWRITE_2) $(IMAGE_F8_PREWRITE_3) $(IMAGE_F8_EMPTY_0) $(IMAGE_F8_EMPTY_1) $(IMAGE_F8_EMPTY_2) $(IMAGE_F8_EMPTY_3)

$(IMAGE_W0):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_W0)
	./fillimage.py $(IMAGE_W0) $$((0x1a + 1*4*16*512)) 0 614 4 16 512
$(IMAGE_W1):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_W1)

$(IMAGE_F8_PREWRITE_0):
	dd if=/dev/zero of=$(IMAGE_F8_PREWRITE_0) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_PREWRITE_0) 0 0 77 1 26 128
$(IMAGE_F8_PREWRITE_1):
	dd if=/dev/zero of=$(IMAGE_F8_PREWRITE_1) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_PREWRITE_1) 0 $$((1*77*26)) 77 1 26 128
$(IMAGE_F8_PREWRITE_2):
	dd if=/dev/zero of=$(IMAGE_F8_PREWRITE_2) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_PREWRITE_2) 0 $$((2*77*26)) 77 1 26 128
$(IMAGE_F8_PREWRITE_3):
	dd if=/dev/zero of=$(IMAGE_F8_PREWRITE_3) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_PREWRITE_3) 0 $$((3*77*26)) 77 1 26 128

$(IMAGE_F8_EMPTY_0):
	dd if=/dev/zero of=$(IMAGE_F8_EMPTY_0) bs=128 count=$$((77*26))
$(IMAGE_F8_EMPTY_1):
	dd if=/dev/zero of=$(IMAGE_F8_EMPTY_1) bs=128 count=$$((77*26))
$(IMAGE_F8_EMPTY_2):
	dd if=/dev/zero of=$(IMAGE_F8_EMPTY_2) bs=128 count=$$((77*26))
$(IMAGE_F8_EMPTY_3):
	dd if=/dev/zero of=$(IMAGE_F8_EMPTY_3) bs=128 count=$$((77*26))

clean:
	rm -f $(IMAGE_W0) $(IMAGE_W1) $(IMAGE_F8_PREWRITE_0) $(IMAGE_F8_PREWRITE_1) $(IMAGE_F8_PREWRITE_2) $(IMAGE_F8_PREWRITE_3) $(IMAGE_F8_EMPTY_0) $(IMAGE_F8_EMPTY_1) $(IMAGE_F8_EMPTY_2) $(IMAGE_F8_EMPTY_3)
