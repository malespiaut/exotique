#!/bin/sh

gcc -c -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 -I/usr/include/SDL2 -D_REENTRANT ../../exotique.c
gcc -c -std=c89 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 -nostdlib -nostdinc -nodefaultlibs -nolibc -ffreestanding -D ARCH=64 bitmap_font.c
gcc exotique.o bitmap_font.o -lSDL2 -o bitmap_font
