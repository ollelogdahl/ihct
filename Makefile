exec = test
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -g -Wall -std=c99

$(exec): $(objects)
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c src/%.h
	gcc -c $(flags) $< -o $@

clean:
	-rm $(exec)
	-rm src/*.o