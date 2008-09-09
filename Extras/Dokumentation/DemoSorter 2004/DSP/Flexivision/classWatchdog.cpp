
#include "classWatchdog.h"

#include <clk.h>
#include <csl_irq.h>

#include <stdio.h>

#include "libLed.h"


// Allocate the singleton object
CWatchdog CWatchdog::g_Watchdog;

// *************************************************************************
	
CWatchdog::CWatchdog()
{
	Int i;
	
	for (i=0; i<WD_MAXOBJ; i++)
	{
		m_aryWatchedObjects[i].bUsed = FALSE;
	}
}

// *************************************************************************
	
CWatchdog * CWatchdog::Instance( void )
{
	return &g_Watchdog;
}

// *************************************************************************

Uint32 CWatchdog::AddToWatch( const Char * strTaskName, Uint32 unTimeout )
{
	Int i=0;
	
	while( (i<WD_MAXOBJ) && (m_aryWatchedObjects[i].bUsed) )
		i++;
		
	if ( i== WD_MAXOBJ )
		return 0xFFFFFFFF;
		
	// CLK_getprd = microseconds per interrupt = 100
	/*
	Int prd = CLK_getprd();
	Int numIntsPerSecond = 1000*1000 / CLK_getprd();
	Int numCallsPerSecond = numIntsPerSecond / PRD_Watchdog.period;
*/

	m_aryWatchedObjects[i].bUsed = TRUE;
	m_aryWatchedObjects[i].bEnabled = FALSE;
	m_aryWatchedObjects[i].bDead = FALSE;
	m_aryWatchedObjects[i].strWatchedObject = strTaskName;
	m_aryWatchedObjects[i].unCounter = 0;
	m_aryWatchedObjects[i].unTimeout = unTimeout * WD_INTERVAL;	
	
	return i;	
}

// *************************************************************************

void CWatchdog::EnableWatch( Uint32 unWatchId, Bool bEnable )
{
	if ( unWatchId >= WD_MAXOBJ )
		return;	
		
	// Reset counter and set the enable bit.
	m_aryWatchedObjects[ unWatchId ].unCounter = 0;
	m_aryWatchedObjects[ unWatchId ].bEnabled = bEnable;
	
}

// *************************************************************************

void CWatchdog::SignalAlive( Uint32 unWatchId )
{
	if ( unWatchId >= WD_MAXOBJ )
		return;
		
	// Don't have to lock, since this function is called from TSKs only, which
	// can't preempt the SWI, which WatchObjects() runs in.
	m_aryWatchedObjects[ unWatchId ].unCounter = 0;
}

// *************************************************************************

void CWatchdog::WatchObjects()
{
	Int i;
	
	for ( i=0; i<WD_MAXOBJ; i++)
	{
		if ( m_aryWatchedObjects[i].bUsed && m_aryWatchedObjects[i].bEnabled )
		{
			m_aryWatchedObjects[i].unCounter++;
			
			if ( m_aryWatchedObjects[i].unCounter > m_aryWatchedObjects[i].unTimeout )
			{
				if ( ! m_aryWatchedObjects[i].bDead )
				{
					dbgLog("Watchdog: object %s is dead.", m_aryWatchedObjects[i].strWatchedObject );
					
					// Don't yet reset, let the debug log some time to pass through. Reset next time.
					m_aryWatchedObjects[i].bDead = TRUE;
				}
				else
				{
					printf("Watchdog reset because of %s!\n", m_aryWatchedObjects[i].strWatchedObject );
					
					IRQ_globalDisable();
					
					// Reset here by jumping to 0x00. Statement (c) Limacher
					(( void (*)()) 0 )();
				} 				
				
			} // timeout
			
		} // used and enabled
		
	} // for all watches.
	
}


// *************************************************************************

void PRD_Watchdog_funct( void )
{
	CWatchdog::Instance()->WatchObjects();
}
