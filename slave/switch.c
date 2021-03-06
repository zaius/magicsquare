#include "conf.h"

#include "switch.h"
#include "network.h"
#include "message.h"
#include "led.h"


// Initialise the external interrupt
void switch_init() {
  // Switches to input
  DDRD &= ~SWITCHES_MASK;
  // Switches have hardware pulldowns - deactivate internal pullup
  PORTD &= ~SWITCHES_MASK;
}


uint8_t switches_state = 0;
uint8_t switches_history[4] = {1, 2, 3, 4};

void switch_timer() {
  // Rotate in the new status of the switches
  for (uint8_t i = 3; i > 0; i--) {
    switches_history[i] = switches_history[i - 1];
  }
  switches_history[0] = PIND & SWITCHES_MASK;

  // Wait for next timer interrupt if history values aren't all equal
  for (uint8_t i = 0; i < 3; i++) {
    if (switches_history[i] != switches_history[i+1]) { return; }
  }

  // Send a message for each changed button. This has the added benefit of
  // doing nothing if it was just noise.
  uint8_t changed_switches = switches_state ^ switches_history[0];
  switches_state = switches_history[0];

  // NOTE: Switches are PD4-PD7. Pin order is reversed compared to switch order.
  // i.e. Pin 7 = switch 0, pin 6 = switch 1, etc.
  for (uint8_t pin = 4, switch_index = 3; pin < 8; pin++, switch_index--) {
    if (changed_switches & _BV(pin)) {
      uint8_t switch_bool = !!(switches_state & _BV(pin));

      // Only change on push down
      if (!switch_bool) { continue; }

      // MESSAGE_SWITCH_CHANGE switch_change = { switch_index, switch_bool };
      // message_send(&switch_change);
      cycle_rgb(switch_index);
    }
  }
}
