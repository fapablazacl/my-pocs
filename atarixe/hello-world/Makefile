#!/bin/bash

CC=cc65
AS=ca65
LD=ld65

main.s: main.c
	$(CC) -t atari -Oi main.c

main.o: main.s
	$(AS) main.s 

hello-world.xex: main.o
	$(LD) -o hello-world.xex -t atari main.o atari.lib

.PHONY: clean

clean:
	rm main.s main.o
