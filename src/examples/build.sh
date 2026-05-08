#!/bin/sh

DEBUG="-O0 -ggdb3 -fsanitize=address"
EXTRA_CFLAGS="-Waggregate-return -Warith-conversion -Wcast-align=strict -Wcast-qual -Wconversion -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wformat=2 -Wlogical-op -Wmissing-include-dirs -Wnull-dereference -Wstrict-aliasing=2 -Wstrict-overflow=2 -Wswitch-default -Wswitch-enum -Wundef -Wuninitialized -Wwrite-strings"
CFLAGS="-Wall -Wextra -Wpedantic -Wshadow -Werror -Wfatal-errors $EXTRA_CFLAGS $DEBUG -I ../"

if [ -z "$1" ]; then
  echo "Usage: $0 <game_name>"
  exit 1
fi

GAME="${1%.c}"

gcc -c $CFLAGS -std=c23 -I /usr/include/SDL2 -D_REENTRANT ../exotique.c
gcc -c $CFLAGS -std=c89 -nostdlib -nostdinc -nodefaultlibs -nolibc -ffreestanding "$GAME".c

gcc exotique.o "$GAME".o -o "$GAME" -lSDL2 -lasan -fsanitize=address
