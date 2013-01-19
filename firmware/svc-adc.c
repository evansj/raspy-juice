#include "juice.h"
#include "svc-adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile int adc[]={15,30};

void adc_init()
{
    // Set clock prescaler to 128
    ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	ADMUX |= (1 << REFS0); // Set ADC reference to AVCC

    // Right adjust for 8 bit resolution
    // ADMUX |= (1 << ADLAR);

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
    // Read ADC6 first
    ADMUX |= 6;
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
	uint8_t mux, reg;           // temp registers for storage of misc data

    mux = ADMUX;            	// read the value of ADMUX register
    reg = mux & 0x07;           // AND the first 3 bits (value of ADC pin being used)

	int ADCval;
	ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    if (reg == 6) {
        adc[0] = ADCval;
        // adc[0]++;
        mux = ADMUX;
        mux &= 0xF8; // clear last 3 bits
        ADMUX = mux | 7;
    } else {
        adc[1] = ADCval;
        // adc[1]++;
        mux = ADMUX;
        mux &= 0xF8; // clear last 3 bits
        ADMUX = mux | 6;
    }
}
