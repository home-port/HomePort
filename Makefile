CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-lev
SOURCES=main.c webserver.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=wstest

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o wstest

mrproper:
	make clean
	make

run:
	./$(EXECUTABLE)

debug:
	gdb -ex run ./$(EXECUTABLE)

memcheck:
	valgrind ./$(EXECUTABLE)
