#include "fanctrl.h"
#include "board.h"

volatile uint8_t	tFast		= 0;
volatile uint8_t	tSeconds	= 0;
volatile int	 	adcData		= 0;
volatile uint8_t	state		= 0;


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
			// process machine state
			ProcessStateMachine();

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
