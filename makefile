
CC = gcc

CCFLAGS= -Wall

OBJECTS = sfind.o finder.o

sfind: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o sfind

finder.o: finder.c finder.h
	$(CC) $(CFLAGS) -c finder.c
	
sfind.o: finder.h sfind.c


