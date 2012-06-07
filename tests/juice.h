/***********************************************************************
 * juice.h
 * Header file definitions for Raspy Juice (Raspberry Pi Exp Board)
 *
 *
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

#ifndef __JUICE_H__
#define __JUICE_H__

#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/io.h>


/* 
 * PD7: AVR-to-RPI-GPIO7 activity signal (and LED) output
 * PD6: Uncommitted exp pin, set as output
 * PD5: Uncommitted exp pin, set as output
 * PD4: RS485 TXEN output
 * PD3: RS232 TXD output
 * PD2: RS232 RXD (INT0) input
 * PD1: RS485 TXD output
 * PD0: RS485 RXD input
 *
 * PC6: User for RESET#
 * PC5: RPISCL slave OC, set as input
 * PC4: RPISDA slave OC, set as input
 * PC3: Servo 3 output
 * PC2: Servo 2 output
 * PC1: Servo 1 output
 * PC0: Servo 0 output
 *
 * PB7: Used for XTAL2
 * PB6: Used for XTAL1
 * PB5: Uncommitted (SCK for ICP)
 * PB4: Uncommitted (MISO for ICP)
 * PB3: Uncommitted exp pin, set as output (MOSI for ICP)
 * PB2: Uncommitted exp pin, set as output
 * PB1: Uncommitted exp pin, set as output
 * PB0: Uncommitted exp pin, set as output
 *
 */

#define JUICE_PCBA_PINS_INIT() {		\
		DDRD =  0b11111010;		\
		PORTD = 0b00001010;		\
		DDRC =  0b00001111;		\
		PORTC = 0b00000000;		\
		DDRB =  0b00001111;		\
		PORTB = 0b00000000; }

#define LED_ON()	(PORTD |=  (1<<PD7))
#define LED_OFF()	(PORTD &= ~(1<<PD7))
#define PD6_ON()	(PORTD |=  (1<<PD6))
#define PD6_OFF()	(PORTD &= ~(1<<PD6))
#define PD5_ON()	(PORTD |=  (1<<PD5))
#define PD5_OFF()	(PORTD &= ~(1<<PD5))

#define RS485_TXEN_ON()	(PORTD |=  (1<<PD4))
#define RS485_TXEN_OFF() (PORTD &= ~(1<<PD4))

#define RS232_TXD_HIGH() (PORTD |=  (1<<PD3))
#define RS232_TXD_LOW()	(PORTD &= ~(1<<PD3))
#define RS232_RXD()	(PIND  &   (1<<PD2))

#define BLINK1(A)	{ LED_ON(); msleep(A); LED_OFF(); }
#define BLINK2(A)	{ BLINK1(A); msleep(A); BLINK1(A);}
#define BLINK3(A)	{ BLINK1(A); msleep(A); BLINK2(A);}
#define WAKEUP	800
#define PERIOD	400
#define DITDIT	120

void rs232_swuart_init(void);
void rs232_swuart_off(void);
char rs232_havechar(void);
char rs232_getc(void);
void rs232_putc(char);
void rs232_puts(char *);
/* For avrgcc stdio.h */
int rs232_putchar(char c, FILE *stream);
int rs232_getchar(FILE *stream);

void rs485_init(unsigned long baud);
char rs485_havechar(void);
char rs485_getc(void);
void rs485_putc(char c);
/* For avrgcc stdio.h */
int rs485_putchar(char c, FILE *stream);
int rs485_getchar(FILE *stream);





void servo_init(void);
void servo_set(uint8_t chan, int usec);


/* Addresses of I2C slaves on Juice PCBA */
#define PCF8523_ADDR_WRITE	(unsigned char) 0b11010000
#define PCF8523_ADDR_READ	(unsigned char) 0b11010001
#define AVRSLAVE_ADDR		(unsigned char) 0x48
#define AVRSLAVE_ADDR_WRITE	(unsigned char) 0x90
#define AVRSLAVE_ADDR_READ	(unsigned char) 0x91

/* Addresses of Juice emulated AVR I2C registers */
#define GSTAT	0x00
#define RXA232	0x01	/* RS232 data available */
#define RXA485	0x04	/* RS485 data available */
#define EEBUSY	0x20	/* EEPROM write in progress */
#define ADCBUSY	0x40	/* ADC conversion in progress */

/* AVR PC0..PC3 servo pulse width setting registers */
#define SRV_0	0x01
#define SRV_1	0x02
#define SRV_2	0x03
#define SRV_3	0x04
#define	SRV_4	0x05
#define SRV_5	0x06
#define SRV_6	0x07
#define SRV_7	0x08

/* AVR RS232 registers */
#define RS232D	0x10

/* AVR RS485 registers */
#define RS485D	0x20

/* AVR ADC set and readback registers */
#define ADCMUX	0x40
#define ADCDAT	0x41

/* AVR ADC EEPROM registers */
#define EEADDR	0x50
#define EEDATA	0x51

/* AVR Reboot register and constant to write to reboot*/
#define REBOOT	0xb0
#define BOOTVAL	0x0d

#endif

