#include "conf.h"

#include "switch.h"
#include "network.h"

#define MASTER 1

void master_init(void);
void slave_init(void);

uint8_t group_index = 1;

// TODO: Is there a way to do this with the preprocessor?
// see http://www.gamedev.net/topic/260159-single-cc-macro-to-create-enum-and-corresponding-char-array/
// TODO: check if ports can be changed to const
volatile uint8_t* LEDS[4] = { &LED1, &LED2, &LED3, &LED4 };
const uint8_t LED_MASKS[4] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
const uint8_t REDS[4] = { RED1, RED2, RED3, RED4 };
const uint8_t GREENS[4] = { GREEN1, GREEN2, GREEN3, GREEN4 };
const uint8_t BLUES[4] = { BLUE1, BLUE2, BLUE3, BLUE4 };


int main(void) {
  // Disable the analog comparitor before enabling interrupts, otherwise a
  // floating analog pin will fire interrupts.
  ACSR = _BV(ACD);
  ADCSRA = 0;

  network_init();

#ifdef MASTER
  master_init();
#else
  slave_init();
#endif

  // Enable interrupts
  sei();

  while (TRUE) {
    // wait for interrupts
    // TODO: sleep mode
  }
}


void master_init(void) {
  // Unused
  DDRB = 0xff;
  DDRC = 0xff;
}

void slave_init(void) {
  switch_init();

  // Set LEDs to output
  DDRB = LED1_MASK | LED2_MASK;
  DDRC = LED3_MASK | LED4_MASK;
}
