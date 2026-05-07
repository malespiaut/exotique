# 🌴 Exotique

Exotique is a lightweight, low-level multimedia library written in C, designed for creating small games and applications with a focus on simplicity, portability, and "bare-metal" feel.
It provides a thin abstraction over SDL2 to handle graphics, input, and timing.

## Features

* **Retro-Style Graphics**: 8-bit indexed color palette (256 colors) with a software-defined screen buffer.
* **4-Player Input Support**: Simultaneous support for up to 4 gamepads with dynamic hot-plugging. Player 1 also maps to the keyboard by default.
* **Minimalistic API**: A simple `load`/`update`/`draw` loop structure.

## Example code

* **Bitmap Font System**: Includes built-in support for multiple 1-bit bitmap fonts (8x8 and 8x16) with flexible rendering.
* **Memory Safe Drawing**: Primitive drawing functions (like `rectangle_fill_draw`) are hardened with robust clamping to prevent out-of-bounds writes.

## Getting Started

### Prerequisites

* A C compiler (GCC recommended)
* SDL2 development libraries

On Linux (Debian/Ubuntu):
```bash
sudo apt-get install libsdl2-dev
```

### Building the Examples

The `examples` directory contains several demonstrations of the library's capabilities, including a 4-player input test, bitmap font demo, and classic games like Breakout and Tetris.

To build an example (e.g., the input test):

```bash
cd src/examples
sh build.sh input_test
./input_test
```

The build script automatically handles `.c` extensions, so `sh build.sh input_test.c` is also safe to use.

## Project Structure

* `src/exotique.h`: The main library header defining the interface and data structures.
* `src/exotique.c`: The core library implementation (SDL2 backend).
* `src/examples/`: A collection of demonstration programs.
  * `input_test.c`: Comprehensive test for the 4-player input system.
  * `bitmap_font.c`: Demonstrates the built-in font rendering.
  * `breakout.c` / `tetris.c` / `snake.c`: Sample games implemented with Exotique.

## License

This project is dual licensed under the MIT and 0BSD licenses.
Feel free to use the one best suited for your needs!
If you want to give me credits, please use the MIT license!
