
#include "classOutputDispatcher.h"

#include "drvHighResTimer.h"
#include "libDebug.h"

// Instantiate the static unique instance of this class.
COutputDispatcher COutputDispatcher::g_OutputDispatcher;


// *************************************************************************

void PRD_OutputDispatcher_funct( void )
{
	// Just call the entry point in the singletin object.
	COutputDispatcher::Instance()->DispatchCommands();
}

// *************************************************************************

COutputDispatcher::COutputDispatcher()
{
	m_bInitialized = false;
}

// *************************************************************************

COutputDispatcher * COutputDispatcher::Instance()
{
	return &g_OutputDispatcher;
}

// *************************************************************************
	
void COutputDispatcher::Init()
{
	if ( m_bInitialized == TRUE )
		return;
		
	// Open the serial line to the BK8000. We'll use a small buffer of 32 bytes.
	m_hSerial = serOpen( BK_UART_CHANNEL, 32 );
	
	// Configure the channel. The BK8000 needs even parity.
	serConfigChannel( m_hSerial, /*38400*/115200, FALSE, TRUE, TRUE );
	m_BusCoupler.Config( m_hSerial, 1, 2, 16 );
	
	// Open the high res timer. Note: the timer may be opened multiple times.
	timeInit();	
	
	// Go through each channel and set them up to be digital with overlapping resolution.
	for (Int i=0; i<MAX_OUTPUTCHANNELS; i++)
		m_aryChannels[i].Setup( i, 		// Channel number
								true, 	// The channel is digital
								/*true*/false );	// Merge overlapping
		
	m_unLastSend = timeGetHighResTime();
	
	// Set the flag
	m_bInitialized = TRUE;
}

// *************************************************************************

void COutputDispatcher::DispatchCommands()
{
	bool							bNewCommands = false;
	COutputChannel::OutputCommand 	cmd;
	Uint32 							nextTime;
	
	
	// First check that we're initialized	
	if ( ! m_bInitialized )
		return;
	
	// Get the current time
	Uint32 unRealTime = timeGetHighResTime();	
	Uint32 unDecisionTime = unRealTime;
	
	// See if we can send already. Since there is a minimum time required between two sends,
	// we must take care not to be too fast.
	if ( (unRealTime-m_unLastSend) < timeFromMs( MIN_TIME_BETWEENSEND_MS ) )
		return;

	// Go through each channel and see if it has a new commands to issue
	for (Int i=0; i<MAX_OUTPUTCHANNELS; i++)
	{
		while ( m_aryChannels[i].HasDueCommand( unDecisionTime ) )
		{
			//dbgLog("OutputDispatcher: Found command on channel %d", i);
			
			// Get the command and set the digital output accordingly.
			cmd = m_aryChannels[i].PopNextCommand();
			m_BusCoupler.SetDigitalOutput( i, cmd.bState );
				
			// Set the flag that indicates that something has changed.
			// If the flag has not yet been set, advance the time by half the buscoupler's
			// update interval, so that commands are not delayed too long but rather executed
			// a bit earlier (which reduces the total error considerably).
			if ( bNewCommands == false )
			{
				unDecisionTime += timeFromMs( MIN_TIME_BETWEENSEND_MS );
				bNewCommands = true;
			}
						
		} // if has next
		
	} // for all channels	
	
	// Now, if there were changes in the process image, send them to the BK
	if (bNewCommands)
	{
		m_BusCoupler.SendImage();
		m_unLastSend = unRealTime;		
	}	
	else
	{
		// If there were no changes, see if the last send of the image was too far back,
		// so that the buscoupler's watchdog is about to trigger.
		if ( (unRealTime-m_unLastSend) > timeFromMs( MAX_TIME_BETWEENSEND_MS ) )
		{
			m_BusCoupler.SendImage();
			m_unLastSend = unRealTime;			
		}
	}
	
	// Now that we're done with the time-critical business, re-sort the
	// output channels
	for (Int i=0; i<MAX_OUTPUTCHANNELS; i++)
	{
		m_aryChannels[i].ConsumeIncoming( unRealTime );
	}
		
	
}	

// *************************************************************************

COutputChannel & COutputDispatcher::Channel( Int nChannel )
{
	if ( (nChannel >= MAX_OUTPUTCHANNELS) || ( nChannel < 0 ) )
	{
		dbgLog("OutputDispatcher: Illegal channel: %d", nChannel );
		return m_aryChannels[0];
	}
	
	return m_aryChannels[nChannel];
}

// *************************************************************************

// *************************************************************************

// *************************************************************************


