/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSWATCHDOG_H_
#define _CLASSWATCHDOG_H_

#include "FlexiVisioncfg.h"

#include "libDebug.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* This is the entry point for the DSP/BIOS' PRD module. It must be
* specified in C-Linkage.
*/
void 		PRD_Watchdog_funct( void );

#ifdef __cplusplus
}
#endif

/**
* @brief A software watchdog object.
*
* The watchdog class is a singleton object which monitors variables of
* other tasks. It increments those variables periodically and, if any
* of these variables reaches the timeout value, resets the whole system,
* after trying to set off a message to the host, saying which task led
* to the reset. Obviously, if that object was the main control task,
* that message will never be sent.
*
* Those variables are allocated by the watchdog and their pointers are
* given to the tasks when they add themselves to the watched list.
*/
class CWatchdog
{
	friend void PRD_Watchdog_funct();
	
	enum WatchdogConsts {
		/** The maximum number of objects the watchdog can watch. */
		WD_MAXOBJ = 8,
		
		/** The number of times the watchdog is called per second. This value is set
		 *  by the DSP/BIOS config utility. */
		WD_INTERVAL = 10
	};
	
public:
	/**
	* Constructor
	*/
							CWatchdog();
						
	/**
	* Returns a reference to the watchdog objects.
	*/
	static CWatchdog *		Instance();
	
	/**
	* Adds a task to the watch list. The returned pointer is the pointer to
	* the watch counter which, when it reaches the timeout level, triggers
	* a system reset. Thus, the watched object must periodically reset the
	* variable this reference points to by calling SignalAlive().
	*
	* The timeout value is specified in seconds.
	*/	
	Uint32					AddToWatch( const Char * strTaskName, Uint32 unTimeout );
	
	/**
	* Enables and disables a watch of a process.
	*/
	void					EnableWatch( Uint32 unWatchId, Bool bEnable );
	
	/**
	* Signals that the specified process is still alive.
	*/
	void					SignalAlive( Uint32 unWatchId );
	
protected:

	/**
	* Goes through the list of objects to watch and increments all counters.
	* If one counter reaches the timeout, the system is reset.
	*/
	void					WatchObjects();
	
	/**
	* A structure to store the info of the watched objects.
	*/
	typedef struct _WatchInfo
	{
		Uint32 			unCounter;
		Uint32			unTimeout;
		
		Bool			bUsed;
		Bool			bEnabled;
		Bool			bDead;
		
		const Char *	strWatchedObject;
	} WatchInfo;
	
	/**
	* The list to store the watched objects in.
	*/
	WatchInfo				m_aryWatchedObjects[WD_MAXOBJ];
	
	static CWatchdog		g_Watchdog;
	
};



#endif
