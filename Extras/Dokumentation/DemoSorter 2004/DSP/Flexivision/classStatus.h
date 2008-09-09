/**
* @file
* @author Bernhard Mäder
*/
#ifndef _CLASSSTATUS_H_
#define _CLASSSTATUS_H_

#include "FlexiVisioncfg.h"
/*
#ifdef __cplusplus
extern "C"
{
#endif
*/

/**
* The structure that stores the stats values.
*/
typedef struct _sStats
{
	// PPU stats
	Uint32				unPPULastErrorWordCountShould;
	Uint32				unPPULastErrorWordCountIs;
	
	Uint32				unPPUNumPicsGood;
	Uint32				unPPUNumPicsBad;
	Uint32				unPPUNumPicsDropped;
	
	// Conveyor stats
	Uint32				unConvNumTriggers;
	
	// Classifier stats
	Uint32				unClassNumExchanges;
	Uint32				unClassNumExchangeFaults;
	Uint32				unClassNumExchangeFaultsLength;
	Uint32				unClassNumExchangeFaultsMagic1;
	Uint32				unClassNumExchangeFaultsMagic2;	
	Uint32				unClassNumPotTotal;
	Uint32				unClassNumPotMerges;
	Uint32				unClassNumPotAdds;
} sStats;

/** The global stats structure that is accessible from all code. */
extern volatile sStats	gStats;
/*
#ifdef __cplusplus
}
#endif
*/
// only declare this class if compiling for c++...
#ifdef __cplusplus

#include "classControl.h"

/**
* @brief A class to store and display system global status information.
* The status singleton class that does all the display work for the stats.
* The class can obviously only be accessed by c++ code, whereas c code may
* manipulate the status value directly via the gStats struct.
*/
class CStatus
{
protected:
	/** The constructor of this singleton object is made protected. */
							CStatus();

public:
	/** Returns the single instance. */
	static inline CStatus *	Instance()					{ return & m_Instance; }
	
	/** Puts a textual version of the status object on the answer queue. */
	void					Log( CControl * ctrl );
		
protected:
	static CStatus 			m_Instance;	

};

#endif

#endif
