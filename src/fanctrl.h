#ifndef __FANCTRL_H__
#define __FANCTRL_H__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>

#include "board.h"


#define INITIAL_PWM		179
#define TEMP_RANGE		2
#define DELAY			1000



//--------------------------------------------------
// Fan control state machine
#define STATE_INIT			0
#define STATE_HOT			10
#define STATE_HOT_NXT		11
#define STATE_COOL			20
#define STATE_COOL_NXT		21
#define STATE_NEXT			254
#define STATE_DECAY			255




#endif
