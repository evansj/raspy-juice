/***********************************************************************
 * rtc.c
 * MCU: ATmega168A, 7.3728MHz Resonator.
 *
 * Juice Exp Board standalone test application for the PCF8523 RTC.
 * Used only as a test because the RTC is not normally accessed by the
 * MCU, only by the host RasPi motherboard.
 *
 * Author:	Adnan Jalaludin (adnan@singnet.com.sg)
 * Date:	2012-07-05
 ***********************************************************************/

#include "juice.h"
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define msleep(X)       _delay_ms((X))

/* Address bytes */
#define PCF8523_ADDRESS_WRITE	(unsigned char)0b11010000
#define PCF8523_ADDRESS_READ	(unsigned char)0b11010001

unsigned char obuf[20], inbuf[20];

/***********************************************************************
 * Basic no error-checking MASTER i2c routines - only used for testing
 * Not normally implemented in Juice, because MCU is supposed to be an
 * I2C slave.
 *
 * Reference: http://homepage.hispeed.ch/peterfleury/avr-software.html
 ***********************************************************************/
void i2c_start(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
    while (!(TWCR & (1<<TWINT)))
	;
}

void i2c_stop(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

void i2c_out(unsigned char val)
{
    TWDR = val;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)))
	;
}

unsigned char i2c_inACK(void)
{
    /* Read one byte from the I2C device, request more data from device */
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR & (1<<TWINT)))
	;    
    return TWDR;
}

unsigned char i2c_inNACK(void)
{
    /* Read one byte from the I2C device, a i2c_stop is to be followed */
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)))
	;
    return TWDR;
}

void i2c_write(unsigned char addr, unsigned char *buf, int len)
{
    int i;

    i2c_start();
    i2c_out(addr);

    for (i = 0; i < len; i++)
	i2c_out(buf[i]);
    i2c_stop();
}

void i2c_read(unsigned char addr, unsigned char *buf, int len)
{
    int i;

    i2c_start();
    i2c_out(addr);
    for (i = 0; i < 10; i++)
	inbuf[i] = i2c_inACK();
    i2c_stop();
}

#if 0
FILE uart_stream = FDEV_SETUP_STREAM(rs232_putchar, rs232_getchar, _FDEV_SETUP_RW);

int main(void)
{
    int i;

    JUICE_PCBA_PINS_INIT();
    swuart_init();
    sei();

    stdout = stdin = &uart_stream;

    TWBR = 1;
    TWSR = TWSR | 1;

    while(1) {
	BLINK2(DITDIT);
	printf_P(PSTR("\nTest Application of RTC\n"));
	printf_P(PSTR("Second line\n\n"));

	msleep(1000);
	BLINK2(DITDIT);
	msleep(1000);
    }
    i2c_write(PCF8523_ADDRESS_WRITE, obuf, 1);
    i2c_read( PCF8523_ADDRESS_READ, inbuf, 10);
    obuf[0] = 3;
    obuf[1] = 0;
    obuf[2] = 0x37;
    obuf[3] = 0x18;
	
//	i2c_write(PCF8523_ADDRESS_WRITE, obuf, 4);

    while (1) {
	BLINK1(DITDIT);

	obuf[0] = 0;	// set register addr to 0
	i2c_write(PCF8523_ADDRESS_WRITE, obuf, 1);
	i2c_read( PCF8523_ADDRESS_READ, inbuf, 10);

	printf("PCBF8523 read: ");
	for (i = 0; i < 10; i++) {
	    printf("0x%02x ", inbuf[i]);
	}
	printf("\n");

	printf("%02x:%02x:%02x\n", inbuf[5], inbuf[4], inbuf[3] & 0x7f);
	msleep(1000);
    }
}
#endif
