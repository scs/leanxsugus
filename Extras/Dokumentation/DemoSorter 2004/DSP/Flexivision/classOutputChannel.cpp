
#include "classOutputChannel.h"

#include "drvHighResTimer.h"
#include "libDebug.h"

COutputChannel::COutputChannel( )
{
	m_nNumCommands = 0;
	
	m_nNextIncoming = 0;
	m_nNextFreeIncoming = 0;
	
	m_bDigital = true;
	m_bMergeOverlapping = false;
}

// *************************************************************************

COutputChannel::~COutputChannel()
{
}

// *************************************************************************

void COutputChannel::Setup( Int nChannelNum, bool bDigital, bool bMergeOverlapping )
{	
	if ( bMergeOverlapping && (!bDigital) )
		dbgLog( "OutputChannel %d: cannot specify bMergeOverlapping on an analog channel.", nChannelNum );

	m_nChannelNum = nChannelNum;
	m_bDigital = bDigital;		
	m_bMergeOverlapping = bMergeOverlapping;
}

// *************************************************************************
	
void COutputChannel::AddCommand( Uint32 unTime, bool bState )
{
	SWI_disable();
	
	Int next = GetNextFreeIncoming();
	
	if ( next != -1 )
	{
		int n, nf;
		n = m_nNextIncoming;
		nf = m_nNextFreeIncoming;
		
		m_aryIncoming[next].unTime = unTime;
		m_aryIncoming[next].bState = bState;
		
		AddIncoming();		
		
		//dbgLog("Added: n:%d, nf:%d -> n:%d, nf:%d", n, nf, m_nNextIncoming, m_nNextFreeIncoming );
	}

	SWI_enable();
}

// *************************************************************************

void COutputChannel::AddCommand( Uint32 unTime, Int16 nState )
{
	SWI_disable();
	
	Int next = GetNextFreeIncoming();
	
	if ( next != -1 )
	{
		m_aryIncoming[next].unTime = unTime;
		m_aryIncoming[next].nState = nState;
		
		AddIncoming();
	}
	
	SWI_enable();
}

// *************************************************************************

void COutputChannel::ConsumeIncoming( Uint32 unCurrentTime )
{
	int next;
	bool bChangesMade = false;
	
	// Consume all incoming commands
	next = GetNextIncoming();
	//int debug=0;
	while( next != -1 )
	{
//		debug++;

		bChangesMade = true;
		
		// Abort if the list is already full. We do not consume the incoming event, in the
		// hope that it's consumed later.
		if ( m_nNumCommands == MAX_COMMANDS-1 )
		{
			//dbgLog( "OutputChannel %d: sorted list full! Could not consume incoming", m_nChannelNum );
			return;	
		}
		
		// Now insertion-sort the element and consume it.
		InsertSortCommand( m_aryIncoming[next], unCurrentTime );
		RemoveNextIncoming();
		
		// Advance to the next incoming		
		next = GetNextIncoming();
	}	
		
	// If the flag is set, merge the overlapping

	if ( m_bMergeOverlapping && bChangesMade )
		MergeOverlapping();	
	
	/*
	if ( debug > 0 )
		dbgLog("Transfered %d items: listsize: %d, list: %d < %d < %d < %d < %d < %d\r\nInputlist: %d, %d, %d, %d, %d", debug, 
					m_nNumCommands, 
					m_arySortedCommands[0].unTime,
					m_arySortedCommands[1].unTime,
					m_arySortedCommands[2].unTime,
					m_arySortedCommands[3].unTime,
					m_arySortedCommands[4].unTime,
					m_arySortedCommands[5].unTime,
					m_aryIncoming[0].unTime,
					m_aryIncoming[1].unTime,
					m_aryIncoming[2].unTime,
					m_aryIncoming[3].unTime,
					m_aryIncoming[4].unTime
					 );
					 */
}

// *************************************************************************

bool COutputChannel::HasDueCommand( Uint32 unCurrentTime )
{
	if ( m_nNumCommands == 0 )
		return false;
		
	if ( (Int32)(m_arySortedCommands[0].unTime - unCurrentTime) < 0 )
		return true;
		
	return false;	
}

// *************************************************************************

COutputChannel::OutputCommand COutputChannel::PopNextCommand()
{
	// First, read the element at position 0
	OutputCommand cmd = m_arySortedCommands[ 0 ];
	
	// Then remove it by moving all elements down by one index.
	m_nNumCommands--;
	for ( int i=0; i<m_nNumCommands; i++ )
		m_arySortedCommands[i] = m_arySortedCommands[i+1];
		
	return cmd;	
}	

// *************************************************************************

