#include "conf.h"
#include "slip.h"

// Encode an array of data using SLIP encoding
uint8_t slip_encode(uint8_t * dest, uint8_t dest_size, uint8_t * source, uint8_t source_size) {
  uint8_t c, source_index, dest_index = 0;

  // Zero sized buffers can break things
  if (dest_size < 1 || dest == NULL || source == NULL) return 0;

  dest[dest_index] = SLIP_END;

  for (source_index = 0; source_index < source_size; source_index++) {
    c = source[source_index];

    if (c == SLIP_END) {
      if (++dest_index < dest_size) dest[dest_index] = SLIP_ESC;
      if (++dest_index < dest_size) dest[dest_index] = SLIP_ESC_END;
    }
    else if (c == SLIP_ESC) {
      if (++dest_index < dest_size) dest[dest_index] = SLIP_ESC;
      if (++dest_index < dest_size) dest[dest_index] = SLIP_ESC_ESC;
    }
    else {
      if (++dest_index < dest_size) dest[dest_index] = c;
    }
  }

  if (++dest_index < dest_size) dest[dest_index] = SLIP_END;

  return ++dest_index;
}


uint8_t packet[MAX_PACKET_SIZE], packet_length = 0, previous_byte;
ISR(USART_RXC_vect) {
  uint8_t c = UDR;

  if (c == SLIP_END) {
    // End marker found, we have a complete packet.

    // Ignore 0 length packets - these should happen at the start of every data
    // packet anyway to flush out the noise
    if (packet_length == 0) return;

    // some_stored_callback(data, packet_length);
    // network_send("Received packet", 20);
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

