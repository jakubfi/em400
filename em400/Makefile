WITH_DEBUGGER = 1

CC = gcc
LEX = flex
YACC = bison
YFLAGS = -d -y
CFLAGS_BASIC = -std=c99 -O3 -D_XOPEN_SOURCE=600 -I include
CFLAGS = $(CFLAGS_BASIC) -Wall
LEXCFLAGS = $(CFLAGS_BASIC)
LFLAGS = -lrt

OBJS = \
em400.o\
cpu.o\
iset.o\
instructions.o\
memory.o\
registers.o\
interrupts.o\
timer.o\
utils.o\
io.o\
errors.o

DEBUG_OBJS = \
debugger_parser.o\
debugger_scanner.o\
dasm.o\
dasm_iset.o\
debugger.o\
debugger_ui.o\
debugger_cmd.o\
debugger_eval.o\
awin.o

ifdef WITH_DEBUGGER
OBJS += $(DEBUG_OBJS)
CFLAGS += -DWITH_DEBUGGER
LFLAGS += -lncurses -lreadline
endif

em400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

debugger_scanner.c: debugger_scanner.l
	$(LEX) -o $@ $^

debugger_scanner.o: debugger_scanner.c
	$(CC) $(LEXCFLAGS) $^ -c -o $@

debugger_parser.c: debugger_parser.y
	$(YACC) $(YFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o em400 *.s debugger_parser.c debugger_scanner.c debugger_parser.h

.PHONY: all clean
