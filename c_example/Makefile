###############################################################################
# Makefile for the loop-test.c
###############################################################################
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=

SOURCES=example1.c lib-juice.c
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=example1

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGET)


