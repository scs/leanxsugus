
#include "classJetControl.h"

#include <string.h>

#include "drvHighResTimer.h"
#include "libDebug.h"
#include "classProfiler.h"


/**
* Allocate the static single instance of the classification object.
*/
CJetControl CJetControl::g_JetControl;


// DEBUG
Uint16 * ppuMemory = (Uint16*)0x60000000;
	
// flag
bool bFirstTime = true;
Int nProfileId;


// *************************************************************************

void PRD_JetControl_funct( void )
{
	if (bFirstTime)
	{
		nProfileId = CProfiler::Instance()->NewProfileTask("PRD_JetControl");
		bFirstTime = false;
	}
	else
	{
		CProfiler::Instance()->StopProfileTask( nProfileId );
	}
	
	CJetControl::Instance()->CheckCommands();
	
	CProfiler::Instance()->StartProfileTask( nProfileId );
}

// *************************************************************************

CJetControl::CJetControl()
{
	m_nNextFreeCommand = 0;
	m_nNextCommand = 0;
	
	m_bInitialized = FALSE;
}

// *************************************************************************

CJetControl * CJetControl::Instance()
{
	return &g_JetControl;
}

// *************************************************************************

void CJetControl::Init()
{
	if ( m_bInitialized == TRUE )
		return;
		
	m_hSerial = serOpen( BK_UART_CHANNEL, 32 );
	
	// Configure the channel. The BK needs even parity.
	serConfigChannel( m_hSerial, 38400, FALSE, TRUE, TRUE );
	m_BusCoupler.Config( m_hSerial, 1, 2, 32 );
	
	// Set the BK8000 ready output.
	// TODO: maybe this is not the right place to do this.
	m_BusCoupler.SetDigitalOutput( 10, TRUE );
	
	// DEBUG: HACK the write enable bit
	ppuMemory[0xF3] &= ~0x0010;
	ppuMemory[0xF3] |= 0x0020;	

	
	// Open the high res timer. Note: the timer may be opened multiple times.
	timeInit();	
	
	m_unLastCmdTime = 0;
	
	m_bInitialized = TRUE;
}


// *************************************************************************

void CJetControl::CheckCommands()
{
	static bool bWarned = false;
	static int nWait = 0;
	
	if ( ! m_bInitialized )
	{
		if ( !bWarned )
		{
			nWait++;
			if (nWait > 10000);
			{
				dbgLog("JetControl: Must be initialized!");
				bWarned = true;
			}
		}
		
		return;
	}
	
	static int nFreq = 0;
	nFreq++;
	if (nFreq < 12)
		return;
	nFreq = 0;
	
	
	// Debug::
	static Int i=0;
	i++;
	for ( int j=0; j<31; j++)
	{
		m_BusCoupler.SetDigitalOutput( j, (i>>j)&1 );
	}
		
	// Receive the current image state from the bus coupler. This is triggered by the last
	// SendImage() call and we now assume that the bus coupler's data is lying ready in the
	// SW UART's fifos.
	m_BusCoupler.ReceiveImage();
	
	// Only do something if the buffer is NOT empty
	if ( m_nNextFreeCommand != m_nNextCommand )
	{
		Uint32 curTime;
		Int32 deltaTime;
		Int32 cmdTime;
		
		// Get the command's time
		cmdTime = m_aryJetCommands[ m_nNextCommand ].unCmdTime;
		
		// Get the current time and calculate the delta time between the command's trigger time and the
		// the current time. Since these are all high res timer values (which will wrap around about every
		// minute...), they can only be used as 32 bit differences.
		curTime = timeGetHighResTime();
		deltaTime = (Int32)(curTime - cmdTime);
		
		//dbgLog("jetctrl: curtime: 0x%X, delta: 0x%X = %d", curTime, deltaTime, deltaTime );
		
		// Now see if the requested time has already passed, which is the case if
		// the delta time is positive.
		if ( deltaTime > 0 )
		{
			for ( Int i=0; i<JetCommand::MAX_JETS; i++)
			{
				m_BusCoupler.SetDigitalOutput( i, ( m_aryJetCommands[ m_nNextCommand ].aryJetState[i] != 0 ) );				
			}

			// Consume the command and increment the index.
			m_nNextCommand++;
			if (m_nNextCommand == MAX_COMMANDS)
				m_nNextCommand = 0;
		}		
	}
	
	// Send the image. This has to be done periodically so that the bus coupler won't trigger
	// its watchdog.
	m_BusCoupler.SendImage();	
	
	
/*
	if ( (count & 0x0F) == 0 )
	{
		// Set the BK8000 ready signal to the SPS
		// TODO: do this in the right place.
		m_BusCoupler.SetDigitalOutput( 10, TRUE );
		for ( int i=0; i<10; i++)
		{
			m_BusCoupler.SetDigitalOutput( i, ((count & 0x00F0) >> 4) == i );
		}
		
		
	}	
	*/

}

// *************************************************************************

void CJetControl::AddCommand( JetCommand * cmd )
{
	Int nNext_ahead;
	
	// Generate an index one ahead of next free.
	nNext_ahead = m_nNextFreeCommand + 1;
	if (nNext_ahead == MAX_COMMANDS)
		nNext_ahead = 0;
			
	// Check if the buffer is full
	if ( nNext_ahead != m_nNextCommand )
	{
		// Check, if the command's execution time is not earlier than
		// of any of the ones already in the list. Don't do this check
		// if the list is empty.
		if ( 	(m_nNextFreeCommand != m_nNextCommand)
			&& 	( (Int32)(cmd->unCmdTime - m_unLastCmdTime) < 0) )
			return;
			
		// Thus, this command is the last to be executed.
		m_unLastCmdTime = cmd->unCmdTime;
		
		// Copy the command into the list
		memcpy( &(m_aryJetCommands[ m_nNextFreeCommand ]), cmd, sizeof( JetCommand ) );
		
		// Increment the index 
		m_nNextFreeCommand  = nNext_ahead;				
	}
	else
	{
		dbgLog("JetControl: Command queue full!");
	}
}

// *************************************************************************
