CC=gcc

wolfram: main.c subs.c bmp.c cellular_automata.h
	$(CC) -Wall -o wolfram main.c
