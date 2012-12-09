###############################################################################
# Makefile for the loop-test.c
###############################################################################
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=

SOURCES=example1.c example2.c example3.c example4.c lib-juice.c
OBJECTS=$(SOURCES:.cpp=.o)
TARGETS=example1 example2 example3 example4

example1_objects= example1.o lib-juice.o
example2_objects= example2.o lib-juice.o
example3_objects= example3.o lib-juice.o
example4_objects= example4.o lib-juice.o


all: $(SOURCES) $(TARGETS)

example1: $(example1_objects)
	$(CC) $(LDFLAGS) $(example1_objects) -o $@

example2: $(example2_objects)
	$(CC) $(LDFLAGS) $(example2_objects) -o $@

example3: $(example3_objects)
	$(CC) $(LDFLAGS) $(example3_objects) -o $@

example4: $(example4_objects)
	$(CC) $(LDFLAGS) $(example4_objects) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf *.o *~ $(TARGETS)
