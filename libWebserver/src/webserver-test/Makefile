CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-lcurl
SOURCES=main.c client.c tests.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=wsTester

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o wsTester

mrproper:
	make clean
	make

run:
	./$(EXECUTABLE)

debug:
	gdb -ex run ./$(EXECUTABLE)

memcheck:
	valgrind ./$(EXECUTABLE)

