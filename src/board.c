#include "board.h"


//----------------------------------------------------------------
// Configures Timer1 for 1Hz interrupt
void ConfigureSystemTimer(void)
{
	// For 25kHz output
	// 6.4MHz / 512 / 125 = 100Hz

	// Setup PLLCSR - note: PLL is enabled via fuses because we are driving uC clock from the PLL! (pg 97)
	PLLCSR	=	0;

	GTCCR	|=	(0<<PWM1B)	|		// 
				(0<<TSM)	|		// 
				(0<<COM1A1);		// 

	TCCR1	=	(1<<CTC1)	|		// CTC mode
				(0<<COM1A1)	|		// 
				(0<<COM1A0)	|		// 
				(0<<PWM1A)	|		// 
				(1<<CS13)	|		// PCK/512
				(0<<CS12)	|		// 
				(1<<CS11)	|		// 
				(0<<CS10);			// 

	TIMSK	|=	(1<<OCIE1A)	|		// Enable CompareA int.
				(0<<OCIE1B)	|		// 
				(0<<TOIE1);			// 

	OCR1A	=	125-1;				// Set for 100Hz
}


//----------------------------------------------------------------
// Configures Timer0 for PWM output (25kHz)
void ConfigurePWMOutputTimer(void)
{
	// 6.4Mhz / 256 = 25kHz

	GTCCR	|=	(0<<TSM)	|	// 
				(0<<PSR0);		// 

	TCCR0A	=	(1<<COM0A1)	|	// Normal output on OC0A
				(0<<COM0A0)	|	// 
				(0<<COM0B1)	|	// 
				(0<<COM0B0)	|	// 
				(1<<WGM01)	|	// Fast PWM
				(1<<WGM00);		// Fast PWM

	TCCR0B	=	(0<<FOC0A)	|	// 
				(0<<FOC0B)	|	// 
				(0<<WGM02)	|	// Fast PWM
				(0<<CS02)	|	// CLK/1
				(0<<CS01)	|	// 
				(1<<CS00);		// 

	TIMSK	|=	(0<<OCIE0A)	|	// No interrupts
				(0<<OCIE0B)	|	// 
				(0<<TOIE0);		// 


	PWM_REG	=	INITIAL_PWM;	// Set default PWM duty cycle

	DDRB	|=	(1<<PWM_OUTPUT);	// Enable PWM output
}


//----------------------------------------------------------------
// Setup ADC to read from the temp sensor.
void ConfigureADC(void)
{
	ADMUX	=	ADC_RIGHTADJ|	// Right adjust result
				ADC_VREF_VCC;	// Vcc as Aref

	ADCSRA	=	ADC_ENABLE	|	// ADC Enable
				ADC_INT_EN	|	// Enable interrupts
				ADC_CLK_64;		// Fcpu/64

	ADCSRB	=	0;				// Nothing to set here
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


