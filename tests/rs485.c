#include "juice.h"
#include <avr/interrupt.h>

#define MYUBRR (F_CPU/16/BAUD-1)

void rs485_init(unsigned long baud)
{
	/* Set baud rate */
	unsigned long ubrr = (F_CPU / 16 / baud - 1);
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)(ubrr>>0);
	/* Enable receiver and transmitter */
//	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0B = (1<<RXEN0);
	/* Set frame format: Async,8,N,1 */
	UCSR0C = 0b00000110;
	RS485_TXEN_OFF();
}     

void rs485_off(void)
{
	UCSR0B = 0;
}

char rs485_havechar(void)
{
	return (UCSR0A & (1<<RXC0));
}

char rs485_getc(void)
{
	//TX485_OFF();
	while (!rs485_havechar())
		;
	return UDR0;
}

void rs485_putc(char c)
{
	RS485_TXEN_ON();
	loop_until_bit_is_set(UCSR0A, UDRE0);

	UCSR0B = (1<<TXEN0);

	UDR0 = c;
	UCSR0A |= (1<<TXC0);
	loop_until_bit_is_set(UCSR0A, TXC0);
	RS485_TXEN_OFF();

	UCSR0B = (1<<RXEN0);

}

void rs485_puts(char *s)
{
	while (*s)
		rs485_putchar(*s++, NULL);
}

int rs485_putchar(char c, FILE *stream)
{
	if (c == '\n')
		rs485_putc('\r');
	rs485_putc(c);
	return 0;
}

int rs485_getchar(FILE *stream)
{
	return rs485_getc();
}
