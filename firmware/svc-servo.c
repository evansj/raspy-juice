/***********************************************************************
 * svc-servo.c
 * 4-port RC servo control for Raspy Juice (Raspberry Pi Exp Board)
 * Using PC0, PC1, PC2, PC3
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
#include <avr/pgmspace.h>

/* Default for 1.5ms setting */
#define SDEF	2764

/* For servo_port[], PORTA = 1, PORTB = 2, etc */
uint8_t servo_port[12]		= { 3,		3,	3,	3	};
uint8_t servo_pin [12]		= { 0,		1,	2,	3	};
volatile int servo_table[12]	= { SDEF,	SDEF,	SDEF,	SDEF	};
volatile uint8_t num_servos = 4;
volatile uint8_t servo_index = 0;

void servo_init(void)
{
    // Init timer1 for servo-control:
    // Resolution = cpuclk / prescale8 = 7.3728MHz = 1.085us
    // Servo index change-rate = 2765 * above = 3ms
    OCR1A = SDEF;
    ICR1 = 5528;		// =TOP, frequency of servo-pin change
    TCCR1A = 0b00000010;	// WGM11,10 = Fast PWM, TOP=ICR1
    TCCR1B = 0b00011010;	// WGM13,12 = Fast PWM, prescale clk/8
    TIFR1  = 0b00100111;
    TIMSK1 = (1<<OCIE1A) | (1<<TOIE1); // Enable Ovrflw & Match-Comp irqs
}

void servo_set(uint8_t chan, int usec)
{
    if (usec <  600) usec =  600;
    if (usec > 2400) usec = 2400;
    long corrected = (long)usec * 18432 / 10000;
    servo_table[(int)chan] = (int)corrected;
}

/***********************************************************************
 * Servo Interrupts
 ***********************************************************************/
ISR(TIMER1_COMPA_vect)
{
    int pulse_width;
    // disassert last servo pin, then increment index to 
    // servo of interest for next overflow interrupt.
    switch(servo_port[servo_index]) {
    case 2: PORTB &= ~(1 << servo_pin[servo_index]); break;
    case 3: PORTC &= ~(1 << servo_pin[servo_index]); break;
    case 4: PORTD &= ~(1 << servo_pin[servo_index]); break;
    }
    servo_index = (servo_index + 1) % num_servos;
    pulse_width = servo_table[servo_index];
    OCR1A = pulse_width;
}


ISR(TIMER1_OVF_vect)
{
    // start of new servo cycle, assert servo pin of interest
    switch(servo_port[servo_index]) {
    case 2: PORTB |= (1 << servo_pin[servo_index]); break;
    case 3: PORTC |= (1 << servo_pin[servo_index]); break;
    case 4: PORTD |= (1 << servo_pin[servo_index]); break;
    }
}
