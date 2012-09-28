/***********************************************************************
 * svc-rs485.c
 * RS485 USART driver for Raspy Juice (Raspberry Pi Exp Board)
 * Half-duplex, 115200, 8N1. Hardware pins based on juice.h
 * MCU: ATmega168A, 14.7456MHz
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
#include <avr/interrupt.h>

#define DEFAULT_BPS115200_UBRR (F_CPU/16/115200L-1)

#define RS485_RX_BUFSIZE 64
#define RS485_TX_BUFSIZE 64
#define RS485_RX_BUFMASK (RS485_RX_BUFSIZE - 1)
#define RS485_TX_BUFMASK (RS485_TX_BUFSIZE - 1)
#if    (RS485_RX_BUFSIZE & RS485_RX_BUFMASK)
#  error RX buffer size is not a power of 2
#endif
#if    (RS485_TX_BUFSIZE & RS485_TX_BUFMASK)
#  error TX buffer size is not a power of 2
#endif

static unsigned char rs485_rxbuf[RS485_RX_BUFSIZE];
static unsigned char rs485_txbuf[RS485_TX_BUFSIZE];
static volatile unsigned char rxhead, rxtail, txhead, txtail;

/* localecho may be used for collision detection, currently unset */
volatile unsigned localecho;

void rs485_init(void)
{
    localecho = 0;
    rxhead = rxtail = txhead = txtail = 0;
    rs485_setbaud(DEFAULT_BPS115200_UBRR);
    RS485_TXEN_OFF();
    /* Enable receiver and transmitter */
    UCSR0B = (1<<RXCIE0) |  (1<<RXEN0) | (1<<TXEN0) | (1<<TXCIE0);
    /* Set frame format: Async,8,N,1 */
    UCSR0C = 0b00000110;
}     

void rs485_setbaud(int ubrr)
{
    UBRR0 = ubrr;
}

ISR(USART_RX_vect)
{
    unsigned char data;
    unsigned char tmphead;
    data = UDR0;
    tmphead = (rxhead + 1) & RS485_RX_BUFMASK;
    rxhead = tmphead;
    if (tmphead == rxtail) {
	/* error! recv buffer overflow routine */
    }
    rs485_rxbuf[tmphead] = data;
}

ISR(USART_UDRE_vect)
{
    unsigned char tmptail;
    if (txhead != txtail) {
	tmptail = (txtail + 1) & RS485_TX_BUFMASK;
	txtail = tmptail;
	if (localecho == 0)
	    UCSR0B &= ~(1<<RXEN0);
	RS485_TXEN_ON();
	UDR0 = rs485_txbuf[tmptail];
    }
    else {
	UCSR0B &= ~(1<<UDRIE0);
    }
}

ISR(USART_TX_vect)
{
    RS485_TXEN_OFF();
    UCSR0B |= (1<<RXEN0);
}

char rs485_havechar(void)
{
    return (rxhead != rxtail);
}

unsigned char rs485_getc(void)
{
    unsigned char tmptail;
    while (rxhead == rxtail)
	;
    tmptail = (rxtail + 1) & RS485_RX_BUFMASK;
    rxtail = tmptail;
    return rs485_rxbuf[tmptail];
}

void rs485_putc(unsigned char c)
{
    unsigned char tmphead;
    tmphead = (txhead +1) & RS485_TX_BUFMASK;
    while (tmphead == txtail)
	;
    rs485_txbuf[tmphead] = c;
    txhead = tmphead;
    UCSR0B |= (1<<UDRIE0);
}

int rs485_getchar(FILE *stream)
{
    return rs485_getc();
}

int rs485_putchar(char c, FILE *stream)
{
    if (c == '\r')
	rs485_putc('\n');
    rs485_putc(c);
    return 0;
}

void rs485_puts(char *s)
{
    while (*s)
	rs485_putchar(*s++, NULL);
}

