// TODO:
// * don't transmit when we're receiving? would need a timeout...
// * handle collisions
// * handle TXEN/RXEN pins
// * add checksum
// * Fix multiple transmissions
// * fix overwriting buffer when receiving / sending at the same itme
#include "conf.h"

#include <avr/eeprom.h>
#include "network.h"

#define TXEN_PORT PORTD
#define TXEN_PIN 3
#define RXEN_PORT PORTD
#define RXEN_PIN 2

// From datasheet: BAUD = FREQUENCY / 16*(UBRR+1)
#define BAUD_PRESCALE (((F_CPU / (BAUD * 16UL))) - 1)

uint16_t hardware_address = 0;

void network_init() {
  // Load the hardware address from EEPROM
  hardware_address = eeprom_read_word((const uint16_t *) 0);

  // Initialise the on-chip UART
  // UBRR - UART Baud Rate Register
  // Initialise the baud rate controller
  UBRRH = BAUD_PRESCALE >> 8;
  UBRRL = BAUD_PRESCALE;

  // UCR - UART Control Register.
  // Bit 7 - RXCIE: Receive interrupt enable
  // Bit 6 - TXCIE: Transmit interrupt enable
  // Bit 5 - UDRIE: USART Data Register Empty Interrupt Enable
  // Bit 4 - RXEN: Receiver enable
  // Bit 3 - TXEN: Transmitter enable
  // Transmit interrupts are only activated once a send happens.
  UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);

  // Set the size to be 8 bits
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);

  // Pin 0: UART RXD
  // Pin 1: UART TXD
  // Pin 2: NOT RS485 Receive enable
  // Pin 3: RS485 Transmit enable
  DDRD &= ~_BV(0);
  DDRD |= _BV(1) & _BV(2) & _BV(3);
  // Enable receive, disable transmit
  PORTD &= ~_BV(2) & ~_BV(3);
}



uint8_t buffer_putc(BUFFER* buffer, uint8_t in) {
  // Queue full
  if (buffer->length >= MAX_PACKET_SIZE) { return 0; }

  uint8_t write_index = (buffer->read_index + buffer->length) % MAX_PACKET_SIZE;
  buffer->data[write_index] = in;
  buffer->length++;
  return 1;
}
uint8_t buffer_getc(BUFFER* buffer, uint8_t* out) {
  // Queue empty
  if (buffer->length == 0) { return 0; }

  *out = buffer->data[buffer->read_index];
  buffer->read_index = (buffer->read_index + 1) % MAX_PACKET_SIZE;
  buffer->length--;
  return 1;
}

// Initialize to zero
BUFFER rx_buffer = {}, tx_buffer = {};
void network_send(uint8_t* new_data, uint8_t data_length) {
  uint8_t transmitting = tx_buffer.length > 0;

  // Queue the data up
  for (uint8_t i = 0; i < data_length; i++) {
    buffer_putc(&tx_buffer, new_data[i]);
  }

  uint8_t out;
  if (!transmitting && buffer_getc(&tx_buffer, &out)) {
    // Starting a new transmission. Enable rs485 transmit. Disable receiving.
    TXEN_PORT |= _BV(TXEN_PIN);
    // TODO: would be good to actually check the incoming byte, rather than
    // just ignoring it. Then we could check for collisions. What's the best
    // way to do this though? Maybe disable interrupts? Or have a flag in the
    // receive interrupt?
    RXEN_PORT |= _BV(RXEN_PIN);
    // Fire interrupt as soon as the bit is shifted out of UDR, not when its
    // done transmitting.
    UCSRB |= _BV(UDRIE);
    // Start the sending
    UDR = out;
    // TODO: there is an edgecase here where the UDRE interrupt has put
    // something in UDR and then send is called
  }
}

// UDR is double buffered. UDRE fires when UDR is empty and ready for a new
// character. TXC fires when the character has finished sending.
ISR(USART_UDRE_vect) {
  uint8_t out;

  if (buffer_getc(&rx_buffer, &out)) {
    UDR = out;
  } else {
    // We're at the end of the buffer. Wait until the transmission is actually
    // finished
    UCSRB |= _BV(TXC);
    // If UDR isn't set, UDRE will refire
    UCSRB &= ~_BV(UDRIE);
  }
}

ISR(USART_TXC_vect) {
  // Disable this interrupt
  UCSRB &= ~_BV(TXCIE);

  // A new message could have hit the buffer between UDRE and TXC. Just need
  // to ignore this interrupt and wait for the next UDRE.
  if (tx_buffer.length > 0) { return; }

  // Finish the transmission
	TXEN_PORT &= ~_BV(TXEN_PIN);
	RXEN_PORT &= ~_BV(RXEN_PIN);
}
