#include "conf.h"
#include "network.h"

#include <avr/io.h>
#include <avr/interrupt.h>

void network_send_byte(void);

// Initialise the on-chip UART
void network_init() {
  // UBRR - UART Baud Rate Register
  // Initialise the baud rate controller
  // From datasheet: BAUD = FREQUENCY / 16*(UBRR+1)
  UBRRH = 0;
  UBRRL = F_CPU / 16 / BAUD - 1;

  // UCR - UART Control Register. Enable:
  // Bit 4 - RXEN: Receiver enable
  // Bit 3 - TXEN: Transmitter enable
  UCSRB = _BV(RXEN) | _BV(TXEN);

  // Set the size to be 8 bits
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
}


#define MAX_DATA_SIZE 256
uint8_t receive_data[MAX_DATA_SIZE];
uint16_t receive_data_length = 0;

ISR(USART_RXC_vect) {
  if (receive_data_length > MAX_DATA_SIZE) {
    // TODO: Handle overflow
  }

  // Have to read UDR to clear the interrupt
  receive_data[receive_data_length++] = UDR;

  // TODO: framing, do something with complete message
}


uint8_t *transmit_message;
uint8_t transmit_message_length = 0;
uint8_t transmit_index = 0;
void network_send_message(uint8_t *message, uint16_t length) {
  transmit_message = message;
  transmit_message_length = length;
  network_send_byte();
}

void network_send_byte() {
  if (transmit_index < transmit_message_length) {
    UDR = transmit_message[transmit_index++];
  } else {
    transmit_index = 0;
  }
}

ISR(USART_TXC_vect) {
  network_send_byte();
}
