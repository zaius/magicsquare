# Fuses:
# http://www.engbedded.com/fusecalc/
# Preserve EEPROM memory through the Chip Erase cycle;
# External crystal, start up time 1K CK + 64ms
# SPI enabled
FUSES=-U lfuse:w:0xcf:m -U hfuse:w:0xd1:m

PROJECT=magicsquare
MMCU=atmega8

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-mmcu=$(MMCU) -Wall -O2 -std=c99
SOURCES=main.c network.c switch.c slip.c message.c led.c

binary:
	$(CC) $(CFLAGS) -I./ -o build/build.out $(SOURCES)

hex: binary
	$(OBJCOPY) -j .text -O ihex build/build.out build/build.hex



fuses:
	avrdude -p $(MMCU) -c stk500v2 -P /dev/cu.PL2303-* $(FUSES)
program:
	avrdude -p $(MMCU) -c stk500v2 -P /dev/cu.PL2303-* -U flash:w:build/build.hex
eeprom:
	# Generate a random hardware address
	ADDRESS=`hexdump -n 2 -v -e '"%04x"' /dev/random`
	avrdide -p $(MMCU) -c stk500v2 -P /dev/cu.PL2303-* -U eeprom:w:0x$(ADDRESS):m


clean:
	rm -f build/*
