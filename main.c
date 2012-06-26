#include "conf.h"
#include "network.h"

int main(void) {
  // Disable the analog comparitor before enabling interrupts, otherwise a
  // floating analog pin will fire interrupts.
  ACSR = _BV(ACD);
  ADCSRA = 0;

  init();
  network_init();

  // Enable interrupts
  sei();

  while (TRUE) {
    // wait for interrupts
    // TODO: sleep mode
  }
}
