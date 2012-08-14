#include "conf.h"

#include "switch.h"
#include "network.h"
#include "message.h"

uint8_t switch_state = 0;

// Initialise the external interrupt
void switch_init() {
  // Switches to input
  DDRD &= ~SWITCHES;
  // Switches have hardware pulldowns - deactivate internal pullup
  PORTD &= ~SWITCHES;
}


// Be sure to call with a pointer to an input port such as &PINB or similar
// TODO: switch to timer debouncing, so we're not blocking on the network.
// TODO: take a mask as an input, not as a comparison after.
uint8_t debounce_port(volatile uint8_t *port) {
  uint8_t i, equal;
  uint8_t history[4] = {1, 2, 3, 4};

  uint8_t last = 0xaa, current = 0xbb;
  while (last != current) {
    last = current;
    current = *port;
    _delay_ms(20);
  }
  return current;

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


uint8_t network_count = 0;

void button_press() {
  // Read which switch is active. Invert because the active switch will be 0.
  uint8_t new_switch_state = ~debounce_port(&PIND) & SWITCHES;
  uint8_t changed_switches = switch_state ^ new_switch_state;
  switch_state = new_switch_state;

  // Don't change when it's just noise
  if (changed_switches) {
    // TODO: handle multiple switches changing at the same time. Seems
    // impossible, but with debouncing it could happen.
    // NOTE: this is an index for the array. Actual pin indexes are +4.. maybe
    // there's a nicer way to show that
    uint8_t switch_index = find_lowest_bit_set(changed_switches) - 4;

    // yuck. sorry.
    uint8_t value = !!(new_switch_state & _BV(switch_index + 4));

    if (value) {
      MESSAGE msg = {0, GROUP_INDEX, switch_index, 0};
      // Fake receiving this message instead of actually sending
      message_receive(&msg);
    }
    // Don't send for now
    // message_encode(&msg);
  }

  // Flicking the switches seems to re-trigger the interrupt, and for some
  // reason (maybe line capacitance?) it retriggers after this ISR has finished
  // so it continuously refires wihtout a delay here.
  _delay_ms(1);
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
