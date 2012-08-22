// Look in /usr/local/CrossPack-AVR-20120217/avr/include/avr/ for headers
#include <avr/io.h>
#include <avr/iom8.h>
#include <avr/interrupt.h>

#include <inttypes.h>

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

#define SWITCHES 0xf0

#define LED1 PORTB
#define LED1_MASK 0x07
#define BLUE1 0
#define GREEN1 1
#define RED1 2

#define LED2 PORTB
#define LED2_MASK 0x38
#define BLUE2 3
#define GREEN2 4
#define RED2 5

#define LED3 PORTC
#define LED3_MASK 0x07
#define BLUE3 0
#define GREEN3 1
#define RED3 2

#define LED4 PORTC
#define LED4_MASK 0xf8
#define BLUE4 3
#define GREEN4 4
#define RED4 5

#define MAX_PACKET_SIZE 128

#define F_CPU 7372800UL
#define BAUD 9600

// Has to come after definition of F_CPU
#include <util/delay.h>

#define _set_with_mask(dest, mask, in) dest = (dest & ~mask) | in
