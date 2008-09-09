/**
* @file
* @author Bernhard Mäder
*/

#ifndef _DRVHIGHRESTIMER_H_
#define _DRVHIGHRESTIMER_H_

#include "FlexiVisioncfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct HighResTimer_DevObject
{
	/** Flag to indicate whether the driver has already been initialized. */
	Bool				bInitialized;
	
	/** 
	* A reference counter that is incremented each time timeInit() is called and decremented on
	* timeClose(). When it reaches 0, the timer is freed.
	*/
	Uint32				unRefCount;
	
	/** The DSP/BIOS timer handler that we're going to use. */
	TIMER_Handle		hTimer;
	
	/** 
	* These are the number of timer ticks per milliseconds. It equals the DSP/BIOS CLK_countspms() value,
	* but is stored permanently for performance reasons.
	*/
	Uint32				unTicksPerMs;
	
	/**
	* Same here, but for microseconds. 
	*/
	Uint32				unTicksPerUs;
	
} HighResTimer_DevObject;

extern HighResTimer_DevObject timerObject;

/**
* Initializes and starts the timer resource. This function must be called before any of the other
* high resolution timer functions are used. It may be called multiple times and maintains a reference
* counter to keep track of the number of processes that are using the timer.
*/
void			timeInit();

/**
* Decrements the reference counter and de-allocates the needed resources if no process is left to use
* the high resolution timer.
*/
void 			timeClose();

/**
* Gets the high resolutions timer's current value.
*/	
inline Uint32	timeGetHighResTime() 				{ return TIMER_getCount( timerObject.hTimer ); }

inline Uint32	timeToMs( Uint32 unHighResTime ) 	{ return (unHighResTime / timerObject.unTicksPerMs); }
inline Uint32 	timeToUs( Uint32 unHighResTime )	{ return (unHighResTime / timerObject.unTicksPerUs); }
inline Uint32	timeFromMs( Uint32 unMs )			{ return (unMs * timerObject.unTicksPerMs); }
inline Uint32	timeFromUs( Uint32 unMs )			{ return (unMs * timerObject.unTicksPerUs); }


#ifdef __cplusplus
}
#endif


#endif
