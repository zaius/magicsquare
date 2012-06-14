#include "conf.h"
// Has to come after conf so F_CPU is defined
#include <util/delay.h>

#include "sleep.h"

// TODO: there might be a more standard way to deal with delays now
void msleep(uint16_t delay) {
  uint16_t i = 0;
  // 4 operations per cycle
  // max 65536 cycles
  // F_CPU / 1000 operations per ms

  for (i = 0; i < delay; i++) 
    _delay_loop_2(F_CPU / 1000 / 4);
}
