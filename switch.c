#include "conf.h"

#include "switch.h"
#include "network.h"

uint8_t switch_state = 0;

// Initialise the external interrupt
void switch_init() {
  // In order to use only one interrupt pin for all 4 switches, we do a bit
  // of hackery:
  //  * All switches are wired from their pin to INT1
  //  * INT1 is initially set to input, and pulled high
  //  * Switches are set to output, and set low
  //  * When the switch gets pressed, the interrupt fires
  //  * Switch pins are set to input, and pulled high
  //  * INT1 is set to output, and set low
  //  * Switches are read, any that are low have been pressed
  //  * Compare to the existing state of buttons to see which has changed
  //
  // Idea from here:
  // http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=80891
  // This could be solved by upgrading to the ATMega88 which has an interrupt
  // for every input pin.


  // MCU Control Register – MCUCR
  // Bit 1, 0 – ISC11, ISC10: Interrupt Sense Control 1 Bit 1 and Bit 0
  // ISC11  ISC10  Description
  // 0      0      The low level of INT1 generates an interrupt request
  // 0      1      Any logical change on INT1 generates an interrupt request
  // 1      0      The falling edge of INT1 generates an interrupt request
  // 1      1      The rising edge of INT1 generates an interrupt request

  // Trigger int1 on any change
  MCUCR |= _BV(ISC10);

  // General Interrupt Control Register – GICR
  // Bit 7 – INT1: External Interrupt Request 1 Enable
  GICR |= _BV(INT1);

  switches_to_output();
}

void switches_to_input() {
  // Switches to input
  DDRD &= ~SWITCHES;

  // Interrupt to output
  DDRD |= _BV(PD3);

  // Pull switch pins high
  PORTD |= SWITCHES;

  // Set interrupt pin low
  PORTD &= ~_BV(PD3);
}
void switches_to_output() {
  DDRD |= SWITCHES;
  DDRD &= ~_BV(PD3);
  PORTD &= ~SWITCHES;
  PORTD |= _BV(PD3);
}


// Be sure to call with a pointer to an input port such as &PINB or similar
// TODO: switch to timer debouncing, so we're not blocking on the network.
uint8_t debounce(volatile uint8_t *port, uint8_t pin) {
  uint8_t value, history = 0xaa;

  // History stores the last 8 states of the switch. When they're all
  // identical, we assume the bouncing has settled and return
  do {
    // Bitshift back so value is either 1 or 0.
    value = (*port & _BV(pin)) >> pin;
    history = (history << 1) | value;

    _delay_ms(5);
  } while (0 != history && 0xff != history);

  return !!history;
}


ISR(INT1_vect) {
  uint8_t value, switch_index, new_switch_state;
  volatile uint8_t* led;
  uint8_t mask, red, green, blue;

  switches_to_input();

  // Read which switch is active
  new_switch_state = PIND | SWITCHES;
  // find which has changed
  // TODO: this won't work - have to convert back from a BV to the index of
  // whichever bit is set
  switch_index = switch_state ^ new_switch_state;
  // NOTE: this is an index for the array. Actual pin indexes are +4.. maybe
  // there's a nicer way to show that
  switch_index = 1;
  // TODO: handle multiple switches changing at the same time. Seems
  // impossible, but with debouncing it could happen.

  // Debounce the switch used to call the interrupt
  value = debounce(&PIND, switch_index + 4);

  // Make the if statement below readable
  // TODO: THESE AREN'T WORKING. SHIT. All set to 0.
  led = &PORTB; //LEDS[switch_index];
  mask = 0x38; // LED_MASKS[switch_index];
  red = _BV(5); // _BV(REDS[switch_index]);
  green = _BV(4); // _BV(GREENS[switch_index]);
  blue = _BV(3); // _BV(BLUES[switch_index]);

  // Cycle through red, green, blue, off
  // TODO: set up a macro for this operation.
  if (red & *led) {
    *led = (*led & ~mask) | green;
  } else if (green & *led) {
    *led = (*led & ~mask) | blue;
  } else if (blue & *led) {
    *led &= ~mask;
  } else {
    *led = (*led & ~mask) | red;
  }


  switches_to_output();

  // Flicking the switches seems to re-trigger the interrupt, and for some
  // reason (maybe line capacitance?) it retriggers after this ISR has finished
  // so it continuously refires wihtout a delay here.
  _delay_ms(1);

  // Clear any button interrupts that might have happened since
  GIFR = _BV(INTF1);
}
