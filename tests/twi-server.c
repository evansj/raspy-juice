/***********************************************************************
 * twi-server.c
 * Main I2C/TWI processing loop for Raspy Juice (Raspberry Pi Exp Board)
 * MCU: ATmega168A, 7.3728MHz Resonator
 *
 *
 * Copyright (c) 2012, Adnan Jalaludin
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

uint8_t led_state = 0;
long led_counter = 0, led_timing[4] = { 100000L, 20000L, 10000L, 20000L }; 
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

volatile int adcval;

#define TWI_POLL_MODE 1
void TWI_vect(void);

int main(void)
{
	char c;

	JUICE_PCBA_PINS_INIT();
	rs232_swuart_init();
	servo_init();

	TWAR = (AVRSLAVE_ADDR<<1);
#ifdef TWI_POLL_MODE
	TWCR = (1<<TWEA) | (1<<TWEN);
#else
	TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
#endif
    
	sei();
	stdout = stdin = &rs232_stream;

	printf_P(PSTR("\nTest Application of Juice Firmware\n"));
	printf_P(PSTR("Second line\n\n"));

	while (1) {
		led_heartbeat();

		/* Testing rs232 echo */
		if (rs232_havechar()) {
			c = rs232_getc();
			if (c == '\r')
				rs232_putc('\n');
			rs232_putc(c);
		}

#ifdef TWI_POLL_MODE
		if (TWCR & 0x80) {
			TWI_vect();
		}
#endif
	
	}
    
}

ISR(ADC_vect)
{
  adcval = ADCW;
  ADCSRA &= ~_BV(ADIE);		/* disable ADC interrupt */
}

#ifdef TWI_POLL_MODE
# define TWI_debug(fmt, ...)	printf_P(fmt, ##__VA_ARGS__)
#else
# define TWI_debug(fmt, ...)
#endif

#ifdef TWI_POLL_MODE
void TWI_vect(void)
#else
ISR(TWI_vect)
#endif
{
	uint8_t data;
	uint8_t ack = (1<<TWEA);
	static uint8_t bcnt, reg, eebyte;
	static int servo_pwm, eeaddr;
  
	bcnt++;
	switch (TWSR & 0xF8) {
	case 0x60:
		TWI_debug(PSTR("\nSLA+W AVR slave addressed -> recv data and ACK\n"));
		bcnt = 0;
                TWCR |= (1<<TWINT) | (1<<TWEA);
                break;

	case 0x80:
		TWI_debug(PSTR("prev SLA+W, data recvd, ACK returned -> recv data and ACK\n"));
		data = TWDR;
		switch (bcnt) {
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
			else if (reg == ADCMUX) {
				/* Set mux here, use int 1.1V ref and start */
				ADMUX = 0xc0 | (data & 0x0f);
				ADCSRA = 0b11000000;
			}
			else if (reg == EEADDR) {
				eeaddr = data;
			}
			else if (reg == EEDATA) {
				eeprom_write_byte((uint8_t *)eeaddr, data);
			}
			else if ((reg == REBOOT) && (data == BOOTVAL)) {
				TWI_debug(PSTR("Resetting to bootloader\n\n"));
				wdt_enable(WDTO_30MS);
				while(1) {}; 
			}
			else
				ack = 0;
	    
			break;
		default:
			TWI_debug(PSTR("extended data = 0x%02x\n"), data);
			if (reg >= 1 && reg <= 4) {
				servo_pwm = (data << 8) | servo_pwm;
				if (servo_pwm <  500) servo_pwm = 500;
				if (servo_pwm > 2500) servo_pwm = 2500;
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
	

	case 0xA8:
		TWI_debug(PSTR("\nSLA+R AVR slave addressed for read, so send data\n"));
		bcnt = 0;
		/* No break: fall-through to case 0xB8 for sending data */
	case 0xB8:
		TWI_debug(PSTR("prev SLA+R recvd, ACK returned, so send more data\n"));

		data = 0;
		switch(reg) {
		case GSTAT:
			data = (rs232_havechar() ? RXA232 : 0) |
				(eeprom_is_ready() ? 0 : EEBUSY) |
				(ADCSRA & _BV(ADSC));
			break;

		case ADCDAT:
			if (bcnt == 0)
				data = ADCL;
			if (bcnt == 1)
				data = ADCH;
			break;

		case EEDATA:
			data = eebyte;
			break;
		}			

		TWDR = data;
		TWCR |= (1<<TWINT) | (1<<TWEA);
		break;
	
		/* STOP or repeated START */
	case 0xA0:
		TWI_debug(PSTR("STOP or repeated START, do what?\n"));

		/* data sent, NACK returned */
	case 0xC0:
		TWI_debug(PSTR("data sent, NACK returned, do what?\n"));
		TWCR |= (1<<TWINT) | (1<<TWEA);
		break;
	
		/* illegal state -> reset hardware */
	case 0xF8:
		TWI_debug(PSTR("illegal state, resetting hardware\n"));
		TWCR |= (1<<TWINT) | (1<<TWSTO) | (1<<TWEA);
		break;
	}
}
