/***********************************************************************
 * servo.c
 * 4-port RC servo control for Raspy Juice (Raspberry Pi Exp Board)
 * Using PC0, PC1, PC2, PC3
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
#include <avr/pgmspace.h>

/* For servo_port[], PORTA = 1, PORTB = 2, etc */
uint8_t servo_port[12]		= {    3,    3,    3,    3 };
uint8_t servo_pin [12]		= {    0,    1,    2,    3 };
volatile int servo_table[12]	= { 1382, 1382, 1382, 1382 };
volatile uint8_t num_servos = 4;
volatile uint8_t servo_index = 0;

void servo_init(void)
{
	// Init timer1 for servo-control:
	// Resolution = cpuclk / prescale8 = 7.3728MHz = 1.085us
	// Servo index change-rate = 2765 * above = 3ms
	OCR1A = 1382;
	ICR1 = 2765;		// =TOP, frequency of servo-pin change
	TCCR1A = 0b00000010;	// WGM11,10 = Fast PWM, TOP=ICR1
	TCCR1B = 0b00011010;	// WGM13,12 = Fast PWM, prescale clk/8
	TIFR1  = 0b00100111;
	TIMSK1 = (1<<OCIE1A) | (1<<TOIE1); // Enable Ovrflw & Match-Comp irqs
}

void servo_set(uint8_t chan, int usec)
{
	if (usec <  600) usec =  600;
	if (usec > 2400) usec = 2400;
	/* how did i get 9216 from 7.3728MHz cpu clock? */
	long corrected = (long)usec * 9216 / 10000;
	servo_table[(int)chan] = (int)corrected;
}

/***********************************************************************
 * Servo Interrupts
 ***********************************************************************/
ISR(TIMER1_COMPA_vect)
{
#if 1
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
#endif
}


ISR(TIMER1_OVF_vect)
{
#if 1
	// start of new servo cycle, assert servo pin of interest
	switch(servo_port[servo_index]) {
	case 2: PORTB |= (1 << servo_pin[servo_index]); break;
	case 3: PORTC |= (1 << servo_pin[servo_index]); break;
	case 4: PORTD |= (1 << servo_pin[servo_index]); break;
	}
#endif
}


#ifdef STANDALONE_SERVO_TEST

long led_timing[4] = { 100000L, 20000L, 10000L, 20000L }; 
long led_counter = 0;
uint8_t led_state = 0;
void led_heartbeat(void)
{
	led_counter++;
	if (led_counter > led_timing[led_state]) {
		led_state++;
		led_counter = 0;
		if (led_state > 3)
			led_state = 0;
		if (led_state % 2)
			LED_ON();
		else
			LED_OFF();
	}
}

FILE rs232_stream = FDEV_SETUP_STREAM(rs232_putchar, rs232_getchar, 
				      _FDEV_SETUP_RW);
int main(void)
{
	char c;
	
	JUICE_PCBA_PINS_INIT();
	rs232_swuart_init();
	servo_init();
    
	sei();
	
	stdout = stdin = &rs232_stream;
	printf_P(PSTR("\nStandalone Servo Test Application of Raspy Juice\n"));
	printf_P(PSTR("Second line\n\n"));
	
	while(1) {

		led_heartbeat();

		if (rs232_havechar()) {
			c = rs232_getc();
			rs232_putc(c);
		}
		else
			continue;
		
		switch(c) {
		case '1': LED_OFF();	break;
		case '2': LED_ON();	break;
			
		case 'z': servo_set(0, 2000); break;
		case 'a': servo_set(0, 1500); break;
		case 'q': servo_set(0, 1000); break;
			
		case 'x': servo_set(1, 2000); break;
		case 's': servo_set(1, 1500); break;
		case 'w': servo_set(1, 1000); break;
			
		case 'e': servo_set(2, 2000); break;
		case 'd': servo_set(2, 1500); break;
		case 'c': servo_set(2, 1000); break;
			
		case 'r': servo_set(3, 2000); break;
		case 'f': servo_set(3, 1500); break;
		case 'v': servo_set(3, 1000); break;
			
		default: BLINK2(50); break;
		}
	}
	return 0;
}
#endif

