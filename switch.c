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
  // http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=80891
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
uint8_t debounce_port(volatile uint8_t *port) {
  uint8_t i, equal;
  uint8_t history[4] = {1, 2, 3, 4};

  while (TRUE) {
    // Shift the array one right
    for (i = 1; i < 4; i++) { history[i] = history[i - 1]; }
    // Append the current value of the pins
    history[0] = *port;

    // Check that all 4 values are equal
    equal = TRUE;
    for (i = 0; i < 3; i++) {
      equal = equal && history[i] == history[i + 1];
    }
    if (equal) { return history[0]; }

    _delay_ms(5);
  }
}


ISR(INT1_vect) {
  uint8_t value, changed_switches, switch_index, new_switch_state;
  volatile uint8_t* led;
  uint8_t mask, red, green, blue;

  switches_to_input();

  // Read which switch is active. Invert because the active switch will be 0.
  new_switch_state = ~debounce_port(&PIND) & SWITCHES;
  changed_switches = switch_state ^ new_switch_state;
  switch_state = new_switch_state;

  // TODO: handle multiple switches changing at the same time. Seems
  // impossible, but with debouncing it could happen.
  // NOTE: this is an index for the array. Actual pin indexes are +4.. maybe
  // there's a nicer way to show that
  switch_index = find_lowest_bit_set(changed_switches) - 4;

  // yuck. sorry.
  value = !!(new_switch_state & _BV(switch_index + 4));

  // Make the if statement below readable
  // TODO: Get these back as an extern
  // TODO: check if ports can be changed to const
  // TODO: Is there a way to do this with the preprocessor?
  // see http://www.gamedev.net/topic/260159-single-cc-macro-to-create-enum-and-corresponding-char-array/
  // TODO: work out why it doesn't work for LEDS
  // volatile uint8_t* LEDS[] = { &LED1, &LED2, &LED3, &LED4 };
  const uint8_t LED_MASKS[] = { LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK };
  const uint8_t REDS[] = { RED1, RED2, RED3, RED4 };
  const uint8_t GREENS[] = { GREEN1, GREEN2, GREEN3, GREEN4 };
  const uint8_t BLUES[] = { BLUE1, BLUE2, BLUE3, BLUE4 };
  // Hackety hack.
  led = switch_index > 2 ? &PORTC : &PORTB;

  // led = LEDS[switch_index];
  mask = LED_MASKS[switch_index];
  red = _BV(REDS[switch_index]);
  green = _BV(GREENS[switch_index]);
  blue = _BV(BLUES[switch_index]);

  if (value) {
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
  }


  switches_to_output();

  // Flicking the switches seems to re-trigger the interrupt, and for some
  // reason (maybe line capacitance?) it retriggers after this ISR has finished
  // so it continuously refires wihtout a delay here.
  _delay_ms(1);

  // Clear any button interrupts that might have happened since
  GIFR = _BV(INTF1);
}

// This is effectively the inverse of _BV
uint8_t find_lowest_bit_set(uint8_t value) {
  uint8_t index = 0;
  if (0 == value) return -1;

  while (TRUE) {
    if (value & 1) return index;

    index++;
    value >>= 1;
  }
}
