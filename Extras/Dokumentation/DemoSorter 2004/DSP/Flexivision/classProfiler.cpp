
#include "classProfiler.h"

#include "drvHighResTimer.h"
#include "libDebug.h"

#include <stdio.h>

CProfiler * CProfiler::g_Profiler = NULL;

CProfiler::CProfiler()
{
	Uint32 i;
	
	for (i=0; i<MAX_TASKS; i++)
	{
		ResetProfileTask(i);
		m_Tasks[i].bCreated = FALSE;
		m_Tasks[i].strTaskName = NULL;
	}
	
	// Initialize the timer
	timeInit();
}
  
// *************************************************************************

CProfiler::~CProfiler()
{
	timeClose();
}

// *************************************************************************

CProfiler * CProfiler::Instance()
{
	if ( g_Profiler == NULL )
	{
		g_Profiler = new CProfiler();
	}
	
	return g_Profiler;
}
  
// *************************************************************************

Int CProfiler::NewProfileTask( const Char * strTaskName )
{
	Int i=0;
	
	while ( m_Tasks[i].bCreated )
	{
		i++;
		
		if ( i == MAX_TASKS )
		{
			dbgLog("Profiler: No free task for '%s'", strTaskName);
			return -1;
		}
	}
	
	m_Tasks[i].bCreated = TRUE;
	m_Tasks[i].strTaskName = strTaskName;
	
	ResetProfileTask( i );
	
	return i;	
}

// *************************************************************************

void CProfiler::DeleteProfileTask( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return;
		
	m_Tasks[tasknum].bCreated = FALSE;
}

// *************************************************************************

void CProfiler::StartProfileTask( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return;
		
	//m_Tasks[tasknum].unStartTicks = CLK_gethtime();
	//m_Tasks[tasknum].unStartTicks = CLK_getltime();
	m_Tasks[tasknum].unStartTicks = timeGetHighResTime();
}

// *************************************************************************

void CProfiler::StopProfileTask( Int tasknum )
{
	Uint32 time;
	Uint32 ticks;
	Uint32 us;
	
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return;
	
	// Calculate delta ticks
	//time = CLK_gethtime();
	time = timeGetHighResTime();
	ticks = time - m_Tasks[tasknum].unStartTicks;
	
	// Return if the task was reset in the meantime
	if ( m_Tasks[tasknum].unStartTicks == 0 )
		return;
	
	// Convert to microseconds.
	//us = ticks * 1000 / CLK_countspms();
	//us = ticks * 100;
	us = timeToUs( ticks );
			
	// Add to accumulators.
	m_Tasks[tasknum].unTotalMicroseconds += us;
	
	if (us > m_Tasks[tasknum].unMaxMicroseconds)
		m_Tasks[tasknum].unMaxMicroseconds = us;
	
	m_Tasks[tasknum].unTotalCalls++;
	m_Tasks[tasknum].unStartTicks = 0;
}
// *************************************************************************

void CProfiler::ResetProfileTask( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return;
		
	m_Tasks[tasknum].unStartTicks = 0;	
	m_Tasks[tasknum].unTotalMicroseconds = 0;
	m_Tasks[tasknum].unMaxMicroseconds = 0;
	m_Tasks[tasknum].unTotalCalls = 0;
}

// *************************************************************************

Uint32 CProfiler::GetMaxTaskTime( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return 0;
		
	return m_Tasks[tasknum].unMaxMicroseconds;
}

// *************************************************************************

Uint32 CProfiler::GetAverageTaskTime( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return 0;

	return m_Tasks[tasknum].unTotalMicroseconds / m_Tasks[tasknum].unTotalCalls;
}

// *************************************************************************

Uint32 CProfiler::GetTotalTaskTime( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return 0;

	return m_Tasks[tasknum].unTotalMicroseconds;
}

// *************************************************************************

Uint32 CProfiler::GetNumTaskCalls( Int tasknum )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return 0;

	return m_Tasks[tasknum].unTotalCalls;
}

// *************************************************************************

Bool CProfiler::GetTaskInfo( const Int tasknum, Char * strInfo, const Int size )
{
	if ( (tasknum < 0) || (tasknum >= MAX_TASKS) )
		return FALSE;
		
	if ( m_Tasks[tasknum].bCreated != TRUE )
		return FALSE;
		
	sprintf( strInfo, "%s: avg: %d us, max: %d us, num calls: %d", 
				m_Tasks[tasknum].strTaskName,
				m_Tasks[tasknum].unTotalMicroseconds / m_Tasks[tasknum].unTotalCalls,
				m_Tasks[tasknum].unMaxMicroseconds,
				m_Tasks[tasknum].unTotalCalls );
				
	return TRUE;
				
}

// *************************************************************************

Bool CProfiler::EnumTasks( Int & tasknum )
{
	// First move to the next possible tasknum
	tasknum++;
	
	// check.
	if ( tasknum < 0 )
		tasknum = 0;

	// Then move on until we find a task or reach the end of the list.
	while( tasknum < MAX_TASKS )
	{
		if (m_Tasks[tasknum].bCreated)
			return TRUE;
			
		tasknum++;
	}
	
	return FALSE;
}


// *************************************************************************
