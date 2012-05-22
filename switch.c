#include "conf.h"

#include "switch.h"
#include "network.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h> // snprintf

// Initialise the external interrupt
void switch_init() {
  // MCU Control Register – MCUCR
  // Bit 1, 0 – ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0
  // ISC01  ISC00  Description
  // 0      0      The low level of INT0 generates an interrupt request.
  // 0      1      Any logical change on INT0 generates an interrupt request.
  // 1      0      The falling edge of INT0 generates an interrupt request.
  // 1      1      The rising edge of INT0 generates an interrupt request.

  // Trigger int0 and int1 on any change
  MCUCR = _BV(ISC10) | _BV(ISC00);

  // General Interrupt Control Register – GICR
  // Bit 6 – INT0: External Interrupt Request 0 Enable
  GICR = _BV(INT0) | _BV(INT1);
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


ISR(INT0_vect) {
  uint8_t value = FALSE;
  uint8_t message[32];
  uint8_t message_length;
  // TODO: Work out which one of the switches has changed
  uint8_t switch_index = 2;

  // Debounce the switch used to call the interrupt
  value = debounce(&PIND, PD2);

  // Send a message back to the master
  // TODO: move this into a struct, use it to generate message string. Ditch
  // the hard coded char limit.
  // TODO: work out a proper message layout
  message_length = sprintf(&message, "%d %d: %d", group_index, switch_index, value);
   
  network_send_message(message, message_length);

  // Clear any button interrupts that might have happened since
  GIFR = _BV(INTF0);
}
