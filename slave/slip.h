// SLIP special characters

// Character used to signal the start or end of a frame
#define SLIP_END            0xC0
// Character used to escape the use of special characters in the data
#define SLIP_ESC            0xDB
// Character used to replace a SLIP_END character in the data
#define SLIP_ESC_END        0xDC
// Character used to replace a SLIP_ESC character in the data
#define SLIP_ESC_ESC        0xDD

uint8_t slip_encode(uint8_t * dest, uint8_t dest_size, uint8_t * source, uint8_t source_size);
