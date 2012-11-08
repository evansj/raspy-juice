/***********************************************************************
 * i2c-master.c
 * AVR I2C/TWI-master module for Raspy Juice (Raspberry Pi Exp Board)
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
	i2c_out(*buf++);
    i2c_stop();
}

void i2c_read(unsigned char addr, unsigned char *buf, int len)
{
    int i;

    i2c_start();
    i2c_out((addr << 1) | 1);

    for (i = 0; i < len; i++)
	*buf++ = i2c_inACK();
    i2c_stop();
}

