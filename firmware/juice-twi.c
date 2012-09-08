/***********************************************************************
 * juice-twi.c
 * Main I2C/TWI processing loop for Raspy Juice (Raspberry Pi Exp Board)
 * MCU: ATmega168A, 14.7456MHz
 *
 *
 *
 * Copyright (c) 2012, Adnan Jalaludin <adnan singnet.com.sg>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#include "juice.h"

#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

FILE rs232_stream = FDEV_SETUP_STREAM(rs232_putchar, rs232_getchar, 
				      _FDEV_SETUP_RW);

uint8_t led_state = 1;
long led_counter = 0, led_timing[4] = { 500000L, 40000L, 20000L, 40000L }; 
void led_heartbeat(void)
{
    if (led_counter++  > led_timing[led_state]) {
	led_counter = 0;
	led_state = ++led_state % 4;
	if (led_state % 2)
	    LED_ON();
	else
	    LED_OFF();
    }
}

void TWI_vect(void);

int main(void)
{
    JUICE_PCBA_PINS_INIT();
    
    rs232_swuart_init();
    rs485_init(115200);
    servo_init();
    /* Enable ADC, and set clock prescaler to div 128 */
    ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    
    TWAR = (AVRSLAVE_ADDR << 1);
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
    
    sei();

    /* Test printouts */
    stdout = stdin = &rs232_stream;
    printf_P(PSTR("\nTest Application of Juice Firmware\n"));
    printf_P(PSTR("Second line\n\n"));
    
    while (1) {
	led_heartbeat();
    }
    
}

#ifdef TWI_POLL_MODE
# define TWI_debug(fmt, ...)	printf_P(fmt, ##__VA_ARGS__)
#else
# define TWI_debug(fmt, ...)
#endif

/* TWI slave transmitter mode is extremely timing sensitive */
/* this is a workaround for the situation, what crappy programming */
volatile unsigned char gstat = 0;
volatile unsigned char adch, adcl;
volatile unsigned char rs485d, rs232d; 

volatile int adcval;
ISR(ADC_vect)
{
    adch = ADCH;
    adcl = ADCL;
    gstat = 0;
    //    ADCSRA &= ~(1 << ADIE);	/* disable ADC interrupt */
}

ISR(TWI_vect)
{
    uint8_t data;
    uint8_t ack = (1 << TWEA);
    static uint8_t bcnt, reg, eebyte;
    static int servo_pwm, eeaddr;
    
    TWI_debug(PSTR("twsr = 0x%02x\n"), twsr);
    
    switch (TWSR & 0xF8) {
	/* Own SLA+R has been received; ACK has been returned */
    case 0xA8:
	TWI_debug(PSTR("\nSLA+R AVR slave addressed for read, so send data\n"));
	bcnt = 0;
	/* No break: fall-through to case 0xB8 for sending data */
	/* Data byte in TWDR has been transmitted; ACK has been recv */
    case 0xB8:
	TWI_debug(PSTR("prev SLA+R recvd, ACK returned, so send more data\n"));
	switch(reg) {
	case GSTAT:
#if 0
	    TWDR =
		(rs232_havechar() ? RXA232 : 0) |
		(rs485_havechar() ? RXA485 : 0) |
		(eeprom_is_ready() ? 0 : EEBUSY) |
		(ADCSRA & (1 << ADSC) ? ADCBUSY : 0);
#else
	    TWDR = gstat;
#endif
	    TWCR |= (1<<TWINT) | (1<<TWEA);
	    break;
	case ADCDAT:
	    if (bcnt == 0) {
		TWDR = ADCL;
		bcnt++;
	    }
	    else
		TWDR = ADCH;
	    TWCR |= (1<<TWINT) | (1<<TWEA);
	    break;
	case RS232D:
	    if (rs232_havechar())
		TWDR = rs232_getc();
	    TWCR |= (1<<TWINT) | (1<<TWEA);
	    break;
	case RS485D:
	    if (rs485_havechar())
		TWDR = rs485_getc();
	    TWCR |= (1<<TWINT) | (1<<TWEA);
	    break;
	case EEDATA:
	    TWDR = eebyte;
	    TWCR |= (1<<TWINT) | (1<<TWEA);
	    bcnt++;
	    break;
	}			
	//	TWDR = data;
	//	TWCR |= (1<<TWINT) | (1<<TWEA);
	break;
	
    case 0x60:
	bcnt = 0;
	TWCR |= (1<<TWINT) | (1<<TWEA);
	TWI_debug(PSTR("\nSLA+W AVR slave addressed -> recv data and ACK\n"));
	break;
	
    case 0x80:
	TWI_debug(PSTR("prev SLA+W, data recvd, ACK returned -> recv data and ACK\n"));
	data = TWDR;
	switch (++bcnt) {
	case 0:
	    break;

	case 1:
	    reg = data;
	    TWI_debug(PSTR("reg addr = 0x%02x\n"), data);
	    break;

	case 2:
	    TWI_debug(PSTR("data = 0x%02x\n"), data);
	    if (reg >= 1 && reg <= 4) {
		servo_pwm = data;
	    }
	    else if (reg == RS232D) {
		rs232_putc(data);
	    }
	    else if (reg == RS485D) {
		rs485_putc(data);
	    }
	    else if (reg == ADCMUX) {
		/* Set mux here, force ADLAR bit to zero, and start */
		/* let user i2c setting to select the volt ref src  */
		ADMUX = (data & 0b11001111);
		ADCSRA |= (1 << ADSC);
		gstat |= 0b01000000;
	    }
	    else if (reg == EEADDR) {
		eeaddr = data;
	    }
	    else if (reg == EEDATA) {
		eeprom_write_byte((uint8_t *)eeaddr, data);
	    }
	    else if ((reg == REBOOT) && (data == BOOTVAL)) {
		TWCR |= (1<<TWINT) | (1<<TWEA);
		TWI_debug(PSTR("Resetting to bootloader\n\n"));
		wdt_enable(WDTO_30MS);
		while(1) {}; 
	    }
	    else
		ack = 0;
	    break;

	default:
	    TWI_debug(PSTR("extended data = 0x%02x\n"), data);
	    if (reg >= SERVO_0 && reg <= SERVO_3) {
		servo_pwm = (data << 8) | servo_pwm;
		TWI_debug(PSTR("Setting servo %d to %dusec\n"), reg-1, servo_pwm);
		servo_set(reg-1, servo_pwm);
	    }
	    else if (reg == RS232D) {
		rs232_putc(data);
	    }
	    else if (reg == EEADDR) {
		eeaddr = ((data << 8) & 0x01) | eeaddr;
		/* do eeprom readback here */
		eebyte = eeprom_read_byte((uint8_t *)eeaddr);
	    }
	    else 
		ack = 0;
	    break;
	}
	TWCR |= (1<<TWINT) | ack;
	break;
	
	/* STOP or repeated START */
    case 0xA0:
	TWCR |= (1<<TWEN) | (1<<TWINT) | (1<<TWEA) | (1<<TWSTO);
	TWI_debug(PSTR("STOP or repeated START, do what?\n"));
	break;
	
	/* data sent, NACK returned */
    case 0xC0:
	TWCR |= (1<<TWINT) | (1<<TWEA);
	TWI_debug(PSTR("data sent, NACK returned, do what?\n"));
	break;
	
	/* illegal state -> reset hardware */
    case 0xF8:
	TWCR |= (1<<TWINT) | (1<<TWEA) | (1<<TWSTO);
	TWI_debug(PSTR("illegal state, resetting hardware\n"));
	break;
    }
}
