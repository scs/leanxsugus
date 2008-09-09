
#ifndef _TSTEDMAMANAGER_H_
#define _TSTEDMAMANAGER_H_

#include "libEDMAManager.h"

#ifdef __cplusplus
extern "C"
{
#endif

void	tstedmaPrepareBuffers( Ptr src, Ptr dst, Uint32 unSize );
Bool	tstedmaCompareResult( Ptr dst, Uint32 unSize );
Bool	tstedma1DCopyTest( Ptr src, Ptr dst, Uint32 unSize, Bool bSem );
Bool	tstedmaParallelCopyTest( Ptr src, Ptr dst, Uint32 unSize, Bool bSem );
Bool	tstedmaCopyTest2D( Int type, Ptr src, Ptr dst, Ptr cmp, Uint32 unLineLen, Uint32 unLineCnt, Uint32 unLinePitch, Bool bSem );

#ifdef __cplusplus
}
#endif

#endif
