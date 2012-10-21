CC=gcc
CFLAGS=-std=c99 -Wall -O3 -D_XOPEN_SOURCE=600
LFLAGS=-lncurses -lrt
OBJS=\
em400.o\
cpu.o\
iset.o\
instructions.o\
dasm.o\
memory.o\
registers.o\
timer.o\
debuger.o\
utils.o\
io.o\
errors.o \
debuger_ui.o

em400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o em400 *.s

.PHONY: all clean
