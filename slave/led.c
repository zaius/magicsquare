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

// We do a 255 length cycle of all the LEDs.
// A better version would space the on / off sections of the cycle out a bit
// better. This might be easy to do with some modulus trickery.

uint8_t duty_cycle = 0;
void led_timer() {
  uint8_t LED_MASKS[4] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
  uint8_t REDS[4] = { RED1, RED2, RED3, RED4 };
  uint8_t GREENS[4] = { GREEN1, GREEN2, GREEN3, GREEN4 };
  uint8_t BLUES[4] = { BLUE1, BLUE2, BLUE3, BLUE4 };

  for (uint8_t i = 0; i < 4; i++) {
    uint8_t color = 0;

    if (duty_cycle < colors[i].red) {
      color |= _BV(REDS[i]);
    }
    if (duty_cycle > colors[i].green) {
      color |= _BV(GREENS[i]);
    }
    if (duty_cycle > colors[i].blue) {
      color |= _BV(BLUES[i]);
    }

    // Hackety hack. Should really be:
    // _set_with_mask(*LEDS[i], LED_MASKS[i], color);
    volatile uint8_t* led = i > 1 ? &PORTC : &PORTB;
    _set_with_mask(*led, LED_MASKS[i], color);
  }

  duty_cycle++;
}

uint8_t current_colors[4] = { 0, 0, 0, 0 };
void cycle_rgb(uint8_t switch_index) {
  uint8_t LED_MASKS[4] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
  uint8_t REDS[4] = { RED1, RED2, RED3, RED4 };
  uint8_t GREENS[4] = { GREEN1, GREEN2, GREEN3, GREEN4 };
  uint8_t BLUES[4] = { BLUE1, BLUE2, BLUE3, BLUE4 };

  uint8_t current_color = current_colors[switch_index] % 3 + 1;
  current_colors[switch_index] = current_color;
  uint8_t color = 0;
  if (current_color == 1) {
    color = _BV(REDS[switch_index]);
  } else if (current_color == 2) {
    color = _BV(GREENS[switch_index]);
  } else {
    color = _BV(BLUES[switch_index]);
  }

  volatile uint8_t* led = switch_index > 1 ? &PORTC : &PORTB;
  _set_with_mask(*led, LED_MASKS[switch_index], color);
}
