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
  DDRD |= _BV(PD2);

  // Pull switch pins high
  PORTD |= SWITCHES;

  // Set interrupt pin low
  PORTD &= ~_BV(PD2);
}
void switches_to_output() {
  DDRD |= SWITCHES;
  DDRD &= ~_BV(PD2);
  PORTD &= ~SWITCHES;
  PORTD |= _BV(PD2);
}


// Be sure to call with a pointer to an input port such as &PINB or similar
uint8_t debounce(volatile uint8_t *port, uint8_t pin) {
  uint8_t value, previous = -1;

  // Get the initial value of the pin
  value = *port & _BV(pin);

  // Loop until the value present on the pin and the value
  // that was there 10 milliseconds ago are equal
  while (previous != value) {
    msleep(20);

    previous = value;
    value = *port & _BV(pin);
  }

  // Bitshift so the returned value is either 0 or 1
  return (value >> pin);
}


ISR(INT1_vect) {
  uint8_t value, switch_index, new_switch_state;
  volatile uint8_t* led;
  uint8_t mask, red, green, blue;

  switches_to_input();

  // Read which switch is active
  new_switch_state = PIND | SWITCHES;
  // find which has changed
  switch_index = switch_state ^ new_switch_state;
  // TODO: handle multiple switches changing at the same time. Seems
  // impossible, but with debouncing it could happen.

  // Debounce the switch used to call the interrupt
  value = debounce(&PIND, switch_index);

  // Make the if statement below readable
  led = LEDS[switch_index];
  mask = LED_MASKS[switch_index];
  red = REDS[switch_index];
  blue = BLUES[switch_index];
  green = GREENS[switch_index];

  // Cycle through red, green, blue, off
  if (!(*led | mask)) {
    *led |= mask | red;
  } else if (_BV(RED1) | LED1) {
    *led |= mask | green;
  } else if (_BV(RED1) | LED1) {
    *led |= mask | blue;
  } else if (_BV(RED1) | LED1) {
    *led &= ~mask;
  }


  switches_to_output();

  // Clear any button interrupts that might have happened since
  GIFR = _BV(INTF0);
}
