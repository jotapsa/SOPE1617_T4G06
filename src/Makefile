
.DEFAULT_GOAL := all

COMPILER_TYPE= gnu
CC = gcc
CCFLAGS= -Wall

PROG = sfind.out

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY : all
all : $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) -o $(PROG)
	rm -f $(OBJECTS)

# To obtain object files
%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

# To remove generated files
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(PROG)
