###############################################################################
# Makefile for the loop-test.c
###############################################################################
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=

SOURCES=example1.c example2.c lib-juice.c
OBJECTS=$(SOURCES:.cpp=.o)
TARGETS=example1 example2

example1_objects= example1.o lib-juice.o
example2_objects= example2.o lib-juice.o


all: $(SOURCES) $(TARGETS)

example1: $(example1_objects)
	$(CC) $(LDFLAGS) $(example1_objects) -o $@

example2: $(example2_objects)
	$(CC) $(LDFLAGS) $(example2_objects) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGETS) $(example1_objects) $(example2_objects)
