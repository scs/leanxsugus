
/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSOUTPUTCHANNEL_H_
#define _CLASSOUTPUTCHANNEL_H_

#include "FlexiVisioncfg.h"

class COutputDispatcher;


/**
* @brief Implements a bus couplet's output channel.
*
* This class is used to abstract a single output channel of the bus coupler.
* It is basically a queue to which a task writes commands to and from which
* a periodic SWI takes the commands at the apropriate time. 
* 
* In order to allow a quick execution of the SWI's side of the process, the 
* OutputChannel sorts the commands by their timestamps. So, determining whether 
* there is a command on the channel that must be issued at the present time, it suffices 
* to check only one command.
*
* The interface to the task are the AddComand() functions, the other functions are for
* use by the SWI.
*/
class COutputChannel
{
	friend class COutputDispatcher;
	
public:
	
	/**
	* Constructor.
	*/
	COutputChannel( );
	
	/** 
	* Destructor.
	*/
	~COutputChannel();
	
	/*
	* It has to be specified whether the channel is digital (otherwise it will be
	* analog). The bMergeOverlapping is only available on digital channels and enables
	* merging of two overlapping command. 
	*
	* example: A constellation like the following (commands 1-4)
	*
	* <pre>
	* __________1--------2__________
	* ______3-------4_______________
	* </pre>
	*
	* which would result in
	*
	* <pre>
	* ______3---1---4____2__________
	* </pre>
	*
	* Will become this by deleting commands 1 and 4:
	*
	* <pre>
	* ______3------------2__________	
	* </pre>
	*/
	void					Setup( Int nChannelNum, bool bDigital, bool bMergeOverlapping = false );
	
	/**
	* Adds a command to the incoming list. The command is added to the sorted list
	* the next time ConsumeIncoming() is called. This function is used for digital
	* channels.
	*
	* Note: this function may be called by the TSK.
	*/
	void					AddCommand( Uint32 unTime, bool bState );
	
	/**
	* Adds a command to the incoming list. The command is added to the sorted list
	* the next time ConsumeIncoming() is called. This function is used for analog
	* channels.
	*
	* Note: this function may be called by the TSK.
	*/
	void					AddCommand( Uint32 unTime, Int16 nState );
	
protected:
	/** 
	* The structure that is used to store the output commands in. 
	*/
	struct OutputCommand
	{
		/** The precise time (in highrestime units) of when the command is due. */
		Uint32 	unTime;
		
		/** The resulting state of the output after the command has been executed. */
		union
		{
			bool	bState;
			Int16	nState;
		};
		
		void operator=( const OutputCommand & rhs )	{ unTime = rhs.unTime; nState = rhs.nState; }
	};
	
	/*
	* Trasnfers all incoming events to the sorted list.
	*
	* Note: This function is called by the SWI.
	*/
	void					ConsumeIncoming( Uint32 unCurrentTime );
	
	/**
	* Tells whether there is a next command due at the present time. If the function 
	* returns false, the channel is either empty or the commands are not yet due.
	*
	* Note: This function is called by the SWI.
	*/
	bool					HasDueCommand( Uint32 unCurrentTime );
	
	/**
	* Returns the next due command and removes it from the list.
	*
	* Note: This function is called by the SWI.
	*/
	OutputCommand			PopNextCommand();
	
	/**
	* Returns the next due command's time of execution. 
	*/
	Uint32					GetNextTime();
	
	
	/**
	* Inserts an element.
	*/
	void					InsertSortCommand( OutputCommand & cmdCommand, const Uint32 unCurrentTime );
	
	/**
	* Merges overlapping commands on a digital channel. 
	*/
	void					MergeOverlapping();
	
	/**
	* Removes a command from the sorted list.
	*/
	void					RemoveCommand( int index );
	
	/**
	* Returns the index to the next free incoming element in the queue. If the queue
	* is full, -1 is returned.
	*/
	Int						GetNextFreeIncoming();
	
	/**
	* After performing the request for an incoming element with GetNextFreeIncoming(), the element
	* may be definitely added to the queue with this command. AddIncoming() must not be called
	* before the element at the new index' position is properly filled with the command's data.
	*/
	void					AddIncoming();
	
	/**
	* Retrieves the next incoming command from the incoming list. If there is
	* no element, -1 is returned.
	*/	
	Int						GetNextIncoming();
	
	/**
	* Removes the next incoming element. This must be called to consume an element
	* after it has been transfered to the sorted list.
	*/
	void					RemoveNextIncoming();
	
	
	
	
	enum OutputChannelConsts
	{
		MAX_COMMANDS = 64,
		MAX_INCOMING_COMMANDS = 16
	};
	
	/** The array that serves as a storage for the sorted commands. This array is
	* only accessed by the SWI procedure so we don't have to guard it.*/
	OutputCommand			m_arySortedCommands[MAX_COMMANDS];
	Int						m_nNumCommands;
	
	/** This array serves as an incoming queue for commands. Since both, the command
	* generating task and the command sorting and evaluating SWI will access this
	* array, it has to be implemented as a queue.
	*/
	OutputCommand			m_aryIncoming[MAX_INCOMING_COMMANDS];
	Int						m_nNextIncoming;
	Int						m_nNextFreeIncoming;
	
	Int						m_nChannelNum;
	bool					m_bDigital;
	bool					m_bMergeOverlapping;
	
};

#endif
