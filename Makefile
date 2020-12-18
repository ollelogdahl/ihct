exec = test
sources = $(wildcard src/*.c)
sources += examples/ex.c
objects = $(sources:.c=.o)
LDFLAGS = -lpthread
INCLUDE = -I./src
CFLAGS = -g -Wall -std=gnu99 $(INCLUDE)
CC = gcc

$(exec): $(objects)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c $(sources)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(exec) src/*.o examples/*.o

.PHONY: clean
