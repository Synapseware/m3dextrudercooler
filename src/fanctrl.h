#ifndef __FANCTRL_H__
#define __FANCTRL_H__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>


//--------------------------------------------------
// Board constants
#define DELAY			100
#define	ADC_CHANNEL		3
#define ADC_SMOOTHING	4
#define LED_DBG			PB4
#define PWM_OUTPUT		PB0
#define PWM_REG			OCR0A
#define INITIAL_PWM		179

//-----------------------------------------------------------------------------
// ADC values
// ADMUX
#define ADC_VREF_VCC	0
#define ADC_VREF_EXT	((0<<REFS2) | (0<<REFS1) | (1<<REFS0))
#define ADC_VREF_INT11	((0<<REFS2) | (1<<REFS1) | (0<<REFS0))
#define ADC_VREF_256	((1<<REFS2) | (1<<REFS1) | (0<<REFS0))
#define ADC_VREF_256BP	((1<<REFS2) | (1<<REFS1) | (1<<REFS0))
#define ADC_CHANNEL_0	((0<<MUX3) | (0<MUX2) | (0<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_1	((0<<MUX3) | (0<MUX2) | (0<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_2	((0<<MUX3) | (0<MUX2) | (1<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_3	((0<<MUX3) | (0<MUX2) | (1<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_BGP	((1<<MUX3) | (1<MUX2) | (0<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_GND	((1<<MUX3) | (1<MUX2) | (0<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_TMP	((1<<MUX3) | (1<MUX2) | (1<<MUX1) | (1<<MUX0))

#define ADC_CLK_1		((0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_2		((0<<ADPS2) | (0<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_4		((0<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_8		((0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_16		((1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_32		((1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_64		((1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_128		((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))

// ADCSRA
#define ADC_ENABLE		(1<<ADEN)
#define ADC_NO_INTS		0
#define ADC_INT_EN		(1<<ADIE)
#define ADC_AUTOTRG		(1<<ADATE)

// ADCSRB
#define ADC_TRG_FREE	((0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_COMP	((0<<ADTS2) | (0<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_EI0		((0<<ADTS2) | (1<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_TC0A	((0<<ADTS2) | (1<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_TC0F	((1<<ADTS2) | (0<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_TC0B	((1<<ADTS2) | (0<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_PCIR	((1<<ADTS2) | (1<<ADTS1) | (0<<ADTS0))
#define ADC_LEFTADJ		((1<<ADLAR))
#define ADC_RIGHTADJ	0


//-----------------------------------------------------------------------------
// USI Interrupts
#define USI_NO_INTS		0
#define USI_START_IE	(1<<USISIE)
#define USI_CNT_OVF_IE	(1<<USIOIE)

// USI Mode
#define USI_DISABLED	0
#define USI_THREE_WIRE	((0<<USIWM1) | (1<<USIWM0))
#define USI_TWO_WIRE	((1<<USIWM1) | (0<<USIWM0))
#define USI_TWO_ALT		((1<<USIWM1) | (1<<USIWM0))

// USI clock is bits 3:2:1
#define USI_CLK_OFF		0
#define USI_CLK_SOFT	((0<<USICS1) | (0<<USICS0) | (1<<USICLK))
#define USI_CLK_TC0		((0<<USICS1) | (1<<USICS0) | (0<<USICLK))
#define USI_CLK_EXT_PE	((1<<USICS1) | (0<<USICS0) | (0<<USICLK))
#define USI_CLK_EXT_NE	((1<<USICS1) | (1<<USICS0) | (0<<USICLK))
#define USI_CLK_SOFT_PE	((1<<USICS1) | (0<<USICS0) | (1<<USICLK))
#define USI_CLK_SOFT_NE	((1<<USICS1) | (1<<USICS0) | (1<<USICLK))


#endif
