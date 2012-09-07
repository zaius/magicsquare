#include "conf.h"
#include "led.h"

COLOR colors[4];

void led_init() {
  // Default LEDs to off
  // Make sure to set this before setting to output, as if all LEDs are on, it
  // can dip the power enough to cause a reset.
  PORTB = 0;
  PORTC = 0;

  // Set LEDs to output
  DDRB |= LED1_MASK | LED2_MASK;
  DDRC |= LED3_MASK | LED4_MASK;
}

void set_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
  colors[index] = (COLOR){ red, green, blue };
}

// TODO: work out why it doesn't work for LEDS
// volatile uint8_t* LEDS[] = { &LED1, &LED2, &LED3, &LED4 };
const uint8_t LED_MASKS[] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
const uint8_t REDS[] = { RED1, RED2, RED3, RED4 };
const uint8_t GREENS[] = { GREEN1, GREEN2, GREEN3, GREEN4 };
const uint8_t BLUES[] = { BLUE1, BLUE2, BLUE3, BLUE4 };

// We do a 255 length cycle of all the LEDs.
// A better version would space the on / off sections of the cycle out a bit
// better. This might be easy to do with some modulus trickery.
uint8_t cycle = 0;
void led_timer() {
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t color = 0;
    if (cycle < colors[i].red) {
      color |= _BV(REDS[i]);
    }
    if (cycle > colors[i].green) {
      color |= _BV(GREENS[i]);
    }
    if (cycle > colors[i].blue) {
      color |= _BV(BLUES[i]);
    }

    // Hackety hack. Should really be:
    // _set_with_mask(*LEDS[i], LED_MASKS[i], color);
    volatile uint8_t* led = i > 1 ? &PORTC : &PORTB;
    _set_with_mask(*led, LED_MASKS[i], color);
  }

  cycle++;
}
