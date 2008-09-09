/**
* @file
* @author Bernhard Mäder
*/

#ifndef _DRVCONVEYOR_H_
#define _DRVCONVEYOR_H_


#include "FlexiVisioncfg.h"

#include <sem.h>

#define DRVCONV_FRACTIONAL_BITS	16

#ifdef __cplusplus
extern "C"
{
#endif


/**
* Initializes the conveyor device driver.
*/
void		convInit( );

/**
* Gets the measured speed of the conveyor in meters per second, in Q.16 fixed point format.
*/
Uint32		convGetMeasuredSpeed();

/**
* Gets the last measured interval between two hardware triggers in system ticks.
*/
Uint32		convGetMeasuredInterval();

/**
* Gets the current position of the conveyor belt. The measurement starts with position 0
* and is incremented each time a trigger signal is received.
*/
Uint32		convGetPosition();

/**
* Returns the time at which the conveyor last triggered.
*/
Uint32		convGetLastTriggerTime();

/**
* Registers a semaphore that is triggered each time the conveyor is triggered.
* DEBUG: do it with a bool flag.
*/
void		convRegisterSemTrigger( SEM_Handle sem, Bool * bTrigger );
	

#ifdef __cplusplus
}
#endif


#endif
