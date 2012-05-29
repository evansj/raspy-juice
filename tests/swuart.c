/***********************************************************************
 * swuart.c
 * RS232 software UART driver for Raspy Juice (Raspberry Pi Exp Board)
 * Half-duplex, 9600, 8N1. Hardware pins based on juice.h
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
#include <avr/interrupt.h>

/* With MCU clock of 7372800Hz prescaled by 256, period = 34.7us */
#define BPS9600_TIMER_PRESCALE	0b011   // clk/32
#define BPS9600_FULLPERIOD	24	// 104us
#define BPS9600_ONEHALFPERIOD	30	// should be 36, but try 

/* Anyways, BPS57K6 doesn't work: possibly upto 38400 only - still to test */
#define BPS38K4_TIMER_PRESCALE	0b010	// clk/8
#define BPS38K4_FULLPERIOD	24	// 17.4us
#define BPS38K4_ONEHALFPERIOD	30

#define BPS_PRESCALER		BPS9600_TIMER_PRESCALE
#define BPS_FULLPERIOD		BPS9600_FULLPERIOD
#define BPS_ONEHALFPERIOD	BPS9600_ONEHALFPERIOD

#define PD2_INT0_ENABLE()	{ EIFR |= (1<<INTF0);  EIMSK |=  (1<<INT0); }
#define PD2_INT0_DISABLE()	{ EIMSK &= ~(1<<INT0); }
#define TIMER2_INT_ENABLE()	{ TIFR2 = 0b111;       TIMSK2 = (1<<OCIE2A); }
#define TIMER2_INT_DISABLE()	{ TIMSK2 = 0; }

enum SWUART_STATES {
	IDLE = 0,
	RECV_START = 1,
	XMIT_START = 16,
};

static volatile char swuart_state = IDLE;
static volatile int  swuart_shft_reg;
static volatile int  swuart_data_reg;
static volatile char swuart_data_rdy = 0;
static volatile int  swuart_xmit_reg;

void rs232_swuart_init(void)
{
	/* enable falling-edge INT0 on PD2 pin */
	EICRA |= ((1<<ISC01) | (0<<ISC00));
	/* set timer2 prescaler, mode and run */
	TCCR2A = (1<<WGM21) | (0<<WGM20);
	TCCR2B = (0<<WGM22) | BPS_PRESCALER;
	OCR2A = BPS_FULLPERIOD - 1;
	swuart_state = IDLE;
	RS232_TXD_HIGH();
	TIMER2_INT_DISABLE();
	PD2_INT0_ENABLE();
}

void rs232_swuart_off(void)
{
	TIMER2_INT_DISABLE();
	PD2_INT0_DISABLE();
}

char rs232_havechar(void)
{
	return swuart_data_rdy;
}

char rs232_getc(void)
{
	while (!rs232_havechar())
		;
	swuart_data_rdy = 0;
	return (char)swuart_data_reg;
}

void rs232_putc(char c)
{
	while (swuart_state != 0)
		;
    
	PD2_INT0_DISABLE();
	swuart_xmit_reg = c;
	swuart_xmit_reg |= (1 << 8);
    
	swuart_state = XMIT_START;
	RS232_TXD_LOW();
	TCNT2 = 0;
	TIMER2_INT_ENABLE();
}

void rs232_puts(char *s)
{
	while(*s) {
		if (*s == '\r')
			rs232_putc('\n');
		rs232_putc(*s++);
	}
}

int rs232_putchar(char c, FILE *stream)
{
	if (c == '\n')
		rs232_putc('\r');
	rs232_putc(c);
	return 0;
}

int rs232_getchar(FILE *stream)
{
	return rs232_getc();
}


/***********************************************************************
 * SWUART RxD Interrupts, Char and Packet Reception.
 ***********************************************************************/

ISR(INT0_vect)
{
	TCNT2 = 0;
	OCR2A = BPS_ONEHALFPERIOD;
	TIMER2_INT_ENABLE();
	if (swuart_state == IDLE) {
		swuart_state = RECV_START;
		swuart_shft_reg = 0;
		PD2_INT0_DISABLE();
	}
}
ISR(TIMER2_COMPA_vect)
{

	OCR2A = BPS_FULLPERIOD - 1;
    
	/* do nothing if in IDLE state */
	if (swuart_state == IDLE)
		return;
    
	if (swuart_state < 16) {
		// PD5_ON();
		/* process state machine for reception */
		if (swuart_state < 9) {
			swuart_shft_reg >>= 1;
			if (RS232_RXD())
				swuart_shft_reg |= 0b10000000;
			swuart_state++;
		}
		else {
			/* end reception */
			swuart_data_reg = swuart_shft_reg;
			swuart_data_rdy = 1;
			TIMER2_INT_DISABLE();
			PD2_INT0_ENABLE();
			swuart_state = IDLE;
		}
	}
	else {
		/* swuart state must be bigger than 16, so */
		/* process state machine for transmission  */
		if (swuart_state < 25) {
			if (swuart_xmit_reg & 0x01) {
				RS232_TXD_HIGH();
			}
			else {
				RS232_TXD_LOW();
			}
			//TIMER2_INT_ENABLE();
			swuart_xmit_reg >>= 1;
			swuart_state++;
		}
		else {
			/* end transmission */
			RS232_TXD_HIGH();
			TIMER2_INT_DISABLE();
			PD2_INT0_ENABLE();
			swuart_state = IDLE;
		}
	}
}

#ifdef STANDALONE_UART_TEST
int main(void)
{
	char c;
    
	JUICE_PCBA_PINS_INIT();
	swuart_init();
	sei();
	rs232_puts("\nTest Application of SW-UART\n\n");
    
	while (1) {
		/* Testing rs232 echo */
		if (rs232_havechar()) {
			c = rs232_getc();
			if (c == '\r') {
				rs232_putc('\n');
			}
			rs232_putc(c);
		}
	}
}
#endif

