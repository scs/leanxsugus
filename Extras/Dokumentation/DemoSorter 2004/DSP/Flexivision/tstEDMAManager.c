

#include "tstEDMAManager.h"
#include "libEDMAManager.h"

#include <stdio.h>

// *************************************************************************

void	tstedmaPrepareBuffers( Ptr src, Ptr dst, Uint32 unSize )
{
	Int i;
	
	volatile Uint8 * s;
	volatile Uint8 * d;
	
	s = (Uint8*)src;
	d = (Uint8*)dst;

	for ( i=0; i<unSize; i++)
	{
		s[i] = (Uint8)(i & 0xFF);
		d[i+4] = ~(Uint8)(i & 0xFF);
	}
	
	// Also write guard bytes
	d[0] = 0xEF;
	d[1] = 0xBE;
	d[2] = 0xAD;
	d[3] = 0xDE;
	
	d[i+4] = 0xEF;
	d[i+5] = 0xBE;
	d[i+6] = 0xAD;
	d[i+7] = 0xDE;	
}


// *************************************************************************

void	tstedmaPrepareBuffers2D( Ptr src, Ptr dst, Uint32 unSize )
{
	Int i;
	
	volatile Uint8 * s;
	volatile Uint8 * d;
	
	s = (Uint8*)src;
	d = (Uint8*)dst;

	for ( i=0; i<unSize; i++)
	{
		s[i] = (Uint8)(i & 0xFF);
		d[i+4] = 0xAA;
	}
	
	// Also write guard bytes
	d[0] = 0xEF;
	d[1] = 0xBE;
	d[2] = 0xAD;
	d[3] = 0xDE;
	
	d[i+4] = 0xEF;
	d[i+5] = 0xBE;
	d[i+6] = 0xAD;
	d[i+7] = 0xDE;	
}

// *************************************************************************


Bool	tstedmaCompareResult( Ptr dst, Uint32 unSize )
{
	volatile Uint8 * d;
	Int i;
	Int errors=0;
		
	d = (Uint8*)dst;
	
	for ( i=0; i<unSize; i++)
	{
		if ( d[i+4] != (Uint8)(i & 0xFF) )
		{
			if ( errors < 10 )
			{
				printf("Compare error at %d. Should: %02xh, is %02xh\n", i, (Uint8)(i & 0xFF), d[i+4] );
				errors++;
			}
		}
	}
	
	// Check guards
	if ( 	d[0] != 0xEF
		||	d[1] != 0xBE
		||	d[2] != 0xAD
		||	d[3] != 0xDE )
	{
		printf("Lower guard error!\n");
		return FALSE;
	}
	
	if ( 	d[i+4] != 0xEF
		||	d[i+5] != 0xBE
		||	d[i+6] != 0xAD
		||	d[i+7] != 0xDE )
	{
		printf("Upper guard error!\n");
		return FALSE;
	}

	return TRUE;
}

// *************************************************************************

void tstedmaCopy2D( Uint32 unType, Ptr dst, Ptr src, Uint32 unLineLen, Uint32 unLineCnt, Uint32 unLinePitch )
{
	Uint8 * d;
	Uint8 * s;
	Int i;
	
	d = (Uint8*)dst;
	s = (Uint8*)src;
	
	switch ( unType )
	{
	case EDMACOPY_1D1D:
		memcpy( d, s, unLineLen * unLineCnt );
		break;

	case EDMACOPY_1D2D:
		for ( i=0; i<unLineCnt; i++ )
		{
			memcpy( d, s, unLineLen );
			s += unLineLen;
			d += unLinePitch;
		}
		break;
		
	case EDMACOPY_2D1D:
		for ( i=0; i<unLineCnt; i++ )
		{
			memcpy( d, s, unLineLen );
			s += unLinePitch;
			d += unLineLen;
		}
		break;
		
	case EDMACOPY_2D2D:
		for ( i=0; i<unLineCnt; i++ )
		{
			memcpy( d, s, unLineLen );
			d += unLinePitch;
			s += unLinePitch;
		}
		break;
	}		
}

// *************************************************************************

Bool	tstedmaCompareResultBf( Ptr dst, Ptr cmp, Uint32 unSize)
{
	Int i;
	Int errors = 0;

	Uint8 * d = (Uint8*)dst;
	Uint8 * c = (Uint8*)cmp;
	
	for ( i=0; i<unSize; i++)
	{
		if ( d[i] != c[i] )
		{
			if ( errors < 10 )
			{
				printf("Error at %d, is: %02Xh, should: %02x\n", i, d[i], c[i] );
				errors++;
			}
		}
	}
	
	return (errors == 0);	
}

// *************************************************************************

Bool	tstedma1DCopyTest( Ptr src, Ptr dst, Uint32 unSize, Bool bSem )
{
	Int copy;
	
	tstedmaPrepareBuffers( src, dst, unSize );
	
	// Issue copy
	copy = edmaStartCopy( (Ptr)((Uint32)dst + 4), src, unSize, bSem );
	edmaWaitCopy( copy );
	
	// Now test...
	return tstedmaCompareResult( dst, unSize );
}

// *************************************************************************

Bool	tstedmaParallelCopyTest( Ptr src, Ptr dst, Uint32 unSize, Bool bSem )
{
	#define numChannels 8
	
	Int i=0;	
	Int blocksize;	
	Int testsize;
	Int copy[numChannels];
	Bool bRes = TRUE;
	
	blocksize = unSize / numChannels;
	testsize = blocksize - 8;
	
	for ( i=0; i<numChannels; i++)
		tstedmaPrepareBuffers( (Ptr)((Uint32)src + blocksize*i), (Ptr)((Uint32)dst + blocksize*i), testsize );

	for ( i=0; i<numChannels; i++)
	{
		copy[i] = edmaStartCopy(  (Ptr)((Uint32)dst + blocksize*i + 4), (Ptr)((Uint32)src + blocksize*i), testsize, bSem );
		if (copy[i] == -1)
			printf("Copy issue #%d failed\n", i );
			
	}
	
	for ( i=0; i<numChannels; i++)
		edmaWaitCopy( copy[i] );
		
	for ( i=0; i<numChannels; i++)
		bRes = bRes && tstedmaCompareResult( (Ptr)((Uint32)dst + blocksize*i), testsize );

	return bRes;			
	
}

// *************************************************************************

Bool	tstedmaCopyTest2D( Int type, Ptr src, Ptr dst, Ptr cmp, Uint32 unLineLen, Uint32 unLineCnt, Uint32 unLinePitch, Bool bSem )
{
	Int testsize;
	Int copy;
	
	testsize = (unLinePitch * unLineCnt) / numChannels;
		
	tstedmaPrepareBuffers2D( src, dst, testsize );
	tstedmaPrepareBuffers2D( src, cmp, testsize );

	copy = edmaStartCopy2D( type, (Ptr)((Uint32)dst + 4), src, unLineLen, unLineCnt, unLinePitch, bSem );
	if (copy == -1)
		printf("Copy issue failed\n");
		
	// Create comparison buffer
	tstedmaCopy2D( type, (Ptr)((Uint32)cmp + 4), src, unLineLen, unLineCnt, unLinePitch );

	edmaWaitCopy( copy );
	
	return tstedmaCompareResultBf( dst, cmp, testsize + 8 );
	
}

// *************************************************************************


