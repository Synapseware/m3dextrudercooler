#include "board.h"


//----------------------------------------------------------------
// Configures Timer0 for 1kHz interrupt
void ConfigureSystemTimer(void)
{
	// For 25kHz output
	// 8MHz / 64 / 125 = 1000Hz

	TCCR0A	=	(1<<WGM01)	|	// CTC
				(0<<WGM00);

	TCCR0B	=	(0<<WGM02)	|	// CTC
				TC0_CLK_64;

	TIMSK	|=	TC0_IE_COMPA;		// Compare A interrupts

	OCR0A	=	125-1;				// Set for 1kHz
	OCR0B	=	64;
}


//----------------------------------------------------------------
// Configures Timer1 for PWM output (25kHz)
void ConfigurePWMOutputTimer(void)
{
	// 8Mhz / 8 / 40 = 25kHz
	TCCR1	=	TC1_PWM1A	|	// Enable PWM Channel A
				(1<<COM1A1)	|	// OC1A set on match
				(0<<COM1A0)	|	// /OC1A disconnected
				TC1_CLK_1;		// CLK/1

	// Setup PLLCSR - note: PLL is enabled via fuses because we are driving uC clock from the PLL! (pg 97)
	PLLCSR	=	0;

	PWM_REG	=	128;			// Set default PWM duty cycle
	PWM_TOP	=	255;			// PWM range

	DDRB	|=	(1<<PWM_OUTPUT);	// Enable PWM output
}


//----------------------------------------------------------------
// Setup ADC to read from the temp sensor.
void ConfigureADC(uint8_t channel)
{
	ADMUX	=	ADC_RIGHTADJ|	// Right adjust result
				ADC_VREF_VCC;	// Vcc as Aref

	ADCSRA	=	ADC_ENABLE	|	// ADC Enable
				ADC_INT_EN	|	// Enable interrupts
				ADC_CLK_128;	// Fcpu/128

	ADCSRB	=	0;				// Nothing to set here

	SelectADCChannel(channel);
}


//----------------------------------------------------------------
// Selects the specified ADC channel
void SelectADCChannel(uint8_t channel)
{
	switch (channel)
	{
		case 0x00:
			// Channel 0
			ADMUX = (ADMUX & 0xF0) | (0<<MUX3) | (0<MUX2) | (0<<MUX1) | (0<<MUX0);
			DIDR0 = (1<<ADC0D);
			DDRB &= ~(1<<PB5);
			break;
		case 0x01:
			// Channel 1
			ADMUX = (ADMUX & 0xF0) | (0<<MUX3) | (0<MUX2) | (0<<MUX1) | (1<<MUX0);
			DIDR0 = (1<<ADC1D);
			DDRB &= ~(1<<PB2);
			break;
		case 0x02:
			// Channel 2
			ADMUX = (ADMUX & 0xF0) | (0<<MUX3) | (0<MUX2) | (1<<MUX1) | (0<<MUX0);
			DIDR0 = (1<<ADC2D);
			DDRB &= ~(1<<PB4);
			break;
		case 0x03:
			// Channel 3
			ADMUX = (ADMUX & 0xF0) | (0<<MUX3) | (0<MUX2) | (1<<MUX1) | (1<<MUX0);
			DIDR0 = (1<<ADC3D);
			DDRB &= ~(1<<PB3);
			break;
		case 0x0C:
			// Band-gap voltage
			ADMUX = (ADMUX & 0xF0) | (1<<MUX3) | (1<MUX2) | (0<<MUX1) | (0<<MUX0);
			break;
		case 0x0D:
			// Ground
			ADMUX = (ADMUX & 0xF0) | (1<<MUX3) | (1<MUX2) | (0<<MUX1) | (1<<MUX0);
			break;
		case 0x0F:
			// Internal temperature sensor
			ADMUX = (ADMUX & 0xF0) | (1<<MUX3) | (1<MUX2) | (1<<MUX1) | (1<<MUX0);
			break;
	}
}


//----------------------------------------------------------------
// Configure Debug LED
void ConfigureDebugLed(void)
{
	DDRB |= (1<<LED_DBG);
	PORTB &= ~(1<<LED_DBG);
}
