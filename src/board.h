#ifndef __BOARD_H__
#define __BOARD_H__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>



void ConfigureSystemTimer(void);
void ConfigurePWMOutputTimer(void);
void ConfigureADC(void);
void SelectADCChannel(uint8_t channel);
void ConfigureDebugLed(void);




//--------------------------------------------------
// Board constants
#define	ADC_CHANNEL		3
#define ADC_PIN			PB3
#define ADC_SMOOTHING	4
#define ADC_DIGITAL		ADC3D
#define LED_DBG			PB4
#define PWM_OUTPUT		PB1




//-----------------------------------------------------------------------------
// ADC values
// ADMUX
#define ADC_VREF_VCC	0
#define ADC_VREF_EXT	((0<<REFS2) | (0<<REFS1) | (1<<REFS0))
#define ADC_VREF_INT11	((0<<REFS2) | (1<<REFS1) | (0<<REFS0))
#define ADC_VREF_256	((1<<REFS2) | (1<<REFS1) | (0<<REFS0))
#define ADC_VREF_256BP	((1<<REFS2) | (1<<REFS1) | (1<<REFS0))
#define ADC_LEFTADJ		((1<<ADLAR))
#define ADC_RIGHTADJ	0
#define ADC_CHANNEL_0	((0<<MUX3) | (0<MUX2) | (0<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_1	((0<<MUX3) | (0<MUX2) | (0<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_2	((0<<MUX3) | (0<MUX2) | (1<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_3	((0<<MUX3) | (0<MUX2) | (1<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_BGP	((1<<MUX3) | (1<MUX2) | (0<<MUX1) | (0<<MUX0))
#define ADC_CHANNEL_GND	((1<<MUX3) | (1<MUX2) | (0<<MUX1) | (1<<MUX0))
#define ADC_CHANNEL_TMP	((1<<MUX3) | (1<MUX2) | (1<<MUX1) | (1<<MUX0))

// ADCSRA
#define ADC_ENABLE		(1<<ADEN)
#define ADC_NO_INTS		0
#define ADC_INT_EN		(1<<ADIE)
#define ADC_AUTOTRG		(1<<ADATE)
#define ADC_CLK_1		((0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_2		((0<<ADPS2) | (0<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_4		((0<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_8		((0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_16		((1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_32		((1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0))
#define ADC_CLK_64		((1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))
#define ADC_CLK_128		((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))

// ADCSRB
#define ADC_BIN			((1<<BIN))
#define ADC_IPR			((1<<IPR))
#define ADC_TRG_FREE	((0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_COMP	((0<<ADTS2) | (0<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_EI0		((0<<ADTS2) | (1<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_TC0A	((0<<ADTS2) | (1<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_TC0F	((1<<ADTS2) | (0<<ADTS1) | (0<<ADTS0))
#define ADC_TRG_TC0B	((1<<ADTS2) | (0<<ADTS1) | (1<<ADTS0))
#define ADC_TRG_PCIR	((1<<ADTS2) | (1<<ADTS1) | (0<<ADTS0))


//-----------------------------------------------------------------------------
// Timer0
// GTCCR
#define TC0_SYNC		((1<<TSM))
#define TC0_PSR			((1<<PSR0))

// TCCR0A
#define TC0_OUTA_NRM	((0<<COM0A1) | (0<<COM0A0))
#define TC0_OUTA_TGL	((0<<COM0A1) | (1<<COM0A0))
#define TC0_OUTA_CLR	((1<<COM0A1) | (0<<COM0A0))
#define TC0_OUTA_SET	((1<<COM0A1) | (1<<COM0A0))
#define TC0_OUTB_NRM	((0<<COM0B1) | (0<<COM0B0))
#define TC0_OUTB_TGL	((0<<COM0B1) | (1<<COM0B0))
#define TC0_OUTB_CLR	((1<<COM0B1) | (0<<COM0B0))
#define TC0_OUTB_SET	((1<<COM0B1) | (1<<COM0B0))

// TCCR0B
#define TC0_CLK_OFF		((0<<CS02) | (0<<CS01) | (0<<CS00))
#define TC0_CLK_1		((0<<CS02) | (0<<CS01) | (1<<CS00))
#define TC0_CLK_8		((0<<CS02) | (1<<CS01) | (0<<CS00))
#define TC0_CLK_64		((0<<CS02) | (1<<CS01) | (1<<CS00))
#define TC0_CLK_256		((1<<CS02) | (0<<CS01) | (0<<CS00))
#define TC0_CLK_1024	((1<<CS02) | (0<<CS01) | (1<<CS00))
#define TC0_CLK_EXTFAL	((1<<CS02) | (1<<CS01) | (0<<CS00))
#define TC0_CLK_EXTRIS	((1<<CS02) | (1<<CS01) | (1<<CS00))

// TIMSK
#define TC0_IE_COMPA	((1<<OCIE0A))
#define TC0_IE_COMPB	((1<<OCIE0B))
#define TC0_IE_OVFLW	((1<<TOIE0))


//-----------------------------------------------------------------------------
// Timer1
// GTCCR
#define TC1_PWM1B		((1<<PWM1B))
#define TC1_PSR			((1<<PSR1))

// TCCR1
#define TC1_CTC			((1<<CTC1))
#define TC1_PWM1A		((1<<PWM1A))
#define TC1_CLK_OFF		0
#define TC1_CLK_1		((0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10))
#define TC1_CLK_2		((0<<CS13) | (0<<CS12) | (1<<CS11) | (0<<CS10))
#define TC1_CLK_4		((0<<CS13) | (0<<CS12) | (1<<CS11) | (1<<CS10))
#define TC1_CLK_8		((0<<CS13) | (1<<CS12) | (0<<CS11) | (0<<CS10))
#define TC1_CLK_16		((0<<CS13) | (1<<CS12) | (0<<CS11) | (1<<CS10))
#define TC1_CLK_32		((0<<CS13) | (1<<CS12) | (1<<CS11) | (0<<CS10))
#define TC1_CLK_64		((0<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10))
#define TC1_CLK_128		((1<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10))
#define TC1_CLK_256		((1<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10))
#define TC1_CLK_512		((1<<CS13) | (0<<CS12) | (1<<CS11) | (0<<CS10))
#define TC1_CLK_1024	((1<<CS13) | (0<<CS12) | (1<<CS11) | (1<<CS10))
#define TC1_CLK_2048	((1<<CS13) | (1<<CS12) | (0<<CS11) | (0<<CS10))
#define TC1_CLK_4096	((1<<CS13) | (1<<CS12) | (0<<CS11) | (1<<CS10))
#define TC1_CLK_8192	((1<<CS13) | (1<<CS12) | (1<<CS11) | (0<<CS10))
#define TC1_CLK_16384	((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10))

// TCCR1B

// PLLCSR
#define TC1_PLL_LSM		((1<<LSM))
#define TC1_PLL_PCK		((1<<PCKE))
#define TC1_PLL_PLLE	((1<<PLLE))
#define TC1_PLL_PLOCK	((1<<PLOCK))

// TIMSK
#define TC1_IE_COMPA	((1<<OCIE1A))
#define TC1_IE_COMPB	((1<<OCIE1B))
#define TC1_IE_OVFLW	((1<<TOIE1))


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
