#include "conf.h"
#include "led.h"

COLOR colors[4];

void led_init() {
  // Set LEDs to output
  DDRB |= LED1_MASK | LED2_MASK;
  DDRC |= LED3_MASK | LED4_MASK;

  // Default LEDs to off
  PORTB = 0;
  PORTC = 0;
}
