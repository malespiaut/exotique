#!/usr/bin/env python3

def hex_to_binary(hex_string):
    """Convert a string of comma-separated hex values to binary data."""
    # Remove common formatting characters and split the string
    clean_hex = hex_string.replace('0x', '').replace(',', '').replace(' ', '')
    
    # Handle both upper and lower case hex values
    # Split into pairs of characters (bytes)
    hex_pairs = [clean_hex[i:i+2] for i in range(0, len(clean_hex), 2)]
    
    # Convert hex pairs to integers and then to bytes
    return bytes([int(pair, 16) for pair in hex_pairs])

def main():
    import sys
    
    if len(sys.argv) != 3:
        print("Usage:")
        print(f"{sys.argv[0]} <input_file> <output_file>")
        print("Input file should contain hex values (with or without 0x prefix)")
        print("Example input formats:")
        print("  0x00, 0x01, 0x02, 0x03")
        print("  00 01 02 03")
        print("  00,01,02,03")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    try:
        with open(input_file, 'r') as f:
            hex_string = f.read().strip()
        
        binary_data = hex_to_binary(hex_string)
        
        with open(output_file, 'wb') as f:
            f.write(binary_data)
            
        print(f"Successfully converted {len(binary_data)} bytes to {output_file}")
        
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()
