/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSWRAPPER_H_
#define _CLASSWRAPPER_H_

#ifdef __cplusplus
extern "C"
{
#endif

void StartVisionTask();
void StartControlTask();
void SendDebugMsg( const char * str );

#ifdef __cplusplus
}
#endif

#endif


