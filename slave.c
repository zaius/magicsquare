#include "conf.h"
#include "switch.h"
#include "message.h"

void init() {
  switch_init();

  // Set LEDs to output
  DDRB = LED1_MASK | LED2_MASK;
  DDRC = LED3_MASK | LED4_MASK;

  // All on port D to input with pull ups.
  // TODO: check whether this is actually nescessary.
  DDRD = 0x00;
  PORTD = 0xff;
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
  volatile uint8_t* led = msg->square_index > 2 ? &PORTC : &PORTB;

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
