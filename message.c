#include "conf.h"

#include "message.h"
#include "network.h"
#include "slip.h"


void message_encode(MESSAGE* msg) {
  // TODO: make slip use malloc and return something of the proper size
  uint8_t packet[MAX_PACKET_SIZE];
  uint8_t length = slip_encode(packet, MAX_PACKET_SIZE, (uint8_t*)msg, 4);
  // Should the slip_encode happen inside network_send?
  network_send(packet, length);
}

void message_decode(uint8_t* data, uint8_t data_length) {
  if (data_length != 4) { return; }
  MESSAGE* msg = (MESSAGE*)data;
  if (msg->destination != GROUP_INDEX) { return; }
  message_receive(msg);
}
