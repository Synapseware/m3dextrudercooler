M3D Pro and Micro Extruder Cooler
=================================

## About
The M3D Pro (and micro to some extent) suffer from some really terrible design decisions.  For example, after printer for about 15-20 minutes on the Pro, the extruder usually heats up enough to melt the filament, which of course causes the print to fail and the extruder gear to get all jammed up with melted filament.

This cooler aims to solve that by monitoring the temperature at or near the extruder gear and directing a fan to blow more or less depending on how hot it is.

# MCU
The controlling chip is an ATTiny25/45/85 running around 8MHz and driven from a 5V supply (using a USB-wall wort, LM7805, etc.).

## ADC Input
The cooler samples the ADC input every 10 seconds or so, and ramps up fan speed based on temperature.

### ADC Input Pin
The ADC is configured to read from pin PB3.

### ADC Sense Line
The input to the ADC should be constrained to 0.0v through 1.1v.  This gives the most accuracy on the ADC and allows for a small load/sense resistor.

## PWM Output
Timer1 is used to generate the PWM signal that drives the MOSFET which is connected to the motor.  The switching frequency is set for best performance with the given fan, and will need to be tuned in firmware.

### PWM Output Pin
The PWM output is available on PB1.  The output should not be filtered since it must be fed to the MOSFET switch.  Care should be taken to keep this line short and include an appropriately sized pull-down resistor to ensure safe operation of the power MOSFET or other switching transistor.

## Other Chip Information
The AVR uC is configured to use it's internal R/C oscillator.  High clock accuracy is not needed by this circuit and keeping the part count low is a goal of this example.


## Build
This project compiles and uploads just fine using Linux and avr-gnu.
