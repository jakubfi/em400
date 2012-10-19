CC=gcc
CFLAGS=-std=c99 -Wall -O3 -D_XOPEN_SOURCE=600
LFLAGS=-lncurses -lrt
OBJS=\
em400.o\
mjc400.o\
mjc400_iset.o\
mjc400_instr.o\
mjc400_dasm.o\
em400_mem.o\
mjc400_regs.o\
mjc400_timer.o\
em400_debuger.o\
em400_utils.o\
em400_io.o\
em400_errors.o \
em400_debuger_nc.o

em400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o em400 *.s

.PHONY: all clean
