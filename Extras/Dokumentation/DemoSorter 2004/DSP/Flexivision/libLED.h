/**
* @file
* @brief The LED access library.
* @author Bernhard Mäder
*
* Handles access to the four LEDs available on the flexivision board.
*/

#ifndef _LIBLED_H_
#define _LIBLED_H_

#include "FlexiVisioncfg.h"

#include <csl_gpio.h>

#include <math.h>
#include <assert.h>


#define LED_PULSE_RESOLUTION	64		///< The PWM signal resolution
#define LED_TICKSPERSECOND		1000	///< The numbers of ticks per second.
#define LED_KITT_SPEED			0.07	///< The speed of the walking light.
#define LED_KITT_RANGE			1.0		///< The range of the walking light. Larger values will make the effect more broad.

#define LED_MAX(x,y) ((x>y)?x:y)
#define LED_DIST(x,y) (fabs(x-y))



#ifdef __cplusplus
extern "C"
{
#endif


/**
* Initializes the I/O ports to the LEDs.
*/
void ledInit();

/**
* Turns on or off a certain LED.
*
* @param	lednum		The led to turn on or off.
* @param	on			TRUE, if the LED should be lit, FALSE if it should go dark.
*/
void ledLight( int lednum, Bool on);

/**
* Does a stylish Knightrider-style LED-blinker. The function will not return and loop forever.
*/
void ledBlink();

#ifdef __cplusplus
}
#endif

#endif
