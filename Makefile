CC=gcc
CFLAGS=-std=c99 -Wall -O3
OBJS=mjc400.o mera400.o mjc400_iset.o mjc400_instr.o mjc400_mem.o mjc400_regs.o

mera400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o mera400 *.s

.PHONY: all clean
