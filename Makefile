# Fuses:
# http://www.engbedded.com/fusecalc/
# Preserve EEPROM memory through the Chip Erase cycle;
# External crystal, start up time 1K CK + 64ms
# SPI enabled
FUSES=-U lfuse:w:0xcf:m -U hfuse:w:0xd1:m

PROJECT=magicsquare
MASTER_SOURCES=main.c network.c switch.c
SLAVE_SOURCES=main.c network.c switch.c
MMCU=atmega8

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-mmcu=$(MMCU) -Wall -O2

master.hex: master.out
	$(OBJCOPY) -j .text -O ihex build/master.out build/master.hex

master.out: $(MASTER_SOURCES)
	$(CC) $(CFLAGS) -I./ -o build/master.out $(MASTER_SOURCES)

master: master.hex

slave.hex: slave.out
	$(OBJCOPY) -j .text -O ihex build/slave.out build/slave.hex

slave.out: $(SLAVE_SOURCES)
	$(CC) $(CFLAGS) -I./ -o build/slave.out $(SLAVE_SOURCES)

slave: slave.hex


program: slave.hex
	avrdude -p $(MMCU) -c stk500v2 -P /dev/cu.PL2303-* $(FUSES) -U flash:w:build/slave.hex
clean:
	rm -f build/*
