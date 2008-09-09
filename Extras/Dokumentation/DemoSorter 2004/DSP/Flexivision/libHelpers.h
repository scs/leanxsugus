
#ifndef _LIBHELPERS_H_
#define _LIBHELPERS_H_

#include "FlexiVisioncfg.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
* Converts a given time from milliseconds to system ticks.
*/
Int 			hlpMsToTicks( Int ms );

/**
* Converts system ticks (of the low resolution timer) back to milliseconds.
*/
Int				hlpTicksToMs( Int ticks );


#ifdef __cplusplus
}
#endif

#endif
