#!/bin/bash

# Build script for OpenGL 1.1 backend
# Usage: sh build_ogl1.sh [example_name]

if [ $# -eq 0 ]; then
    echo "Usage: $0 <example_name>"
    echo "Example: $0 breakout"
    exit 1
fi

EXAMPLE_NAME="$1"
EXAMPLE_FILE="${EXAMPLE_NAME}.c"

if [ ! -f "$EXAMPLE_FILE" ]; then
    echo "Error: Example file '$EXAMPLE_FILE' not found"
    exit 1
fi

# Define compiler and flags
CC="gcc"
CFLAGS="-std=c89 -Wall -Wextra -Werror -Wfatal-errors -pedantic"
CFLAGS="$CFLAGS -Wno-unused-parameter -Wno-unused-variable"
CFLAGS="$CFLAGS -DARCH=64"

# OpenGL and GLUT libraries
LIBS="-lGL -lglut -lm"

# Include directories
INCLUDES="-I.."

echo "Building $EXAMPLE_NAME with OpenGL 1.1 backend..."

# Compile
$CC $CFLAGS $INCLUDES \
    "../exotique_ogl1.c" \
    "$EXAMPLE_FILE" \
    $LIBS \
    -o "${EXAMPLE_NAME}_ogl1"

if [ $? -eq 0 ]; then
    echo "Build successful: ${EXAMPLE_NAME}_ogl1"
else
    echo "Build failed"
    exit 1
fi