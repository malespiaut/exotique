#!/usr/bin/env python3
"""
PNG to Binary Converter

Converts PNG images directly to binary data files.
Combines PNG-to-hex and hex-to-binary conversion in a single step.
"""

import argparse
import sys
from pathlib import Path
from typing import Tuple

import numpy as np
from PIL import Image

# Constants
BINARY_THRESHOLD = 127
BITS_PER_BYTE = 8
DEFAULT_OUTPUT_FILE = "output.bin"


def png_to_binary(image_path: str, threshold: int = BINARY_THRESHOLD) -> bytes:
    """
    Convert PNG image directly to binary data.

    Args:
        image_path: Path to the input PNG image
        threshold: Binary threshold value (0-255)

    Returns:
        Binary data as bytes object

    Raises:
        FileNotFoundError: If the image file doesn't exist
        PIL.UnidentifiedImageError: If the file is not a valid image
    """
    if not Path(image_path).exists():
        raise FileNotFoundError(f"Image file not found: {image_path}")

    # Open and convert image to grayscale
    img = Image.open(image_path).convert("L")

    # Convert to numpy array and threshold to binary
    binary_array = np.array(img) > threshold

    # Get dimensions and apply padding
    height, width = binary_array.shape
    binary_array = _pad_width_to_byte_boundary(binary_array, width)

    # Convert binary array directly to bytes
    return _binary_array_to_bytes(binary_array, height)


def _pad_width_to_byte_boundary(binary_array: np.ndarray, width: int) -> np.ndarray:
    """Pad array width to be divisible by BITS_PER_BYTE."""
    padding_needed = (BITS_PER_BYTE - (width % BITS_PER_BYTE)) % BITS_PER_BYTE
    if padding_needed:
        binary_array = np.pad(
            binary_array,
            ((0, 0), (0, padding_needed)),
            mode="constant",
            constant_values=False,
        )
    return binary_array


def _binary_array_to_bytes(binary_array: np.ndarray, height: int) -> bytes:
    """Convert binary array directly to bytes."""
    width = binary_array.shape[1]
    byte_data = []

    for row in range(height):
        for col in range(0, width, BITS_PER_BYTE):
            pixel_group = binary_array[row, col : col + BITS_PER_BYTE]
            byte_value = sum(
                int(pixel) << (BITS_PER_BYTE - 1 - i)
                for i, pixel in enumerate(pixel_group)
            )
            byte_data.append(byte_value)

    return bytes(byte_data)


def save_binary_data(binary_data: bytes, output_path: str) -> None:
    """Save binary data to file."""
    with open(output_path, "wb") as f:
        f.write(binary_data)
    print(f"Converted {len(binary_data)} bytes to {output_path}")


def parse_arguments() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Convert PNG images directly to binary data"
    )
    parser.add_argument("input_image", help="Path to the input PNG image")
    parser.add_argument(
        "-o",
        "--output",
        default=DEFAULT_OUTPUT_FILE,
        help=f"Output binary file (default: {DEFAULT_OUTPUT_FILE})",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=BINARY_THRESHOLD,
        help=f"Binary threshold 0-255 (default: {BINARY_THRESHOLD})",
    )
    return parser.parse_args()


def main() -> None:
    """Main entry point."""
    args = parse_arguments()

    if not 0 <= args.threshold <= 255:
        print("Error: Threshold must be between 0 and 255", file=sys.stderr)
        sys.exit(1)

    try:
        binary_data = png_to_binary(args.input_image, args.threshold)
        save_binary_data(binary_data, args.output)
    except (FileNotFoundError, ValueError, IOError) as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
