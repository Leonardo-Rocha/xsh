CC = gcc
BU = ./build
RC = ./src

# Compiler flags
#
#-O2:			Turn on all optional optimizations except for loop
#			unrolling and function inlining.
#
#
#-Wall:			All of the `-W' options combined (all warnings on)
# 
#-Og:			Optmize and enable debug  
CCOPTS = -Wall -Og 
NCURSES = -lncurses
READLINE = -lreadline

# Makefile targets

all: dir ysh 

dir: 
	mkdir -p $(BU)

ysh: $(RC)/ysh.c
	$(CC) $(CCOPTS) -o $(BU)/$@ $< $(NCURSES) $(READLINE)

# Run my_top
run: $(BU)/ysh
	$(BU)/ysh

# Clean up!
clean:
	rm -f $(BU)/*
