#include "conf.h"
#include "message.h"

void init() {
  // Unused
  DDRB = 0xff;
  DDRC = 0xff;
}

uint8_t color;
void message_receive(MESSAGE* msg) {
  // Re-using the same message because I'm lazy. The 'color' field is actually
  // the switch value. Cycle through colors on press, turn off on release.
  if (msg->color) { color = color % 3 + 1; }
  MESSAGE response = { 0, msg->source, msg->square_index, color };
  message_encode(&response);
}
