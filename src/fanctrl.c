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
	// disable interrupts
	cli();

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
	ConfigureADC(ADC_CHANNEL);
	ConfigureDebugLed();

	// enable interrupts
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
static void ProcessADCValue(int sample)
{
	// basic moving average
	adcData = (sample>>2) + adcData - (adcData>>2);
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
static uint8_t ConvertToCelcius(int sample)
{
	// Temperature sensor is a TMP35

	// ADC transfer function:
	// ADC value = (Vin * 1024) / Vref
	//			Vin * 1024
	// sample = ----------   or,
	//			   5.0v
	// 
	// Vin = 5.0v/1024
	//
	// TMP35 temperature range is 10-125/C
	// 		Output voltage is 0.10 - 1.25
	//		Output calibrated directly in degrees C
	//
	// The board has an op-amp which essentially multiplies the
	// output from the TMP35 by 4.  So, full voltage range being
	// input to the ADC is 0.4v to 5.0v
	//
	// TMP35 transfer function is 5.0/125, or 40mV/C (0.04V/C)
	//
	// To convert from an ADC sample to a temperature (Tc), we need:
	//
	// Tc = (sample * 5.0v/1024) / (5.0v / 125C)
	//
	// Constant portion of the ADC to temp. transfer function is about
	// 0.12207
	// Ex1: sample = 712: Temp = (712)(0.12207) => 86.9 => 86C
	// Ex2: sample = 454: Temp = (454)(0.12207) => 55.4 => 54C

	return (uint8_t) ((sample * (5.0/1024.0)) / (5.0 / 125.0));
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
		map = temperature;
	}
	else
	{
		map = 100;
	}

	return map;
}


//----------------------------------------------------------------
// Sets the PWM duty cycle based on desired fan speed %
static void SetFanSpeed(uint8_t speed)
{
	if (speed > 0)
	{
		DDRB |= (1<<PWM_OUTPUT);

		uint8_t dutyCycle = (speed * 2.04) + 51;
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

	SetFanSpeed(speed);
	return;

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
		// Runs at 1kHz
		if (tFast)
		{
			// start another ADC conversion
			StartConversion();
			tFast = 0;
		}

		// Runs at 1s
		if (tSeconds)
		{
			PORTB |= (1<<LED_DBG);

			// process machine state
			ProcessStateMachine();

			PORTB &= ~(1<<LED_DBG);

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
	static uint16_t delay = DELAY;

	tFast = 1;

	if (!(--delay))
	{
		delay = DELAY;
		tSeconds = 1;
	}
}
