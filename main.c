#include "conf.h"

#include "switch.h"
#include "network.h"

void master_init(void);
void slave_init(void);

uint8_t group_index = 1;


int main(void) {
  // Disable the analog comparitor before enabling interrupts, otherwise a
  // floating analog pin will fire interrupts.
  ACSR = _BV(ACD);
  ADCSRA = 0;

  // network_init();

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
  // Set LEDs to output
  DDRB = LED1_MASK | LED2_MASK;
  DDRC = LED3_MASK | LED4_MASK;

  // All on port D to input with pull ups.
  // TODO: check whether this is actually nescessary.
  DDRD = 0x00;
  PORTD = 0xff;

  switch_init();
}
