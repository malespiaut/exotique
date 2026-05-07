#!/bin/sh

EXTRA_CFLAGS="-Waggregate-return -Warith-conversion -Wcast-align=strict -Wcast-qual -Wconversion -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wformat=2 -Wlogical-op -Wmissing-include-dirs -Wnull-dereference -Wstrict-aliasing=2 -Wstrict-overflow=2 -Wswitch-default -Wswitch-enum -Wundef -Wuninitialized -Wwrite-strings"
CFLAGS="-Wall -Wextra -Wpedantic -Wshadow -Werror -Wfatal-errors $EXTRA_CFLAGS"
ARCH=64
if [ -z "$1" ]; then
  echo "Usage: $0 <game_name>"
  exit 1
fi

GAME="${1%.c}"

gcc -c $CFLAGS -fsanitize=address -std=c23 -O0 -ggdb3 -I /usr/include/SDL2 -D_REENTRANT ../exotique.c
gcc -c $CFLAGS -fsanitize=address -std=c89 -O0 -ggdb3 -I ../ -nostdlib -nostdinc -nodefaultlibs -nolibc -ffreestanding -D ARCH=$ARCH "$GAME".c
gcc exotique.o "$GAME".o -lasan -lSDL2 -o "$GAME" -fsanitize=address
