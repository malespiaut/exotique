#!/bin/sh

gcc -c -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Wdouble-promotion  -Wcast-qual  -Wcast-align=strict  -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wfloat-equal -Wundef -Wpointer-arith -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -O0 -ggdb3 -I/usr/include/SDL2 -D_REENTRANT ../../exotique.c
gcc -c -std=c89 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Wdouble-promotion  -Wcast-qual  -Wcast-align=strict  -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wfloat-equal -Wundef -Wpointer-arith -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code   -O0 -ggdb3 -nostdlib -nostdinc -nodefaultlibs -nolibc -ffreestanding -D ARCH=64 shapes.c
gcc exotique.o shapes.o -lSDL2 -o shapes
