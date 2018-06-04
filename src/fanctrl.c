#include "fanctrl.h"

volatile uint8_t	tFast		= 0;
volatile uint8_t	tSeconds	= 0;
volatile int	 	adcData		= 0;



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
// Setup ADC to read from the temp sensor.  External analog reference
// voltage is 2.5V
static void initADC(void)
{
	ConfigureADC(ADC_CHANNEL);

	ADMUX	=	(0<<ADLAR);		// Right adjust result

	ADCSRA	=	(1<<ADEN)	|	// ADC Enable
				(0<<ADSC)	|	// 
				(0<<ADATE)	|	// 
				(0<<ADIF)	|	// 
				(1<<ADIE)	|	// Enable interrupts
				(1<<ADPS2)	|	// Prescaler of 64
				(1<<ADPS1)	|	// 8MHz / 64 = 125kHz ADC clock
				(0<<ADPS0);		// ...

	ADCSRB	=	(0<<ACME)	|	// Disable the analog comparator
				(0<<ADTS2)	|	// Free running mode
				(0<<ADTS1)	|	// ...
				(0<<ADTS0);		// ...
}


//----------------------------------------------------------------
// Configures Timer1 for 1Hz interrupt
static void ConfigureSystemTimer(void)
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
static void ConfigurePWMOutputTimer(void)
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
// 
static void init(void)
{
	// set defaults
	GTCCR		= 0;
	TIMSK		= 0;
	TIFR		= 0;
	DDRB		= 0;
	PORTB		= 0;
	tFast		= 0;
	tSeconds	= 0;
	adcData		= 0;

	// setup hardware
	ConfigureSystemTimer();
	ConfigurePWMOutputTimer();
	SelectADCChannel(ADC_CHANNEL);

	sei();
}


//----------------------------------------------------------------
// Smooths the ADC sample by averaging readings
static void ProcessADCValue(int adcSample)
{
	// basic moving average
	adcData = (adcSample>>2) + adcData - (adcData>>2);
}


//----------------------------------------------------------------
// 
static int GetLatestAdcData(void)
{
	// just return the ADC data
	return adcData;
}


//----------------------------------------------------------------
// Converts the value from the temperature sensor to degrees C
static int ConvertToCelcius(int adcSample)
{
	// Temperature sensor is a TMP35

	// ADC transfer function:
	// ADC value = (Vin * 1024) / Vref
	//			Vin * 1024
	// result = ----------
	//			   5.0v
	// 
	// TMP35 temperature range is 10-125/C
	// 
	// TMP35 transfer function:
	//           1024
	// result = ------ = 8.192
	//            125

	return adcSample * (1024 / 125);
}


//----------------------------------------------------------------
// Maps the temperature to a fan speed (between 0-100)
// 49 & less = 0%
// 50 - 99 = 50% to 100%
// 100 & more = 100%
static uint8_t MapFanSpeed(int temperature)
{
	if (temperature < 50)
	{
		return 0;
	}
	
	if (temperature > 99)
	{
		return 100;
	}
	
	return (uint8_t)((2.5 * (temperature - 50)) + 128);
}


//----------------------------------------------------------------
// Sets the PWM duty cycle based on desired fan speed
static void SetFanSpeed(uint8_t speed)
{
	if (speed > 0)
	{
		DDRB |= (1<<PWM_OUTPUT);

		uint8_t dutyCycle = 255 - (speed * 2.56);
		PWM_REG = dutyCycle;
	}
	else
	{
		DDRB &= ~(1<<PWM_OUTPUT);
	}
}


//----------------------------------------------------------------
// 
int main(void)
{
	init();

	// Initial delay to allow system to stabilize
	// and ADC to get an average going
	int delay = 15;		// 15 seconds
	SetFanSpeed(100);
	while(1)
	{
		if (tSeconds && delay)
		{
			// establish averages and let temp. stabilize
			ADCSRA |= (1<<ADSC);

			delay--;
			tSeconds = 0;
		}
	}

	// Enter main temperature processing loop
	while(1)
	{
		// Runs at 100Hz
		if (tFast)
		{
			tFast = 0;
		}

		// Runs at 1s
		if (tSeconds)
		{
			// convert the ADC data to Celcius
			int temperature = ConvertToCelcius(GetLatestAdcData());

			// update the resistor ladder with new datate
			SetFanSpeed(MapFanSpeed(temperature));

			// start another ADC conversion
			ADCSRA |= (1<<ADSC);

			// reset the tick
			tSeconds = 0;
		}
	}

	return 0;
}


//----------------------------------------------------------------
// ADC interrupt complete handler
ISR(ADC_vect)
{
	// just read the ADCH register (top 8 bits)
	ProcessADCValue(ADC);
}


//----------------------------------------------------------------
// Timer 0 compare A interrupt handler @ 100Hz
ISR(TIMER0_COMPA_vect)
{
	static uint8_t delay = DELAY;

	tFast = 1;

	if (!delay)
	{
		delay = DELAY;
		tSeconds = 1;
	}
}
