// http://www.engbedded.com/fusecalc/
//
#include <avr/io.h>
#include <avr/iom32.h>

// TODO: pass this from the makefile
#define F_CPU 1000000UL  // 1 MHz
#include <util/delay.h>


int main(void) {
  uint8_t temp = 0x00;

  DDRD = 0x00;
  DDRB = 0xff;

  while (1) {
    PORTB = temp;

    if (bit_is_clear(PIND, 0)) temp++; // Count one down
    if (bit_is_clear(PIND, 1)) temp--; // Count one up
    if (bit_is_clear(PIND, 2)) temp = (temp >> 1) | (temp << 7); // Rotate right. ASM ror
    if (bit_is_clear(PIND, 3)) temp = temp << 1 | (temp >> 7); // Rotate left. ASM rol
    if (bit_is_clear(PIND, 4)) temp = ~temp; // Invert. ASM com
    if (bit_is_clear(PIND, 5)) temp = ~temp + 1; // Invert and add 1. ASM neg
    if (bit_is_clear(PIND, 6)) temp = (temp >> 4) + (temp << 4); // Swap nibbles; ASM swap

    _delay_ms(100);
  }
}
