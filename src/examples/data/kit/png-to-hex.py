from PIL import Image
import numpy as np

def png_to_hex(image_path):
    """
    Convert PNG image to hexadecimal where each byte represents 8 binary pixels (0=black, 1=white)
    Returns a list of hexadecimal values
    """
    # Open and convert image to black and white
    img = Image.open(image_path).convert('L')
    
    # Convert to numpy array and threshold to binary (0 and 1)
    # Pixels with value > 127 (mid gray) become 1 (white), others become 0 (black)
    binary_array = np.array(img) > 127
    
    # Get dimensions
    height, width = binary_array.shape
    
    # Calculate padding needed to make width divisible by 8
    padding_needed = (8 - (width % 8)) % 8
    if padding_needed:
        # Pad with zeros (black) on the right
        binary_array = np.pad(binary_array, ((0, 0), (0, padding_needed)), mode='constant')
        width += padding_needed
    
    # Initialize list to store hex values
    hex_values = []
    
    # Process image row by row
    for row in range(height):
        # Process each group of 8 pixels
        for col in range(0, width, 8):
            # Get 8 pixels
            eight_pixels = binary_array[row, col:col+8]
            
            # Convert 8 pixels to a single byte
            byte = 0
            for i, pixel in enumerate(eight_pixels):
                if pixel:  # if white (1)
                    byte |= (1 << (7-i))  # Set bit from left to right
            
            # Convert byte to hexadecimal and append to list
            hex_values.append(format(byte, '02x'))
    
    return hex_values

def main():
    # Example usage
    image_path = 'input.png'
    try:
        hex_values = png_to_hex(image_path)
        
        # Print results
        print(f"Hexadecimal representation:")
        print(' '.join(hex_values))
        
        # Optional: Save to file
        with open('output.hex', 'w') as f:
            f.write(' '.join(hex_values))
            
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
