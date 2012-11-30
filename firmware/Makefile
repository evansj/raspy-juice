###############################################################################
# Makefile for the Raspy Juice firmware and Flash update
###############################################################################

## Raspberry Pi
PI_MODEL =  $(shell i2cdetect -y 0 | grep -e '29\|48' )
ifeq ( $(PI_MODEL), )
  PI_I2CBUS = 0
else
  PI_I2CBUS = 1
endif

## General Flags
PROJECT = juice
TARGET = juice
MCU = atmega168a
CC = avr-gcc
CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=14745600UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=juice.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = juice-twi.o svc-servo.o svc-rs232.o svc-rs485.o i2c-master.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
#all: $(TARGET).elf $(TARGET).hex $(TARGET).lss size
all: $(TARGET).elf $(TARGET).hex 

## Compile
juice-twi.o: juice-twi.c juice.h
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

svc-servo.o: svc-servo.c juice.h
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

svc-rs232.o: svc-rs232.c juice.h
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

svc-rs485.o: svc-rs485.c juice.h
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<


##Link
$(TARGET).elf: $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET).elf

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET).elf
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET).elf
	avr-objdump -h -S $< > $@

size: $(TARGET).elf
	@echo
	@avr-size -C --mcu=${MCU} $(TARGET).elf

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) dep $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).lss $(TARGET).map *~ 

flashinstall: $(TARGET).hex
	/usr/sbin/i2cset -y $(PI_I2CBUS)  0x48 0xb0 0x0d ; sleep 0.5 ; /usr/local/bin/twiboot -d /dev/i2c-$(PI_I2CBUS) -a 0x29 -w flash:$(TARGET).hex

reset:
	/usr/sbin/i2cset -y $(PI_I2CBUS)  0x48 0xb0 0x0d

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

