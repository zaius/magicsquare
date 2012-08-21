#include "conf.h"
#include <avr/eeprom.h>

#include "message.h"
#include "led.h"
#include "network.h"
#include "slip.h"

// An array of function pointers for how to handle each message type. Their
// index in the same as their message type.
typedef void (*MESSAGE_HANDLER)(uint8_t*, uint8_t);
MESSAGE_HANDLER message_handlers[6] = {
  &message_reset_address,
  &message_ignore,
  &message_assign_address,
  &message_ignore,
  &message_calibration_mode,
  &message_set_color
};

void message_send(MESSAGE* msg) {
  // TODO: make slip use malloc and return something of the proper size
  uint8_t packet[MAX_PACKET_SIZE];
  uint8_t length = slip_encode(packet, MAX_PACKET_SIZE, (uint8_t*)msg, 4);
  // Should the slip_encode happen inside network_send?
  network_send(packet, length);
}

void message_decode(uint8_t* data, uint8_t data_length) {
  MESSAGE* msg = (MESSAGE*)data;
  // TODO: fix this to use hardware addresses
  if (msg->destination != group_index) { return; }

  // TODO: check checksum

  (*message_handlers[msg->message_type])(msg->data, msg->data_length);
}

void message_set_color(uint8_t* data, uint8_t data_length) {
  if (data_length != 4) { return; }
  colors[data[0]] = (COLOR) { data[0], data[1], data[2] };
}
void message_reset_address(uint8_t* data, uint8_t data_length) {
  if (data_length != 4) { return; }
  group_index = 255;
}
void message_assign_address(uint8_t* data, uint8_t data_length) {
  if (data_length != 1) { return; }
  group_index = data[0];
  // TODO: better way to keep track of eeprom addresses. Even defines will work.
  eeprom_write_byte((uint8_t *)2, data[0]);
}
void message_calibration_mode(uint8_t* data, uint8_t data_length) {
  if (data_length != 0) { return; }

  // Flash LEDs white then off
  for (uint8_t i = 0; i < 4; i++) {
    colors[i] = (COLOR) { 0xff, 0xff, 0xff };
  }
  _delay_ms(1000);
  for (uint8_t i = 0; i < 4; i++) {
    colors[i] = (COLOR) { 0, 0, 0 };
  }
}
void message_ignore(uint8_t* data, uint8_t data_length) {
}
