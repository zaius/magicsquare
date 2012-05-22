#include <avr/io.h>
#include <avr/iom8.h>
#include <avr/interrupt.h>

#include "conf.h"

#include "switch.h"
#include "network.h"

#define MASTER 1

void master_init(void);
void slave_init(void);

uint8_t group_index = 1;

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
  }
}


void master_init(void) {
  DDRD = 0x00;
  DDRB = 0xff;
}

void slave_init(void) {
  switch_init();

  DDRD = 0x00;
  DDRB = 0xff;
}
