#include "main.h"


static int adcData = 0;

void ProcessADCValue(int sample)
{
	// basic moving average
	adcData = sample; //(sample>>2) + adcData - (adcData>>2);
}

int GetLatestAdcData(void)
{
	// just return the ADC data
	return adcData;
}

uint8_t ConvertToCelcius(int sample)
{
	// Convert sample to temperature in Celcius
	//return (uint8_t) ((sample * (5.0/1024.0)) / (5.0 / 125.0));

	return (uint8_t) (sample * (5.0/1024.0) * 25);
}


uint8_t MapFanSpeed(uint8_t temperature)
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


void SetFanSpeed(uint8_t speed)
{
	if (speed > 0)
	{
		//DDRB |= (1<<PWM_OUTPUT);
		//cout << "  enabling PWM output" << endl;

		uint8_t dutyCycle = (speed * 2.04) + 51;
		//OCR1A = dutyCycle;
		float ratio = (dutyCycle/256.0);
		cout << "    setting PWM duty cycle to: " << unsigned(dutyCycle);
		cout << " (" << ratio << ")" << endl;
	}
	else
	{
		//DDRB &= ~(1<<PWM_OUTPUT);
		cout << "    disabling PWM output" << endl;
	}
}

int main(int argc, char* argv[])
{
	int inputValue = 0;
	do
	{
		//
		cout << "Enter raw ADC value [0-1023]: ";
		cin >> inputValue;
		if (inputValue < 0 || inputValue > 1023)
			break;

		cout << "You entered: " << inputValue << endl;

		// begin simulation
		ProcessADCValue(inputValue);
		cout << "    ADC value: " << GetLatestAdcData() << endl;

		uint8_t temperature = ConvertToCelcius(GetLatestAdcData());
		cout << "    degress Celcius: " << unsigned(temperature) << endl;

		uint8_t speed = MapFanSpeed(temperature);
		cout << "    fan speed %: " << unsigned(speed) << endl;

		SetFanSpeed(speed);

	} while (inputValue > -1);

	cout << "Done" << endl;

	return 0;
}
