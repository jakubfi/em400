CXX=g++
CXXFLAGS=-O3 -w -c -fpic -pthread
LFLAGS=-lSaleaeDevice -lreadline

OBJS=wds.o logic.o wdc.o

wds: $(OBJS)
	$(CXX) $(LFLAGS) $^ -o $@

clean:
	rm -f *.o wds

.PHONY: all clean

