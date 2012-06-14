###############################################################################
# Makefile for the loop-test.c
###############################################################################
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=

SOURCES=loop-test.c
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=loop-test

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(OBJECTS) $(TARGET)


