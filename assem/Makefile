LEX=flex
YACC=bison
YFLAGS=-d -y
CC=gcc
CFLAGS=-std=c99 -Wall -O3 -D_XOPEN_SOURCE=500
CFLAGSLEX=-std=c99 -O3 -D_XOPEN_SOURCE=500
OBJS=assem_parse.o assem_scan.o ops.o elements.o eval.o main.o

assem: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

assem_scan.o: assem_scan.c
	$(CC) $(CFLAGSLEX) -c $^ -o $@

assem_scan.c: assem.l
	$(LEX) -o $@ $^

assem_parse.o: assem_parse.c
	$(CC) $(CFLAGS) -c $^ -o $@

assem_parse.c: assem.y
	$(YACC) $(YFLAGS) $^ -o $@

clean:
	rm -f *.o assem *.s assem_parse.[ch] assem_scan.[ch]

.PHONY: all clean
