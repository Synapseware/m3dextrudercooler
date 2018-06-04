#include "fanctrl.h"

volatile uint8_t	tFast		= 0;
volatile uint8_t	tSeconds	= 0;
volatile int	 	adcData		= 0;
volatile uint8_t	state		= 0;


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
// Setup ADC to read from the temp sensor.
static void ConfigureADC(void)
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
	DIDR0		= 0;
	tFast		= 0;
	tSeconds	= 0;
	adcData		= 0;
	state		= STATE_INIT;

	// setup hardware
	ConfigureSystemTimer();
	ConfigurePWMOutputTimer();
	ConfigureADC();
	SelectADCChannel(ADC_CHANNEL);

	sei();
}


//----------------------------------------------------------------
// Flags the ADC to start a conversion
static void StartConversion(void)
{
	// start an ADC conversion
	ADCSRA |= (1<<ADSC);
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
static uint8_t ConvertToCelcius(int adcSample)
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

	return (uint8_t)(adcSample * (1024.0 / 125.0));
}


//----------------------------------------------------------------
// Maps the temperature to a fan speed (between 0-100)
// 49 & less = 0%
// 50 - 99 = 50% to 100%
// 100 & more = 100%
static uint8_t MapFanSpeed(uint8_t temperature)
{
	uint8_t map = 0;
	if (temperature < 50)
	{
		map = 0;
	}
	else if (temperature < 100)
	{
		map = (uint8_t)((2.5 * (temperature - 50)) + 128);
	}
	else
	{
		map = 100;
	}

	return map;
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
// Sets the current processing state
static void SetState(uint8_t whatState)
{
	// set the state
	state = whatState;
}


//----------------------------------------------------------------
// Gets the current processing state
static uint8_t GetState(void)
{
	// return the state
	return state;
}


//----------------------------------------------------------------
// Processes the statemachine which responds to temperature changes
static void ProcessStateMachine(void)
{
	static int decay			= 0;
	static uint8_t lastTemp		= 0;

	// read the current temperature and map it to a fan speed
	uint8_t currentTemp = ConvertToCelcius(GetLatestAdcData());
	uint8_t speed = MapFanSpeed(currentTemp);

	switch (GetState())
	{
		case STATE_INIT:
			SetFanSpeed(100);
			decay = 5;
			SetState(STATE_DECAY);
			break;

		case STATE_HOT:
			decay = 30;
			SetFanSpeed(speed);
			SetState(STATE_HOT_NXT);
			break;

		case STATE_HOT_NXT:
			// current temp has to keep dropping until the decay is 0
			if (decay > 0)
			{
				if (currentTemp < lastTemp)
				{
					// hold speed here
					decay--;
				}
				else
				{
					// let the fan speed increase
					SetFanSpeed(speed);
				}

				decay--;
				break;
			}

			SetState(STATE_NEXT);
			break;

		case STATE_COOL:
			// just set the fan speed and allow the system to throttle
			// down as necessary
			SetFanSpeed(speed);
			SetState(STATE_NEXT);
			break;

		case STATE_DECAY:
			if (--decay > 0)
				break;
			SetState(STATE_NEXT);
			break;

		case STATE_NEXT:
			// figure out if the current level of cooling is enough
			if (currentTemp > lastTemp)
			{
				SetState(STATE_HOT);
			}
			else
			{
				SetState(STATE_COOL);
			}

			break;
	}

	// save the current temp
	lastTemp = currentTemp;
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
			StartConversion();

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
			// Set the fan speed based on temperature
			SetFanSpeed
			(
				MapFanSpeed
				(
					ConvertToCelcius
					(
						GetLatestAdcData()
					)
				)
			);

			// start another ADC conversion
			StartConversion();

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
