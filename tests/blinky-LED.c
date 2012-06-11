#include <avr/io.h>
#include <ctype.h>

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sleep.h>

#define msleep(X)	_delay_ms((X))

#define LED_INIT()		(DDRD  |=  (1<<PD7) | (1<<PD6))
#define LEDRED_ON()		(PORTD |=  (1<<PD7))
#define LEDRED_OFF()	(PORTD &= ~(1<<PD7))
#define LEDGRN_ON()		(PORTD |=  (1<<PD6))
#define LEDGRN_OFF()	(PORTD &= ~(1<<PD6))
#define BUCK_ON()		(PORTD &= ~(1<<PD5))
#define BUCK_OFF()		(PORTD |=  (1<<PD5))


volatile int blink_delay;
#define BLINKT(A)	{LEDRED_ON(); blink_delay = (A); }
#define BLINK1(A)	{LEDRED_ON(); msleep(A); LEDRED_OFF(); }
#define BLINK2(A)	{BLINK1(A); msleep(A); BLINK1(A);}
#define BLINK3(A)	{BLINK1(A); msleep(A); BLINK2(A);}

//#define FOSC 1843200// Clock Speed
#define BAUD 115200
#define MYUBRR (F_CPU/16/BAUD-1)
void usart_init(unsigned long baud);
void usart_putc(char c);
char usart_getc(void);


#define WAKEUP	800
#define PERIOD	400
#define DITDIT	120


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

int main(void)
{
	char c;

//	DDRB	= 0b00001111;
//	DDRD	= 0b11100000;
//	DDRC	= 0b00001111;
JUICE_PCBA_PINS_INIT();
	usart_init(BAUD);

#if 1

	PORTB=0xff;
	PORTC=0xff;
	PORTD=0xff;
	msleep(WAKEUP);
	PORTB=0x0;
	PORTC=0x0;
	PORTD=0x0;


	while(1) {
  		msleep(PERIOD);
		PORTB = 0x1;
		BLINK2(DITDIT);
		PORTB = 0;

  		msleep(PERIOD);
		PORTB = 0x2;
		BLINK2(DITDIT);
		PORTB = 0;

  		msleep(PERIOD);
		PORTB = 0x4;
		BLINK2(DITDIT);
		PORTB = 0;

  		msleep(PERIOD);
		PORTB = 0x8;
		BLINK2(DITDIT);
		PORTB = 0;

  		msleep(PERIOD);
		PORTD = (1 << PD5);
		BLINK2(DITDIT);
		PORTD = 0;

  		msleep(PERIOD);
		PORTD = (1 << PD6);
		BLINK2(DITDIT);
		PORTD = 0;

  		msleep(PERIOD);
		PORTC = 0x1;
		BLINK2(DITDIT);
		PORTC = 0;

  		msleep(PERIOD);
		PORTC = 0x2;
		BLINK2(DITDIT);
		PORTC = 0;

  		msleep(PERIOD);
		PORTC = 0x4;
		BLINK2(DITDIT);
		PORTC = 0;

  		msleep(PERIOD);
		PORTC = 0x8;
		BLINK2(DITDIT);
		PORTC = 0;
 
  		msleep(PERIOD);
		PORTC = 0x10;
		BLINK2(DITDIT);
		PORTC = 0;

  		msleep(PERIOD);
		PORTC = 0x20;
		BLINK2(DITDIT);
		PORTC = 0;

		usart_putc(' ');
		usart_putc('T');
		usart_putc('e');
		usart_putc('s');
		usart_putc('t');
		usart_putc(' ');
		usart_putc('1');
		usart_putc('2');
		usart_putc('3');
	}
#else

	while(1) {
		c = usart_getc();
		usart_putc(c);
		switch(c) {
		case '0': LEDGRN_OFF(); LEDRED_OFF(); BUCK_OFF(); break;
		case '1': LEDGRN_ON();	break;
		case '2': LEDGRN_OFF();	break;
		case '3': LEDRED_ON();	break;
		case '4': LEDRED_OFF();	break;
		case '5': BUCK_ON();	break;
		case '6': BUCK_OFF();	break;
		}
	}
#endif

}

#define MYUBRR (F_CPU/16/BAUD-1)

void usart_init(unsigned long baud)
{
	/* Set baud rate */
	unsigned long ubrr = (F_CPU / 16 / baud - 1);
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)(ubrr>>0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: Async,8,N,1 */
	UCSR0C = 0b00000110;
	//TX485_OFF();
}     

void usart_off(void)
{
	UCSR0B = 0;
}

char usart_havechar(void)
{
	return (UCSR0A & (1<<RXC0));
}

char usart_getc(void)
{
	//TX485_OFF();
	while (!usart_havechar())
		;
	return UDR0;
}

void usart_putc(char c)
{
	//TX485_ON();
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	UCSR0A |= (1<<TXC0);
	loop_until_bit_is_set(UCSR0A, TXC0);
	//TX485_OFF();
}

void usart_puts(char *s)
{
	//TX485_ON();
	_delay_us(150);
	while(*s)
		usart_putc(*s++);
}

void usart_write(char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
		usart_putc(*buf++);
}



