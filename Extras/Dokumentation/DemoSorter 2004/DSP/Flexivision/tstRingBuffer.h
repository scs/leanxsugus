/**
* @file
* Test file for libRingBuffer.h
* @see libRingBuffer.h
*/

#ifndef _TSTRINGBUFFER_H_
#define _TSTRINGBUFFER_H_

#include <std.h>
#include <log.h>
#include <assert.h>
#include <string.h>


#include "FlexiVisioncfg.h"

#include "libRingBuffer.h"



#ifdef __cplusplus
extern "C"
{
#endif

Bool	RingBufferTest();
Bool 	RingBufferTestChar();


#ifdef __cplusplus
}
#endif

#endif
