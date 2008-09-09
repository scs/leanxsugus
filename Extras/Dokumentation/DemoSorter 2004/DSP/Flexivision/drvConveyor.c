
#include "drvConveyor.h"

#include <csl_irq.h>
#include <clk.h>

#include "drvConveyor_HAL.h"

#include "drvHighResTimer.h"

#include "classStatus.h"
#include "libDebug.h"
#include "libLED.h"


/**
* This is the HWI entry point for the interrupt that is triggered by the conveyor trigger
* signal. This HWI will measure the current time so that the conveyor's speed can be determined. It will
* also trigger the CClassification tasks SignalTrigger function.
*/
void 		HWI_ConveyorTrigger_funct( void );

/**
* The device information structure that is used to store all information that is
* acquired by the HWI. This information can then be retrieved by the driver's 
* function interface by the application.
*/
typedef struct Conveyor_DevObject
{
	SEM_Handle			semTrigger;
	Bool *				bTrigger;

	Uint32				unMillimetersPerTrigger ;
	
	Uint32				unLastTicks;	
	Uint32				unLastDeltaTicks;	
	
	Uint32				unNumTotalTriggerSignals;
	
	Uint32				unTimeoutTicks;
	
	Bool				bStanding;
} Conveyor_DevObject;

Conveyor_DevObject convObject = { NULL, 0, 0, 0, 0, 0, TRUE };

// *************************************************************************

void		convInit( )
{
	convObject.unMillimetersPerTrigger = CONV_MILLIMETERS_PER_TRIGGER;
	convObject.unLastTicks = 0;
	convObject.unLastDeltaTicks = 0;
	convObject.unNumTotalTriggerSignals = 0;
	convObject.unTimeoutTicks = timeFromMs( convHal.unTimeout );
	convObject.bStanding = TRUE;
	
	// PRE: the HWI link to our handler function is set by the config tool.
	IRQ_clear( convHal.unExtInterrupt );
	IRQ_enable( convHal.unExtInterrupt );
	
	// Enable the interrupt in the FPGA
	ppuMemory[CONV_REG_INTENABLE_ADDR] |= CONV_REG_INTENABLE_MASK;
	
	// Init the high res timer that we're going to use
	timeInit();
}

// *************************************************************************

Uint32		convGetMeasuredSpeed()
{
	Uint32 	unDeltaMs;
	Uint32	unTempDeltaTicks;
	Uint32	unTempDeltaMs;
	Uint32	fpTriggersPerSecond;
	Uint32	fpLastSpeed;
	
		
	// Calculate the last interval in ms.
	unDeltaMs = timeToMs( convObject.unLastDeltaTicks );
	if ( unDeltaMs == 0 )
		return 0;	
	
	// And convert to speed (note: speed is Q.16 format and m/s).
	fpTriggersPerSecond = (1 << DRVCONV_FRACTIONAL_BITS) * 1000  / unDeltaMs;
	fpLastSpeed = fpTriggersPerSecond * convObject.unMillimetersPerTrigger / 1000;	
	
	// If standing, the speed is 0.
	if ( convObject.bStanding )
		return 0;

	// Determine delta time since last trigger.
	unTempDeltaTicks = timeGetHighResTime() - convObject.unLastTicks;
	
	// If delta is too big, the conveyor is declared to be standing and thus its speed
	// is 0 as well.
	if ( timeToMs(unTempDeltaTicks) > convHal.unTimeout )
	{
		//dbgLog("Set to standing because toMs(%ud)=%d > %d", unTempDeltaTicks, timeToMs(unTempDeltaTicks), convHal.unTimeout );
		//dbgLog(" unLastTicks = %ud, curTime= %ud", convObject.unLastTicks, timeGetHighResTime() );
		convObject.bStanding = TRUE;
		return 1;
	}
	
	/*
	// If speed*tempDelta is greater than the distance between two triggers,
	// reduce speed so that speed = distance / curDeltaTime.
	unTempDeltaMs = timeToMs(unTempDeltaTicks);
	
	if ( ( (unTempDeltaMs * fpLastSpeed)>> DRVCONV_FRACTIONAL_BITS ) > CONV_MILLIMETERS_PER_TRIGGER)
	{		
		dbgLog("Time: %ud, deltaMs: %u", timeGetHighResTime(), unTempDeltaMs );
		dbgLog("Reduced speed because %d > %d. From %f to %f",
			(unTempDeltaMs * fpLastSpeed) >> DRVCONV_FRACTIONAL_BITS, 
			CONV_MILLIMETERS_PER_TRIGGER,
			(float)fpLastSpeed / 65536.0,
			(float)(((1<< DRVCONV_FRACTIONAL_BITS ) * CONV_MILLIMETERS_PER_TRIGGER) / unTempDeltaMs) / 65536.0 );
			
		if ( unTempDeltaMs == 0 )
			return 1;
			
		fpLastSpeed = ((1<< DRVCONV_FRACTIONAL_BITS ) * CONV_MILLIMETERS_PER_TRIGGER) / unTempDeltaMs;
	}
	*/	
	
	return fpLastSpeed;
}

