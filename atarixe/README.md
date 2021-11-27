# Atari XEGS development notes 
This folder contains notes for development software and games in the C and 6502 Assembly programming languages, using the cc65 toolchain and the atari800 emulator.

## Compile
    /opt/cc65/bin/cc65 -t atari -Oi test01.c 

## Assemble
    /opt/cc65/bin/ca65 test01.s 

## Link
    /opt/cc65/bin/ld65 -o test01 -t atari test01.o atari.lib

## Run
    /opt/atari800/bin/atari800 -xegs -run test01

## References
1. Atari800 emulator homepage: https://github.com/atari800/atari800
1. cc65 homepage: https://cc65.github.io
