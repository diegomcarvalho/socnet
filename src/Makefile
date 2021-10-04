MODULE = socnet$(shell python3-config --extension-suffix)
VERSION = $(shell git describe --tags --always --long --dirty)
INLCUDES = $(shell python3 -m pybind11 --includes)
OBS = calculate.o population.o pybindmodule.o
CC = clang++

ifeq ($(shell uname -s), Darwin)
	COSFLAGS = #-include-pch statistics.hpp.gch
    LOSFLAGS = -shared -undefined dynamic_lookup
else
	COSFLAGS = -fPIC -shared
    LOSFLAGS = -shared -fPIC
endif

CFLAGS = -O3 -Wall -std=c++2a $(INLCUDES) $(COSFLAGS)
LDFLAGS= $(LOSFLAGS)

all: $(MODULE)

population.o: population.cc population.hpp
	$(CC) $(CFLAGS) -o $@ -c $< 

calculate.o: calculate.cc population.hpp statistics.hpp dynamics.hpp
	$(CC) $(CFLAGS) -o $@ -c $<

pybindmodule.o: pybindmodule.cc calculate.o
	$(CC) $(CFLAGS) -o $@ -c $<

$(MODULE): $(OBS)
	$(CC) $(LDFLAGS) -o $@ $(OBS)
	mv $(MODULE) ..

test:
	./test.sh

clean:
	rm -fr $(MODULE) __pycache__ *.o
