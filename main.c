#include "conf.h"
#include "network.h"
#include "switch.h"

uint8_t state = 0xaa;

int main(void) {
  // Disable the analog comparitor before enabling interrupts, otherwise a
  // floating analog pin will fire interrupts.
  ACSR = _BV(ACD);
  ADCSRA = 0;

  init();
  network_init();

  /*
          // TIMSK - Timer/Counter Interrupt Mask Register
        // Bit 1 - TOIE0: Timer/Counter0 Overflow Interrupt Enable
        TIMSK = _BV(TOIE0);

        // TCCR0 - Timer/Counter0 Control Register
        // Bits 2,1,0 - CS12, CS11, CS10: Clock Select1
        // Setting CS12 and CS10 gives us a prescaler of 1024 clock cycles
        TCCR0 = _BV(CS02) | _BV(CS00);
        */

  // Enable interrupts
  sei();

  // deactivate pullup for SDA
  DDRC = 0x00;
  PORTC = 0x00;
  DDRC = 0xff;
  // PORTC &= ~(_BV(4) | _BV(5));
  while (TRUE) {
    _delay_ms(50);
    uint8_t new_state = PIND;
    if (state != new_state) { button_press(); }
    state = new_state;
  }
}

/*
ISR(TIMER0_OVF_vect) {
    uint8_t new_state = PIND;
    if (state != new_state) { button_press(); }
    state = new_state;
}
*/