// *************************************************************************

Uint32		convGetMeasuredInterval()
{
	return convObject.unLastDeltaTicks;
}

// *************************************************************************

Uint32		convGetPosition()
{
	Uint32 pos;
	Uint32 deltaTimeMs;	
	Uint32 speed;
	Uint32 offset;
	
	pos = convObject.unNumTotalTriggerSignals * CONV_MILLIMETERS_PER_TRIGGER;
	
	// if standing, the conveyor's position is right before the next
	// trigger point.
	if ( convObject.bStanding )
	{
		pos += CONV_MILLIMETERS_PER_TRIGGER;
		return pos;
	}
	
	
	// determine current delta time since last trigger signal
	// conveyor position is at speed * deltatime + last trigger's
	// position.
	deltaTimeMs = timeToMs( timeGetHighResTime() - convObject.unLastTicks );
	speed = convGetMeasuredSpeed();
	offset = ( speed * deltaTimeMs) >> DRVCONV_FRACTIONAL_BITS;
	
	//dbgLog("pos %d: deltaMs: %d, speed: %d, pos+:%d", pos, deltaTimeMs, speed, offset );
	
	pos += offset;
	
	return pos;	
}

// *************************************************************************

Uint32		convGetLastTriggerTime()
{
	return convObject.unLastTicks;
}

// *************************************************************************

void		convRegisterSemTrigger( SEM_Handle sem, Bool * bTrigger )
{
	convObject.semTrigger = sem;
	convObject.bTrigger = bTrigger;
}

// *************************************************************************

void 		HWI_ConveyorTrigger_funct( void )
{
	Uint32 unCurrentTicks;
	
	// Note: We do all needed stuff directly in th HWI because:
	//		 - It's not that much (SEM_post is the most expensive operation,
	//		   needing about 182 cycles)
	//		 - Starting an SWI would result in more overhead ( SWI_post is
	//		   almost as expensive as SEM_post, using 118 cycles without
	//		   switching and even more when the switch is performed).
	//		 - The rest is neglectable.
	
	// Increment the stats
	gStats.unConvNumTriggers++;	

	// First, measure the interval...
	unCurrentTicks = timeGetHighResTime();
	convObject.unNumTotalTriggerSignals++;
			
	// If the conveyor was standing before, this is a special case.
	if ( convObject.bStanding == TRUE )
	{
		// We set the delta ticks to a very high value, so that the speed will also
		// be nearly 0.
		convObject.unLastDeltaTicks = 0x3FFFFFFF;
		
		// But, the conveyor is not standing anymore.
		convObject.bStanding = FALSE;
	}
	else
	{
		convObject.unLastDeltaTicks = unCurrentTicks - convObject.unLastTicks;
	}	
	
	convObject.unLastTicks = unCurrentTicks;
	
	// Finally trigger the registered semaphore, if there is one
	*(convObject.bTrigger) = TRUE;
	if ( convObject.semTrigger != NULL )
		SEM_ipost( convObject.semTrigger );
}

// *************************************************************************
