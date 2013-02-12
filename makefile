CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-lev
SOURCES=main.c webserver.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=wstest

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o wstest