Uint32 COutputChannel::GetNextTime()
{
	return m_arySortedCommands[0].unTime;
}

// *************************************************************************

void COutputChannel::InsertSortCommand( OutputCommand & cmdCommand, const Uint32 unCurrentTime )
{
	// First see, if the command is in the past. If that's the case, don't add it.
	if ( (int)(cmdCommand.unTime - unCurrentTime) < 0 )
		return;
		//cmdCommand.unTime = unCurrentTime+1;
		
	// We have to search the index of the new command.
	// Start from the back of the list on the last free index.
	Int index = m_nNumCommands;
	
	// Now compare the new command's time with the elements in the list (always looking
	// down one index position) and decrement the position as long as the time is smaller.
	while ( index != 0 )
	{
		if ( 		( m_arySortedCommands[index-1].unTime - unCurrentTime ) 
				<= 	( cmdCommand.unTime - unCurrentTime ) )
			break;
		
		index--;
	}
	
	// index now points to the position where the new command should be stored.	
	// Now shift all elements up to make place for our new element
	for ( int i=m_nNumCommands; i>index; i--)
	{
		m_arySortedCommands[i+1] = m_arySortedCommands[i];
	}
	
	// And store the command
	m_arySortedCommands[index] = cmdCommand;	
	m_nNumCommands++;
}

// *************************************************************************

void COutputChannel::MergeOverlapping()
{
	Int numOnCmds = 0;
	
	// We're starting at the next due command and make our progress to the end of the list.
	// The number of commands that will issue an "on" command to the output are counted and may
	// not excess 1. If it does, we got an overlap, in which case the second of the "on" commands
	// will be deleted. Likewise, after such an encounter, the next "off" command has to be
	// deleted.
	for ( int i=0; i<m_nNumCommands; i++)
	{
		// if this is a turn-on command...
		if ( m_arySortedCommands[i].bState )
		{
			// see if we don't already have one.
			if ( numOnCmds > 0 )
			{
				// remove it, which also includes decrementing the counter variable, since
				// the next element's index will also be reduced by one.
				RemoveCommand( i );
				i--;
			}
			
			// Increment the number of encountered on commands in any case
			numOnCmds++;
			
		} // if "on" command
		else
		{ 
			// This is an "off" command. We have to decrement the numOnCmds and if
			// it doesn't reach 0, we have to remove this element and, again, decrement
			// i by one.
			numOnCmds--;
			if ( numOnCmds > 0 )
			{
				RemoveCommand( i );
				i--;
			}
				
			// Cap the numOnCmds at 0, since it may well be possible to have more off
			// commands than on commands, when, e.g., the on command has already been
			// issued, but not so the off command.
			if ( numOnCmds<0 )
				numOnCmds = 0;
		}
				
	} // for all elements
				
}

// *************************************************************************

void COutputChannel::RemoveCommand( int index )
{
	if ( index >= m_nNumCommands )
		return;
		
	// Remove it by moving all elements above it down by one index.
	m_nNumCommands--;
	for ( int i=index; i<m_nNumCommands; i++ )
		m_arySortedCommands[i] = m_arySortedCommands[i+1];
}

// *************************************************************************

Int COutputChannel::GetNextFreeIncoming()
{
	// Advance the next free index pointer
	Int m_nNextFreeIncoming_ahead = ( m_nNextFreeIncoming == MAX_INCOMING_COMMANDS-1 ) ? 0 : m_nNextFreeIncoming+1;
	
	// See if the queue is full.
	if ( m_nNextFreeIncoming_ahead == m_nNextIncoming )
		return -1;
	else
		return m_nNextFreeIncoming;
}

// *************************************************************************
	
void COutputChannel::AddIncoming()
{
	// Try to advance the next free index pointer
	Int m_nNextFreeIncoming_ahead = ( m_nNextFreeIncoming == MAX_INCOMING_COMMANDS-1 ) ? 0 : m_nNextFreeIncoming+1;
	
	// Only store if succesful.
	if ( m_nNextFreeIncoming_ahead != m_nNextIncoming )
		m_nNextFreeIncoming = m_nNextFreeIncoming_ahead;	
}

// *************************************************************************
	
Int COutputChannel::GetNextIncoming()
{
	if ( m_nNextIncoming == m_nNextFreeIncoming)
		return -1;
		
	return m_nNextIncoming;
}
	
// *************************************************************************

void COutputChannel::RemoveNextIncoming()
{
	// increment the pointer to the next incoming element and wrap around if
	// the maximum capacity is reached.
	m_nNextIncoming = ( m_nNextIncoming == MAX_INCOMING_COMMANDS-1 ) ? 0 : m_nNextIncoming+1;	
}
	
// *************************************************************************
