MMCU=atmega8
PORT=/dev/cu.PL2303-*
SOURCES=main.c network.c switch.c slip.c message.c led.c

CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-mmcu=$(MMCU) -Wall -O2 -std=c99
AVRDUDE=avrdude -p $(MMCU) -c stk500 -P $(PORT)

binary:
	$(CC) $(CFLAGS) -I./ -o build/build.out $(SOURCES)

hex: binary
	$(OBJCOPY) -j .text -O ihex build/build.out build/build.hex

# Fuses: http://www.engbedded.com/fusecalc/
# NOTE: Fuse bits are 'programmed' by setting them to 0.
#
# HIGH FUSE: 0xc1 - 1100 0001
# 7    - RSTDISBL - 1 - Use PC6 as reset pin.
# 6    - WDTON    - 1 - Disable Watchdog timer.
# 5    - SPIEN    - 0 - Enable SPI programming.
# 4    - CKOPT    - 0
#   Programming CKOPT makes the oscillator use an amplifier. The datasheet
#   suggests this is isn't nescessary for < 8MHz, however is useful for noisy
#   environments, and we were running into what looked like inconsistent
#   oscillator problems at burning man, so it's programmed.
# 3    - EESAVE   - 0 - Preserve EEPROM through the chip erase cycle.
# 2..1 - BOOTSZ1  - 00
#   Don't use a bootloader, so use the minimal amount of flash for it.
# 0    - BOOTRST  - 1
#   Don't go to a bootloader offset, go straight to Application Reset (address
#   0x0000)
#
#
# LOW FUSE: 0x3f - 0011 1111
# 7    - BODLEVEL - 0
#   Brown out detector trigger level set to 2.7V (as opposed to 4V.)
# 6    - BODEN    - 0 - Brown out detector enable
# 5..4 - SUT1..0  - 11 - Startup times
# 0    - CKSEL0   - 1  - Startup times.
#   High startup times give the osciallator more time to stabilize. We have no
#   startup time requirements, so go as long as possible - 16K CK + 65ms.
# 3..1 - CKSEL3..1 - 111 - Clock source selection
#   We use an external crystal to ensure that uart clocks are stable.
#   The datasheet isn't clear what to do with CKOPT enabled and a 7.3MHz
#   crystal (see page 27). So I've gone with the recommended settings for CKOPT
#   disabled and assumed they are separate.

fuses:
	$(AVRDUDE) -U lfuse:w:0x3f:m -U hfuse:w:0xc1:m
program: hex
	$(AVRDUDE) -U flash:w:build/build.hex
eeprom: ADDRESS=`hexdump -n 2 -v -e '"%04x"' /dev/random`
eeprom:
	# Generate a random hardware address
	$(AVRDUDE) -U eeprom:w:0x$(ADDRESS):m


clean:
	rm -f build/*
