#include "fanctrl.h"
#include "board.h"

volatile uint8_t	tFast		= 0;
volatile uint8_t	tSeconds	= 0;
volatile int	 	adcData		= 0;
volatile uint8_t	state		= 0;

Events events(8);

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
static void StartConversion(eventState_t state)
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

	// An easier solution, is this transfer function:
	//			(ADCresult)(Vref)
	// Ftemp = ------------------(25)
	//				1024
	//
	//return (uint8_t) ((sample * (5.0/1024.0)) / (5.0 / 125.0));

	return (uint8_t) (sample * (5.0/1024.0) * 25);
}


//----------------------------------------------------------------
// Maps the temperature to a fan speed (between 0-100)
// 49 & less = 0%
// 50 - 99 = 50% to 100%
// 100 & more = 100%
static uint8_t MapFanSpeed(uint8_t temperature)
{
	// The motor is off at temperatures below 50/C and on at
	// 100% for all speeds from 100/C and up.
	// For 50/C to 99/C, the fan speed is controlled by a parabolic
	// curve.
	// The parabolic speed curve for the motor temps is defined by 
	// a parabola having vertex (50,153) and a point of (100,255).
	// y=a(x-h)^2+k
	// (h,k) = (50,153)
	// (x,y) = (100,255)
	// a = 102/2500 => 0.0408
	// y= 0.0408(x-50)^2+153

	uint8_t map = 0;
	if (temperature < 50)
	{
		// turn the fan off for all temps below 50/C
		map = 0;
	}
	else if (temperature < 100)
	{
		// map the temperature to fan speed
		map = 0.0408 * pow(temperature - 50, 2) + 153;
	}
	else
	{
		// max
		map = 255;
	}

	return map;
}


//----------------------------------------------------------------
// Sets the PWM duty cycle based on desired fan speed %
static void SetFanSpeed(uint8_t speed)
{
	// This needs to convert a speed percentage to a workable
	// duty cycle for the fan.  Typically, this is somewhere
	// between 60% - 100%
	// The speed is provided as some value between 50% and 100%
	if (speed > 0)
	{
		DDRB |= (1<<PWM_OUTPUT);
		PWM_REG = speed;
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
static void ProcessStateMachine(eventState_t state)
{
	static int decay			= 0;
	static uint8_t lastTemp		= 0;

	//PINB |= (1<<LED_DBG);

	// read the current temperature and map it to a fan speed
	uint8_t currentTemp = ConvertToCelcius(GetLatestAdcData());
	uint8_t speed = MapFanSpeed(currentTemp);

	switch (GetState())
	{
		case STATE_INIT:
			SetFanSpeed(100);
			decay = 5;
			lastTemp = currentTemp;
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
					decay = 30;
				}

				decay--;
			}
			else
			{
				SetState(STATE_NEXT);
			}
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
			if (currentTemp > lastTemp + 3)
			{
				SetState(STATE_HOT);
			}
			else
			{
				SetState(STATE_COOL);
			}

			// save the current temp
			lastTemp = currentTemp;

			break;
	}
}


//----------------------------------------------------------------
// 
static void ledOn(eventState_t state)
{
	PORTB |= (1<<LED_DBG);
}
static void ledOff(eventState_t state)
{
	PORTB &= ~(1<<LED_DBG);
}


//----------------------------------------------------------------
// 
int main(void)
{
    // let the Events instance know what the period size is
    events.setTimeBase(1000);

    // Register a function to toggle an I/O pin to turn an LED on and off
    events.registerEvent(ProcessStateMachine, 1000, EVENT_STATE_NONE);
    events.registerEvent(StartConversion, 100, EVENT_STATE_NONE);
    events.registerEvent(ledOn, 1000, EVENT_STATE_NONE);
    events.registerEvent(ledOff, 1100, EVENT_STATE_NONE);

	init();

	while(1)
	{
		// handle events
		events.doEvents();
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
    events.sync();
}
