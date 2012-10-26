WITH_DEBUGER = 1

CC = gcc
CFLAGS = -std=c99 -Wall -O3 -D_XOPEN_SOURCE=600 -I include
LFLAGS = -lncurses -lrt

OBJS = \
em400.o\
cpu.o\
iset.o\
instructions.o\
memory.o\
registers.o\
timer.o\
utils.o\
io.o\
errors.o

DEBUGOBJS = \
dasm.o\
dasm_iset.o\
debuger.o\
debuger_ui.o

ifdef WITH_DEBUGER
OBJS += $(DEBUGOBJS)
CFLAGS += -DWITH_DEBUGER
endif

em400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o em400 *.s

.PHONY: all clean
