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

# Makefile targets

all: dir ysh run

dir: 
	mkdir -p $(BU)

ysh: $(RC)/ysh.c
	$(CC) $(CCOPTS) -o $(BU)/$@ $< $(NCURSES)

# Run my_top
run: $(BU)/ysh
	$(BU)/ysh

# Clean up!
clean:
	rm -f $(BU)/*
