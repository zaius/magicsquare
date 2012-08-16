#include "conf.h"

#include "message.h"
#include "led.h"
#include "network.h"
#include "slip.h"


void message_send(MESSAGE* msg) {
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

void message_receive(MESSAGE* msg) {
  // TODO: Are structs treated as primitive types? If I assign a struct, is it
  // copied?
  COLOR color = { msg->red, msg->green, msg->blue };
  colors[msg->square_index] = color;
}
