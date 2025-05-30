#!/usr/bin/env python3
"""
Hexadecimal to Binary Converter

Converts hexadecimal strings to binary data files.
Supports various hex formats including space-separated, comma-separated,
and 0x-prefixed values.
"""

import argparse
import re
import sys
from pathlib import Path
from typing import List

# Constants
HEX_BYTE_LENGTH = 2
DEFAULT_OUTPUT_FILE = "output.bin"


def hex_to_binary(hex_string: str) -> bytes:
    """
    Convert a string of hexadecimal values to binary data.

    Supports multiple input formats:
    - Space-separated: "00 01 02 03"
    - Comma-separated: "00,01,02,03" or "0x00,0x01,0x02,0x03"
    - Continuous: "00010203"

    Args:
        hex_string: String containing hexadecimal values

    Returns:
        Binary data as bytes object

    Raises:
        ValueError: If the hex string contains invalid characters or has odd length
    """
    if not hex_string.strip():
        raise ValueError("Empty hexadecimal string provided")

    # Clean the hex string by removing common formatting
    cleaned_hex = _clean_hex_string(hex_string)

    # Validate the cleaned hex string
    _validate_hex_string(cleaned_hex)

    # Convert to binary data
    return _hex_string_to_bytes(cleaned_hex)


def _clean_hex_string(hex_string: str) -> str:
    """
    Remove formatting characters from hex string.

    Args:
        hex_string: Raw hex string with potential formatting

    Returns:
        Cleaned hex string containing only hex digits
    """
    # Remove 0x prefixes, commas, spaces, and newlines
    cleaned = re.sub(r"0x|,|\s+", "", hex_string, flags=re.IGNORECASE)
    return cleaned.lower()


def _validate_hex_string(hex_string: str) -> None:
    """
    Validate that the hex string contains only valid hex characters.

    Args:
        hex_string: Cleaned hex string to validate

    Raises:
        ValueError: If string contains invalid characters or has odd length
    """
    if not re.match(r"^[0-9a-f]*$", hex_string):
        raise ValueError("Invalid hexadecimal characters found in input")

    if len(hex_string) % HEX_BYTE_LENGTH != 0:
        raise ValueError(
            f"Hex string length must be even (got {len(hex_string)} characters)"
        )


def _hex_string_to_bytes(hex_string: str) -> bytes:
    """
    Convert validated hex string to bytes.

    Args:
        hex_string: Validated hex string

    Returns:
        Binary data as bytes object
    """
    hex_pairs = [
        hex_string[i : i + HEX_BYTE_LENGTH]
        for i in range(0, len(hex_string), HEX_BYTE_LENGTH)
    ]

    return bytes([int(pair, 16) for pair in hex_pairs])


def save_binary_to_file(binary_data: bytes, output_path: str) -> None:
    """
    Save binary data to a file.

    Args:
        binary_data: Binary data to save
        output_path: Path to the output file

    Raises:
        IOError: If there's an error writing to the file
    """
    try:
        with open(output_path, "wb") as f:
            f.write(binary_data)
        print(f"Successfully converted {len(binary_data)} bytes to {output_path}")
    except IOError as e:
        print(f"Error writing to file {output_path}: {e}", file=sys.stderr)
        raise


def load_hex_from_file(file_path: str) -> str:
    """
    Load hexadecimal string from a file.

    Args:
        file_path: Path to the input file

    Returns:
        Contents of the file as a string

    Raises:
        FileNotFoundError: If the input file doesn't exist
        IOError: If there's an error reading the file
    """
    if not Path(file_path).exists():
        raise FileNotFoundError(f"Input file not found: {file_path}")

    try:
        with open(file_path, "r", encoding="utf-8") as f:
            return f.read().strip()
    except IOError as e:
        print(f"Error reading file {file_path}: {e}", file=sys.stderr)
        raise


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments.

    Returns:
        Parsed command line arguments
    """
    parser = argparse.ArgumentParser(
        description="Convert hexadecimal strings to binary data",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Example input formats:
  Space-separated:  00 01 02 03
  Comma-separated:  00,01,02,03
  0x-prefixed:      0x00,0x01,0x02,0x03
  Continuous:       00010203""",
    )

    parser.add_argument("input_file", help="Input file containing hexadecimal values")
    parser.add_argument(
        "-o",
        "--output",
        default=DEFAULT_OUTPUT_FILE,
        help=f"Output binary file (default: {DEFAULT_OUTPUT_FILE})",
    )
    parser.add_argument(
        "--print-info",
        action="store_true",
        help="Print additional information about the conversion",
    )

    return parser.parse_args()


def main() -> None:
    """
    Main entry point of the application.
    """
    args = parse_arguments()

    try:
        # Load hex string from input file
        hex_string = load_hex_from_file(args.input_file)

        if args.print_info:
            print(f"Input file: {args.input_file}")
            print(f"Hex string length: {len(hex_string)} characters")

        # Convert hex to binary
        binary_data = hex_to_binary(hex_string)

        # Save binary data to output file
        save_binary_to_file(binary_data, args.output)

        if args.print_info:
            print(f"Output file: {args.output}")
            print(f"Binary data size: {len(binary_data)} bytes")

    except (FileNotFoundError, ValueError, IOError) as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
