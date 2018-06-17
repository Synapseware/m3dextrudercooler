#include "main.h"


static int adcData = 0;

void ProcessADCValue(int sample)
{
	// basic moving average
	//adcData = (sample>>2) + adcData - (adcData>>2);
	adcData = sample;
}

int GetLatestAdcData(void)
{
	// just return the ADC data
	return adcData;
}

uint8_t ConvertToCelcius(int sample)
{
	// Convert sample to temperature in Celcius
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
		map = 0.0408 * pow(temperature - 50, 2) + 153;;
	}
	else
	{
		map = 255;
	}

	return map;
}


void SetFanSpeed(uint8_t speed)
{
	if (speed > 0)
	{
		//DDRB |= (1<<PWM_OUTPUT);
		//cout << "  enabling PWM output" << endl;

		//OCR1A = dutyCycle;
		cout << "    setting PWM to: " << unsigned(speed) << endl;
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
		cout << "    fan speed: " << unsigned(100*speed/255.0) << "%" << endl;

		SetFanSpeed(speed);

	} while (inputValue > -1);

	cout << "Done" << endl;

	return 0;
}
