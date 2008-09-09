
#define assertLog(b) dbgAssertLog( (b), __FILE__, __LINE__ )

#ifndef _LIBDEBUG_H_
#define _LIBDEBUG_H_

#include "libLed.h"
#include "FlexiVisioncfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

void dbgAssertLog(Bool b, char * file, Int line);
void dbgLog( const char * strFormat, ... );

#ifdef __cplusplus
}
#endif

#endif

