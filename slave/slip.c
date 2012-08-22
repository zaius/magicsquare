#include "conf.h"
#include "slip.h"

#include "message.h"
#include <stdlib.h> // malloc

// Encode an array of data using SLIP encoding
uint8_t slip_encode(uint8_t ** dest_pointer, uint8_t * source, uint8_t source_size) {
  // Loop through once to count the number of bytes that are going to be added
  // Start with 2 extra bytes, for the first and last SLIP_END
  uint8_t dest_size = source_size + 2;
  for (uint8_t i = 0; i < source_size; i++) {
    if (source[i] == SLIP_END || source[i] == SLIP_ESC) { dest_size++; }
  }
  uint8_t* dest = malloc(dest_size * sizeof(uint8_t));
  dest_pointer = &dest;

  uint8_t dest_index = 0;
  // First byte is always an slip end, to clear line noise.
  dest[dest_index++] = SLIP_END;

  for (uint8_t i = 0; i < source_size; i++) {
    uint8_t c = source[i];

    if (c == SLIP_END) {
      dest[dest_index++] = SLIP_ESC;
      dest[dest_index++] = SLIP_ESC_END;
    }
    else if (c == SLIP_ESC) {
      dest[dest_index++] = SLIP_ESC;
      dest[dest_index++] = SLIP_ESC_ESC;
    }
    else {
      dest[dest_index++] = c;
    }
  }

  dest[dest_index++] = SLIP_END;

  // Should be the same as dest_size
  return dest_index;
}


uint8_t packet[MAX_PACKET_SIZE], packet_length = 0, previous_byte;
ISR(USART_RXC_vect) {
  uint8_t c = UDR;

  if (c == SLIP_END) {
    // End marker found, we have a complete packet.

    // Ignore 0 length packets - these should happen at the start of every data
    // packet anyway to flush out the noise
    if (packet_length == 0) return;

    message_receive(packet, packet_length);
    previous_byte = packet_length = 0;
    return;
  } else if (previous_byte == SLIP_ESC) {
    // Previous read byte was an escape byte, so this byte will be
    // interpreted differently from others.
    if (c == SLIP_ESC_END) { c = SLIP_END; }
    if (c == SLIP_ESC_ESC) { c = SLIP_ESC; }
  }

  previous_byte = c;

  // Don't store an escape - just wait for the next character
  if (c == SLIP_ESC) { return; }

  // Ignore data that would overflow
  if (packet_length >= MAX_PACKET_SIZE) { return; }

  packet[packet_length++] = c;
}

