# AVR-GCC Makefile
PROJECT=magicsquare
SOURCES=main.c
MMCU=atmega32

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-mmcu=$(MMCU) -Wall -O2

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

program: $(PROJECT).hex
	avrdude -p $(MMCU) -c stk500v2 -P /dev/cu.usbserial -U flash:w:$(PROJECT).hex
clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex

