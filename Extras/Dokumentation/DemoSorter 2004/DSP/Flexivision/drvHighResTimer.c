	
#include "drvHighResTimer.h"

#include <clk.h>
#include <hwi.h>

#include "libDebug.h"

HighResTimer_DevObject timerObject = { FALSE, 0, NULL, 0 };

// *************************************************************************

void timeInterrupt_hwi(Uint32 funcArg)
{
	static int i=0;
	
	if ( i<10)
		dbgLog("Timer event!");
		
	i++;
}

// *************************************************************************

void timeInit()
{
/*
	Uint32 unTimerEventId;
	IRQ_Config config;
	HWI_Attrs attrs;
*/
	timerObject.unRefCount++;
	
	// Abort if already initialized
	if ( timerObject.bInitialized )
		return;
	
	// Open the timer
	timerObject.hTimer = TIMER_open( TIMER_DEVANY, TIMER_OPEN_RESET );
	
	// configure the timer
	TIMER_configArgs( 	timerObject.hTimer,
						TIMER_CTL_RMK( 	0, 							// invinp = don't care
										TIMER_CTL_CLKSRC_CPUOVR8, 	// clksrc
										TIMER_CTL_CP_CLOCK,			// clock pulse
										TIMER_CTL_HLD_NO,			// hold
										TIMER_CTL_GO_YES,			// go!
										0,							// pulse width = don't care
										0,							// data out = don't care
										0,							// invout = don't care
										TIMER_CTL_FUNC_DEFAULT ),	// function of tout pin										
										
						TIMER_PRD_RMK(	0xFFFFFFFF ),				// Period to maximum
						
						TIMER_CNT_RMK( 	0 ) );
						
	// Start the timer
	TIMER_start( timerObject.hTimer ); 
	
	// Store the ticks per second value
	timerObject.unTicksPerMs = CLK_countspms();
	timerObject.unTicksPerUs = CLK_countspms() / 1000;
	/*
	// Get the timer event id.
	unTimerEventId = TIMER_getEventId(timerObject.hTimer); 
	
	// Map the timer interrupt to our ISR and enable it
	config.funcAddr = timeInterrupt_hwi;	// Function address
	config.funcArg = 0;						// Function arguement
	config.ccMask = IRQ_CCMASK_NONE;		// Cache control
	config.ieMask = IRQ_IEMASK_SELF;		// Interrupt mask
							  	
	IRQ_config( unTimerEventId, &config );	
	IRQ_enable(unTimerEventId);

	attrs.intrMask = 1; 	// IER bitmask, 1="self" (default)
	attrs.ccMask = 1;    	// CSR CC bitmask, 1="leave alone" 
	attrs.arg = 0;      	// fxn arg (default = 0)

	HWI_dispatchPlug(
		unTimerEventId,
		timeInterrupt_hwi,
		-1,
		&attrs );
	
		*/
	timerObject.bInitialized = TRUE;
}
	
// *************************************************************************

void timeClose()
{
	// Abort if not initialized
	if ( ! timerObject.bInitialized )
		return;
	
	// Decrement reference count
	timerObject.unRefCount--;
	
	// Only free timer resource if there are no references left.
	if ( timerObject.unRefCount == 0 )
	{
		TIMER_close( timerObject.hTimer );
		timerObject.hTimer = NULL;
	
		timerObject.bInitialized = FALSE;	
	}
}

// *************************************************************************



// *************************************************************************

// *************************************************************************


