#!/bin/sh

gcc -c -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 -I/usr/include/SDL2 -D_REENTRANT -lSDL2 main.c
gcc -c -std=c89 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wstrict-aliasing=2 -Werror -Wfatal-errors -O0 -ggdb3 game.c
gcc main.o game.o -lSDL2 -o exotique
