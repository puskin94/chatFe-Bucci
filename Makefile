SOURCEDIR=src/server/
INDIR=src/server/include
OBJDIR=bin/


CC=gcc
CFLAGS=-Wall
LDFLAGS=-lpthread  -lncurses
SOURCES=chat-server.c hash.c lista.c threadMain.c threadWorker.c utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=chat-server

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
    $(CC) $(CFLAGS) $< -o $@
