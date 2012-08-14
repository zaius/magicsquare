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

void message_receive(MESSAGE* msg) {
  // TODO: Get these back as an extern
  // TODO: work out why it doesn't work for LEDS
  // volatile uint8_t* LEDS[] = { &LED1, &LED2, &LED3, &LED4 };
  const uint8_t LED_MASKS[] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
  const uint8_t REDS[] = { RED1, RED2, RED3, RED4 };
  const uint8_t GREENS[] = { GREEN1, GREEN2, GREEN3, GREEN4 };
  const uint8_t BLUES[] = { BLUE1, BLUE2, BLUE3, BLUE4 };
  // Hackety hack.
  volatile uint8_t* led = msg->square_index > 1 ? &PORTC : &PORTB;

  uint8_t color;
  switch (msg->color) {
    case 1:
      color = _BV(REDS[msg->square_index]);
      break;
    case 2:
      color = _BV(GREENS[msg->square_index]);
      break;
    case 3:
      color = _BV(BLUES[msg->square_index]);
      break;
    default:
      color = 0;
  }

  _set_with_mask(*led, LED_MASKS[msg->square_index], color);
}
