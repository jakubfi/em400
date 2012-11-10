WITH_DEBUGER = 1

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
timer.o\
utils.o\
io.o\
errors.o

DEBUG_OBJS = \
debuger_parser.o\
debuger_scanner.o\
dasm.o\
dasm_iset.o\
debuger.o\
debuger_ui.o\
debuger_cmd.o\
debuger_eval.o\
awin.o

ifdef WITH_DEBUGER
OBJS += $(DEBUG_OBJS)
CFLAGS += -DWITH_DEBUGER
LFLAGS += -lncurses -lreadline
endif

em400: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

debuger_scanner.c: debuger_scanner.l
	$(LEX) -o $@ $^

debuger_scanner.o: debuger_scanner.c
	$(CC) -c $(LEXCFLAGS) $^ -o $@

debuger_parser.c: debuger_parser.y
	$(YACC) $(YFLAGS) $^ -o $@

asm: *.c
	$(CC) $(CFLAGS) -S $^

clean:
	rm -f *.o em400 *.s debuger_parser.c debuger_scanner.c debuger_parser.h

.PHONY: all clean
