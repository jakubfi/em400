CXX=g++
CXXFLAGS=-O3 -w -c -fpic -pthread
LFLAGS=-lSaleaeDevice

OBJS=wds.o

wds: $(OBJS)
	$(CXX) $(LFLAGS) $^ -o $@

clean:
	rm -f *.o wds

.PHONY: all clean

