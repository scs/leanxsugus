/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSPROFILER_H_
#define _CLASSPROFILER_H_

#include "FlexiVisioncfg.h"

/**
* @brief A simple profiler singleton class.
*/
class CProfiler
{
protected:			
						CProfiler();
public:								
						~CProfiler();
						
	static CProfiler *	Instance();
						
	/**
	* Creates a new profile task, giving it a name for later statistics output.
	* the return value identifies a task id which can later be used to identify
	* the task at the profiler.
	*
	* If no task could be allocated, -1 is returned.
	*/
	Int 				NewProfileTask( const Char * strTaskName );
	
	/**
	* Stops and deletes a previously allocated profiler task. The tasknum must not
	* be used again.
	*/
	void				DeleteProfileTask( Int tasknum );

	void				StartProfileTask( Int tasknum );
	void				StopProfileTask( Int tasknum );
	void				ResetProfileTask( Int tasknum );
	
	Uint32				GetMaxTaskTime( Int tasknum );
	Uint32				GetAverageTaskTime( Int tasknum );
	Uint32				GetTotalTaskTime( Int tasknum );
	Uint32				GetNumTaskCalls( Int tasknum );
	
	Bool				GetTaskInfo( const Int tasknum, Char * strInfo, const Int size );
	
	/**
	* Enumerates all tasks currently used with the profiler. Start by setting tasknum to -1 and iterate
	* until the function returns FALSE.
	*/
	Bool 				EnumTasks( Int & tasknum );
	
protected:

	enum ProfilerConsts { MAX_TASKS	= 32 };
	
	/** A reference to the single instance of the profiler. */
	static CProfiler *	g_Profiler;
	
	/**
	* The structure that holds the profiling information for each added task.
	*/			
	typedef struct
	{
		/** Flag to indicate whether this task is created (via NewProfileTask()) or not. */
		Bool			bCreated;
		
		/** A reference to the task's string identifier. */
		const Char *	strTaskName;
		
		/** The task tick count at the StartProfileTask() call. */
		Uint32			unStartTicks;
		
		//* The total accumulated tick count of this task */
		Uint32			unTotalMicroseconds;
		
		/** The maximum tick count.*/
		Uint32			unMaxMicroseconds;
		
		/** The number of profiler calls for this task */
		Uint32			unTotalCalls;
		
	} ProfileTask_t;
	
	/** 
	* Allocate all objects.
	*/
	ProfileTask_t		m_Tasks[MAX_TASKS];
	
	/** The timer handler that we're going to use. */
	TIMER_Handle		m_hTimer;
};

#endif
