#include "juice.h"
#include "svc-adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t adc[2]={15,30};

void adc_init()
{
    // Set clock prescaler to 128
    ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    // Right adjust for 8 bit resolution
    ADMUX |= (1 << ADLAR);

    // Set auto-trigger enable
    ADCSRA |= (1 << ADATE);
    // 0 for free running mode
	ADCSRB = 0;

    // Enable the ADC and enable the interrupt
    ADCSRA |= (1<<ADEN) | (1 << ADIE);
}

/**
 * Start A2D Conversions
 */
void adc_start()
{
	ADCSRA |= (1 << ADSC);
}

/**
 * Stop A2D Conversions
 */
void adc_stop()
{
	ADCSRA &= ~(1 << ADSC);
}


ISR(ADC_vect)
{
	if (ADMUX & MUX0) {
		adc[1] = ADCH;
		ADMUX &= ~(1 << MUX0);
	} else {
		adc[0] = ADCH;
		ADMUX |= (1 << MUX0);
	}
}
