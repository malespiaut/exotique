#!/bin/sh
CFLAGS="-Wall -Wextra -Wpedantic -Waggregate-return    -Wcast-align -Wcast-qual -Wconversion -Wdouble-promotion      -Wfloat-equal -Wformat=2 -Wmissing-include-dirs -Wnull-dereference -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=2 -Wswitch-default -Wswitch-enum -Wundef -Wuninitialized -Wwrite-strings -Werror -Wfatal-errors"
GCC_CFLAGS="-Warith-conversion -Wduplicated-branches -Wduplicated-cond -Wlogical-op" 
SCREEN_SIZE=""
ARCH=64
clang -c $CFLAGS -std=c23 -O0 -ggdb3 -I/usr/include/SDL2 -D_REENTRANT -D kScreenWidth=160 -D kScreenHeight=100 ../../exotique.c
clang -c $CFLAGS -std=c89 -O0 -ggdb3 -nostdlib -nostdinc -nodefaultlibs -ffreestanding -D ARCH=64 -D kScreenWidth=160 -D kScreenHeight=100 breakout.c
clang exotique.o breakout.o -lSDL2 -o breakout
