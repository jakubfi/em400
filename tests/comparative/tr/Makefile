PRJ=tr
INCLUDE_DIR=../../../../asm/lib/
CFG=tr.ini
HW_UPLOAD_PORT=/dev/ttyUSB0
EM400=../../../build/em400
EMBIN=embin
EMAS=emas

$(PRJ): $(PRJ).ppasm
	$(EMAS) -I $(INCLUDE_DIR) -o $(PRJ) -c mera400 -Oraw $(PRJ).ppasm

%.ppasm: %.asm
	cpp -I $(INCLUDE_DIR) -P -undef $< -o $@

emu: $(PRJ)
	$(EM400) -c $(CFG) -p $(PRJ)

push: $(PRJ)
	$(EMBIN) -o $(HW_UPLOAD_PORT) $(PRJ)

clean:
	rm -f $(PRJ) $(PRJ).ppasm *.log
