
.DEFAULT_GOAL := all

COMPILER_TYPE= gnu
CC = gcc
CCFLAGS= -Wall -pthread

SAUNA = sauna.out
SAUNA_SRC = sauna.c
SAUNA_OBJ = $(SAUNA_SRC:.c=.o)

GERADOR = gerador.out
GERADOR_SRC = gerador.c
GERADOR_OBJ = $(GERADOR_SRC:.c=.o)

ALL = sauna gerador

SOURCES = shared.c
SHARED_OBJ = $(SOURCES:.c=.o)

.PHONY : all
all : $(ALL) clean

sauna : $(SAUNA_OBJ) $(SHARED_OBJ)
	$(CC) $(CCFLAGS) $< $(SHARED_OBJ) -o $(SAUNA)

gerador : $(GERADOR_OBJ) $(SHARED_OBJ)
	$(CC) $(CCFLAGS) $< $(SHARED_OBJ) -o $(GERADOR)

# To obtain object files
%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

# To remove generated files
.PHONY: clean
clean:
	rm -f $(SAUNA_OBJ) $(GERADOR_OBJ) $(SHARED_OBJ) $(PROG)
