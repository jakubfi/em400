IMAGE_W0=winchester0.e4i
IMAGE_W1=winchester1.e4i
IMAGE_F8_0=flop8_0.img
IMAGE_F8_1=flop8_1.img
IMAGE_F8_2=flop8_2.img
IMAGE_F8_3=flop8_3.img
IMAGE_F8W_0=flop8w_0.img
IMAGE_F8W_1=flop8w_1.img
IMAGE_F8W_2=flop8w_2.img
IMAGE_F8W_3=flop8w_3.img

all:	$(IMAGE_W0) $(IMAGE_W1) $(IMAGE_F8_0) $(IMAGE_F8_1) $(IMAGE_F8_2) $(IMAGE_F8_3) $(IMAGE_F8W_0) $(IMAGE_F8W_1) $(IMAGE_F8W_2) $(IMAGE_F8W_3) 

$(IMAGE_W0):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_W0)
	./fillimage.py $(IMAGE_W0) $$((0x1a + 1*4*16*512)) 0 614 4 16 512
$(IMAGE_W1):
	../../build/emitool --preset win20 --spt 16 --image $(IMAGE_W1)

$(IMAGE_F8_0):
	dd if=/dev/zero of=$(IMAGE_F8_0) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_0) 0 0 77 1 26 128
$(IMAGE_F8_1):
	dd if=/dev/zero of=$(IMAGE_F8_1) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_1) 0 $$((1*77*26)) 77 1 26 128
$(IMAGE_F8_2):
	dd if=/dev/zero of=$(IMAGE_F8_2) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_2) 0 $$((2*77*26)) 77 1 26 128
$(IMAGE_F8_3):
	dd if=/dev/zero of=$(IMAGE_F8_3) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8_3) 0 $$((3*77*26)) 77 1 26 128

$(IMAGE_F8W_0):
	dd if=/dev/zero of=$(IMAGE_F8W_0) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8W_0) 0 0 77 1 26 128
$(IMAGE_F8W_1):
	dd if=/dev/zero of=$(IMAGE_F8W_1) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8W_1) 0 $$((1*77*26)) 77 1 26 128
$(IMAGE_F8W_2):
	dd if=/dev/zero of=$(IMAGE_F8W_2) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8W_2) 0 $$((2*77*26)) 77 1 26 128
$(IMAGE_F8W_3):
	dd if=/dev/zero of=$(IMAGE_F8W_3) bs=128 count=$$((77*26))
	./fillimage.py $(IMAGE_F8W_3) 0 $$((3*77*26)) 77 1 26 128


clean:
	rm -f $(IMAGE_W0) $(IMAGE_W1) $(IMAGE_F8_0) $(IMAGE_F8_1) $(IMAGE_F8_2) $(IMAGE_F8_3) $(IMAGE_F8W_0) $(IMAGE_F8W_1) $(IMAGE_F8W_2) $(IMAGE_F8W_3)
