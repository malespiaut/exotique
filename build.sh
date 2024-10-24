#!/bin/sh

gcc -c -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 -I/usr/include/SDL2 -D_REENTRANT -DARCH=64 src/exotique.c
gcc -c -std=c89 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 -nostdlib -nostdinc -nodefaultlibs -nolibc -ffreestanding -DARCH=64 src/game.c
gcc exotique.o game.o -lSDL2 -o exotique
