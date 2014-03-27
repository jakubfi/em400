EMAS=emas
EMDAS=emdas
EMLIN=emlin
EMELF=emelfread

PROJ=boot_winch.bin

all:	$(PROJ)

%.bin: %.asm
	$(EMAS) -o $@ -Oraw $<

dasm: $(PROJ)
	$(EMDAS) -o $(PROJ).S $(PROJ)

clean:
	rm -f $(OBJ) $(PROJ) $(PROJ).S $(PROJ).bin
