#include "conf.h"
#include "message.h"
#include "led.h"
#include "network.h"
#include "slip.h"

#include <string.h> // memcpy
#include <stdlib.h> // free


void message_send(MESSAGE_SWITCH_CHANGE* switch_change) {
  uint8_t packet_length = sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_SWITCH_CHANGE);
  uint8_t packet[packet_length];

  // Add the header
  MESSAGE_HEADER header = {hardware_address, MASTER_ADDRESS, MESSAGE_TYPE_SWITCH_CHANGE };
  memcpy(packet, &header, sizeof(MESSAGE_HEADER));

  // Add the data
  memcpy(packet + sizeof(MESSAGE_HEADER), switch_change, sizeof(MESSAGE_SWITCH_CHANGE));

  // Slip encode
  uint8_t* encoded_packet;
  uint8_t encoded_packet_length = slip_encode(&encoded_packet, packet, packet_length);

  network_send(encoded_packet, encoded_packet_length);

  // This doesn't feel right. The free should be with the malloc.
  // Could fix by having a slip_encoded_length function which returns the size
  // that the destination needs to have, then go back to the old slip method.
  free(encoded_packet);
}

void message_receive(uint8_t* data, uint8_t data_length) {
  if (data_length < sizeof(MESSAGE_HEADER)) { return; }

  MESSAGE_HEADER* header = (MESSAGE_HEADER*)data;
  if (header->destination != hardware_address) { return; }
  if (header->message_type != MESSAGE_TYPE_COLOR_CHANGE) { return; }

  uint8_t offset = sizeof(MESSAGE_HEADER);
  if (data_length < offset + sizeof(MESSAGE_COLOR_CHANGE)) { return; }
  MESSAGE_COLOR_CHANGE* change = (MESSAGE_COLOR_CHANGE*)(data + offset);

  // Brain not working. There should be a way to assign to the struct directly.
  set_color(change->switch_index, change->red, change->green, change->blue);
}
