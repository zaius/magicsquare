#include "conf.h"
#include <avr/sleep.h>

#include "network.h"
#include "switch.h"
#include "led.h"

void timer_init();

int main(void) {
  // Default to all inputs with pullups. This can avoid high current draw on
  // boot.
  DDRB = DDRC = DDRD = 0;
  PORTB = PORTC = PORTD = 0xff;

  // Disable the analog comparitor before enabling interrupts, otherwise a
  // floating analog pin will fire interrupts.
  ACSR = _BV(ACD);
  ADCSRA = 0;

  timer_init();
  switch_init();
  led_init();
  // network_init();

  // Enable interrupts
  sei();

  while (TRUE) {
    // Switch to idle sleep (the default, so no need to use set_sleep_mode)
    // TODO: check that USART and LEDs still run in idle sleep.
    // TODO: Find out cycles to go in / out of sleep and make sure it's worth it
    // sleep_mode();
  }
}


void timer_init() {
  // Use timer 2 for led / switch timing. Timer 0 only has overflow, and timer
  // 1 is 16 bit so adds some complexity.
  // TIMSK - Timer/Counter Interrupt Mask Register
  // Bit 7 - OCIE2: Timer/Counter2 Output Compare Match Interrupt Enable
  TIMSK = _BV(OCIE2);

  // TCCR2 - Timer / Counter 2 Control Register
  // Bits 2,1,0 - CS12, CS11, CS10: Clock Select1
  // CS21 means use a prescaler 8. i.e. the timer increments every 8 cpu cycles.
  TCCR2 = _BV(CS21);
  // OCR2 - Timer 2 Output Compare Register
  // Fire every 72 * 8 = 576 cycles.
  OCR2 = 72;

  // Kick off the timer 1 to use as a random number seed at some point. Don't
  // need to use it for any interrupts - just start counting.
  // TCCR1B - Timer/Counter 1 Control Register B
  // Bit 2:0 - CS12:0: Clock Select
  // CS10 means no prescaling
  TCCR1B = _BV(CS10);
}

// By moving the led cycle out of the main loop and into a timer, we ensure
// that the cycle time is consistent and doesn't change regardless of other
// code running (e.g. switch / network code).
//
// We want LEDs to run at around 50Hz, so flicker can't be noticed. This means
// a cycle time of 20ms, which is 147456 clock cycles at 7372800 Hz. There
// are 256 brightness increments per led cycle, which means 576 cpu cycles per
// increment.
//
// This few cycles might be cutting things kind of fine, especially with
// network code. This could be doubled by using a 14.74Mhz crystal, or by
// reducing the number of brightness levels down from 255. Any number could
// work, but it would need to be enough levels for seamless fading.
uint8_t switch_cycle = 0;
ISR(TIMER2_COMP_vect) {
  // led_timer();

  // Switch debouncing should run once every 2.5ms or so. This timer fires once
  // every 0.078125ms, so 32 cycles per switch timer.
  if (++switch_cycle > 32) {
    switch_timer();
    switch_cycle = 0;
  }
}
