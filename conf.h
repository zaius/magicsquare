#include <inttypes.h>

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

// TODO: pass this from the makefile
#define F_CPU 4000000UL  // 1 MHz
#define BAUD 9600

// The index of this button group. Would it be better to track an x/y coordinate?
// TODO: work out a better way to track this? Maybe an initial broadcast when
// switching on, or load / store in eeprom
extern uint8_t group_index;
