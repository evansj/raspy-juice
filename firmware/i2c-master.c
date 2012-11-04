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
    i2c_out(addr << 1);

    for (i = 0; i < len; i++)
	i2c_out(buf[i]);
    i2c_stop();
}

void i2c_read(unsigned char addr, unsigned char *buf, int len)
{
    int i;

    i2c_start();
    i2c_out((addr << 1) | 1);
    for (i = 0; i < 10; i++)
	buf[i] = i2c_inACK();
    i2c_stop();
}

