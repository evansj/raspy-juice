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
#include <avr/pgmspace.h>

char const VERSION_STR[] PROGMEM = "$Id$";

FILE rs232_stream = FDEV_SETUP_STREAM(rs232_putchar, rs232_getchar, 
				      _FDEV_SETUP_RW);

volatile static unsigned char twi_state = 0;

unsigned char led_state = 1;
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


#ifdef TWI_POLL_MODE
# define TWI_debug(fmt, ...)	printf_P(fmt, ##__VA_ARGS__)
#else
# define TWI_debug(fmt, ...)
#endif

int main(void)
{
    unsigned char twi_last_state = 0;
    int twi_reset_count = 0;

    JUICE_PCBA_PINS_INIT();
    
    rs232_swuart_init();
    rs485_init(115200);
    servo_init();
    /* Enable ADC, and set clock prescaler to div 128 */
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    
    TWAR = (AVRSLAVE_ADDR << 1);
    TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
    
    sei();

    /* Test printouts */
    stdout = stdin = &rs232_stream;
    TWI_debug("\r\nTest Application of Juice Firmware\r\n");
    TWI_debug("Second line\r\n\r\n");
    
    while (1) {
	led_heartbeat();

	/* Basic watchdog for AVR TWI module failure */
	if (twi_last_state != twi_state) {
	    twi_last_state = twi_state;
	    TWI_debug("twi_state = %02x\r\n", twi_state);
	    if ((twi_state == 0)) {
		twi_reset_count++;
		TWI_debug("WARNING! Resetting TWI module, count = %d\r\n",
			  twi_reset_count);
		TWAR = (AVRSLAVE_ADDR << 1);
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
	    }
	}
    }
    
}

/****************************************************************************
 * The AVR TWI module slave transmitter mode is extremely timing sensitive to
 * tranfer bytes from itself as a slave to the master.
 * 
 * This TWI ISR follows the Atmel AVR311 App Note closely, where timing
 * priority is placed on TWI slave transmission of data
 * File              : TWI_Slave.c
 * AppNote           : AVR311 - TWI Slave Implementation
 * Description       : Interrupt-driver sample driver to AVRs TWI module. 
 ****************************************************************************/
ISR(TWI_vect)
{
    static unsigned char bcnt, reg, prep_data;
    static int servo_pwm, eeaddr;
    static const char *pgm_ptr;
    unsigned char data;
    
    twi_state = TWSR;
    switch (twi_state & 0xF8) {
    case TWI_STX_ADR_ACK:
    case TWI_STX_DATA_ACK:
	TWDR = prep_data;
	TWCR = (1<<TWEN)| (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);                      

	if (reg == ADCDAT) {
	    prep_data = ADCH;
	}

#if 0
	// TODO: have to re-think how to slave-xmit multiple bytes for
	// received data with ack/nack. Can't put the below lines because
	// if master asks for 1 byte only, pre-unload of reception FIFO's
	// will lose the data. So, for now, the master has to check GSTAT,
	// followed by single byte read -- a bit inefficient.

	else if (reg == RS232D) {
	    if (rs232_havechar())
		prep_data = rs232_getc();
	}
	else if (reg == RS485D) {
	    if (rs485_havechar())
		prep_data = rs485_getc();
	}
#endif
	else if (reg == EEDATA) {
	    prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
	}
	else if (reg == RJ_VERSION) {
	    prep_data = pgm_read_byte(pgm_ptr++);
	}
	break;

    case TWI_STX_DATA_NACK:
	// Data byte in TWDR has been transmitted; NACK has been received. 
	// Do nothing
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO)| (0<<TWWC);
	break;     

    case TWI_SRX_GEN_ACK:
    case TWI_SRX_ADR_ACK:
	bcnt = 0;
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;

    case TWI_SRX_ADR_DATA_ACK:
    case TWI_SRX_GEN_DATA_ACK:
	data = TWDR;
	switch (++bcnt) {
	case 0:
	    break;

	case 1:
	    reg = data;
	    
	    /* For readback registers, prepare data here */
	    if (reg == GSTAT) {
		prep_data = 
		    (rs232_havechar() ? RXA232 : 0) |
		    (rs485_havechar() ? RXA485 : 0) |
		    (eeprom_is_ready() ? 0 : EEBUSY) |
		    (ADCSRA & (1<<ADSC) ? ADCBUSY : 0);
	    }
	    else if (reg == ADCDAT) {
		prep_data = ADCL;
	    }
	    else if (reg == RS232D) {
		if (rs232_havechar())
		    prep_data = rs232_getc();
	    }
	    else if (reg == RS485D) {
		if (rs485_havechar())
		    prep_data = rs485_getc();
	    }
	    else if (reg == EEDATA) {
		prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
	    }
	    else if (reg == RJ_VERSION) {
		prep_data = pgm_read_byte(pgm_ptr);
	    }
	    break;

	case 2:
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
	    }
	    else if (reg == EEADDR) {
		eeaddr = data;
	    }
	    else if (reg == EEDATA) {
		eeprom_write_byte((unsigned char *)eeaddr, data);
	    }
	    else if (reg == RJ_VERSION) {
		pgm_ptr = VERSION_STR;
	    }
	    else if ((reg == REBOOT) && (data == BOOTVAL)) {
		TWCR |= (1<<TWINT) | (1<<TWEA);
		wdt_enable(WDTO_30MS);
		while(1) {}; 
	    }
	    break;

	default:
	    if (reg >= SERVO_0 && reg <= SERVO_3) {
		servo_pwm = (data << 8) | servo_pwm;
		servo_set(reg - 1, servo_pwm);
	    }
	    else if (reg == RS232D) {
		rs232_putc(data);
	    }
	    else if (reg == RS485D) {
		rs485_putc(data);
	    }
	    else if (reg == EEADDR) {
		eeaddr = ((data << 8) & 0x01) | eeaddr;
		prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
	    }
	    break;
	}
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;

    case TWI_SRX_STOP_RESTART:
	// Enter not addressed mode and listen to address match
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;           

    case TWI_SRX_ADR_DATA_NACK:
    case TWI_SRX_GEN_DATA_NACK:
    case TWI_STX_DATA_ACK_LAST_BYTE:
    case TWI_BUS_ERROR:
	TWCR = (1<<TWSTO) | (1<<TWINT);
	break;

    default:
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;
    }

}
