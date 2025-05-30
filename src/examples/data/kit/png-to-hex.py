#!/usr/bin/env python3
"""
PNG to Hexadecimal Converter

Converts PNG images to hexadecimal representation where each byte represents 8 binary pixels.
Black pixels are represented as 0, white pixels as 1.
"""

from typing import List
import argparse
import sys
from pathlib import Path

from PIL import Image
import numpy as np

# Constants
BINARY_THRESHOLD = 127
BITS_PER_BYTE = 8
DEFAULT_OUTPUT_FILE = "output.hex"


def png_to_hex(image_path: str) -> List[str]:
    """
    Convert PNG image to hexadecimal where each byte represents 8 binary pixels.

    Args:
        image_path: Path to the input PNG image

    Returns:
        List of hexadecimal strings representing the image data

    Raises:
        FileNotFoundError: If the image file doesn't exist
        PIL.UnidentifiedImageError: If the file is not a valid image
    """
    if not Path(image_path).exists():
        raise FileNotFoundError(f"Image file not found: {image_path}")

    # Open and convert image to grayscale
    img = Image.open(image_path).convert("L")

    # Convert to numpy array and threshold to binary (0 and 1)
    # Pixels with value > BINARY_THRESHOLD become 1 (white), others become 0 (black)
    binary_array = np.array(img) > BINARY_THRESHOLD

    # Get image dimensions
    height, width = binary_array.shape

    # Calculate padding needed to make width divisible by BITS_PER_BYTE
    padding_needed = (BITS_PER_BYTE - (width % BITS_PER_BYTE)) % BITS_PER_BYTE
    if padding_needed:
        # Pad with zeros (black pixels) on the right
        binary_array = np.pad(
            binary_array,
            ((0, 0), (0, padding_needed)),
            mode="constant",
            constant_values=False,
        )
        width += padding_needed

    # Convert binary array to hexadecimal values
    hex_values = _convert_binary_to_hex(binary_array, height, width)

    return hex_values


def _convert_binary_to_hex(
    binary_array: np.ndarray, height: int, width: int
) -> List[str]:
    """
    Convert binary array to list of hexadecimal strings.

    Args:
        binary_array: 2D numpy array of boolean values
        height: Image height in pixels
        width: Image width in pixels (must be divisible by 8)

    Returns:
        List of hexadecimal strings
    """
    hex_values = []

    # Process image row by row
    for row in range(height):
        # Process each group of BITS_PER_BYTE pixels
        for col in range(0, width, BITS_PER_BYTE):
            # Extract BITS_PER_BYTE pixels
            pixel_group = binary_array[row, col : col + BITS_PER_BYTE]

            # Convert pixel group to a single byte value
            byte_value = _pixels_to_byte(pixel_group)

            # Convert byte to hexadecimal string and append
            hex_values.append(f"{byte_value:02x}")

    return hex_values


def _pixels_to_byte(pixels: np.ndarray) -> int:
    """
    Convert array of 8 boolean pixels to a single byte value.

    Args:
        pixels: Array of 8 boolean values representing pixels

    Returns:
        Integer byte value (0-255)
    """
    byte_value = 0
    for i, pixel in enumerate(pixels):
        if pixel:  # if white pixel (True)
            byte_value |= 1 << (BITS_PER_BYTE - 1 - i)  # Set bit from left to right
    return byte_value


def save_hex_to_file(hex_values: List[str], output_path: str) -> None:
    """
    Save hexadecimal values to a file.

    Args:
        hex_values: List of hexadecimal strings
        output_path: Path to the output file
    """
    try:
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(" ".join(hex_values))
        print(f"Hexadecimal data saved to: {output_path}")
    except IOError as e:
        print(f"Error writing to file {output_path}: {e}", file=sys.stderr)
        sys.exit(1)


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments.

    Returns:
        Parsed command line arguments
    """
    parser = argparse.ArgumentParser(
        description="Convert PNG images to hexadecimal representation"
    )
    parser.add_argument("input_image", help="Path to the input PNG image")
    parser.add_argument(
        "-o",
        "--output",
        default=DEFAULT_OUTPUT_FILE,
        help=f"Output file path (default: {DEFAULT_OUTPUT_FILE})",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=BINARY_THRESHOLD,
        help=f"Binary threshold value 0-255 (default: {BINARY_THRESHOLD})",
    )
    parser.add_argument(
        "--print-only",
        action="store_true",
        help="Print hexadecimal values to stdout only, do not save to file",
    )

    return parser.parse_args()


def main() -> None:
    """Main entry point of the application."""
    args = parse_arguments()

    # Validate threshold value
    if not 0 <= args.threshold <= 255:
        print("Error: Threshold must be between 0 and 255", file=sys.stderr)
        sys.exit(1)

    # Update global threshold if different from default
    global BINARY_THRESHOLD
    BINARY_THRESHOLD = args.threshold

    try:
        # Convert PNG to hexadecimal
        hex_values = png_to_hex(args.input_image)

        # Print results
        print("Hexadecimal representation:")
        hex_string = " ".join(hex_values)
        print(hex_string)

        # Save to file unless print-only mode is enabled
        if not args.print_only:
            save_hex_to_file(hex_values, args.output)

    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
