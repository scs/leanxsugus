
#include "classVisMorphology.h"

//DEBUG
#include <string.h>

CVisMorphology::CVisMorphology( const char * strName, CVisPort::PortDataType pdtInputType, MorphologyOperation moOperation)
	:	CVisComponent(	strName, "Morphology" ),
		m_iportInput( "input", pdtInputType ),
		m_oportOutput( "output", pdtInputType, CVisOutputPort::OUTPORT_FAST ),
		m_propNumIterations( "numIterations" )
{
	m_iportInput.Init( this );
	m_oportOutput.Init( this );

	m_propNumIterations.Init( this, CVisProperty::PT_INTEGER, & m_nNumIterations );
		
	m_moOperationType = moOperation;

	m_nNumIterations = 5;
	
}

// *************************************************************************

CVisMorphology::~CVisMorphology()
{
}

// *************************************************************************

void CVisMorphology::Prepare()
{
	Uint32 unSize;

	// We need four times the room of a single line
	unSize = m_oportOutput.GetImageBPP() * m_unResultWidth * 4;

	CVisBufferManager::Instance()->RequestBuffer( this, unSize, (Ptr*)&m_pTempLines, CVisBufferManager::BUF_FAST );
}

// *************************************************************************

void CVisMorphology::DoProcessing()
{
	Uint8 * pInput = (Uint8*)m_iportInput.GetBuffer();
	Uint8 * pOutput = (Uint8*)m_oportOutput.GetBuffer();

	Int bpp = m_iportInput.GetImageBPP();
	if ( (bpp != 8) && (bpp != 1) )
		return;

	switch ( m_moOperationType )
	{
	case MORPH_DILATE:
		DilateImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		break;
		
	case MORPH_ERODE:
		ErodeImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		break;		
		
	case MORPH_OPEN:
		ErodeImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		DilateImg( pOutput, pOutput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		break;

	case MORPH_CLOSE:		
		DilateImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		ErodeImg( pOutput, pOutput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );		
		break;

	case MORPH_TOPHAT:
		// First open the image
		ErodeImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		DilateImg( pOutput, pOutput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		Difference( pOutput, pInput, pOutput, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		break;

	case MORPH_BLACKHAT:
		// First close the image
		DilateImg( pOutput, pInput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		ErodeImg( pOutput, pOutput, m_nNumIterations, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		Difference( pOutput, pOutput, pInput, bpp, m_unResultWidth, m_unResultHeight, m_unResultWidth, m_unResultWidth );
		break;
	}

}

// *************************************************************************

void CVisMorphology::ErodeImg(	Uint8 * restrict dst, const Uint8 * restrict src, 
								const Int iterations,
								const Int bpp,
								const Int cols, const Int rows, 
								const Int srcPitch,  const Int dstPitch )
{
	Int it = iterations;

	// do the real processing
	if ( bpp == 8 )
	{
		// Process the image line-wise for the first time
		ErodeImg_8u( dst, src, cols, rows, srcPitch, dstPitch );
		it--;

		while( it > 0 )
		{
			ErodeImg_8u( dst, dst, cols, rows, srcPitch, dstPitch );
			it--;
		}
	}
	else if ( bpp == 1 )
	{
	}

}

// *************************************************************************

void CVisMorphology::DilateImg(	Uint8 * restrict dst, const Uint8 * restrict src, 
								const Int iterations,
								const Int bpp,
								const Int cols, const Int rows, 
								const Int srcPitch,  const Int dstPitch )
{
	Int it = iterations;

	// do the real processing
	if ( bpp == 8 )
	{
		// Process the image line-wise for the first time
		DilateImg_8u( dst, src, cols, rows, srcPitch, dstPitch );
		it--;

		while( it > 0 )
		{
			DilateImg_8u( dst, dst, cols, rows, srcPitch, dstPitch );
			it -= 1;
		}
	}
	else if ( bpp == 1 )
	{
	}
}

// *************************************************************************

void CVisMorphology::Difference(	Uint8 * restrict dst, Uint8 * restrict src1, const Uint8 * restrict src2,
									const Int bpp,
									const Int cols, const Int rows, 
									const Int srcPitch,  const Int dstPitch )
{

	if ( bpp == 8 )
	{
		Difference_8u( dst, src1, src2, cols, rows, srcPitch, dstPitch );
	}
	else if ( bpp == 1 )
	{
	}

}

// *************************************************************************

void CVisMorphology::ErodeImg_8u(	Uint8 * restrict dst, const Uint8 * restrict src, 
									const Int cols, const Int rows, 
									const Int srcPitch,  const Int dstPitch )
{
	Uint8 * pTempLines[4] = { m_pTempLines, m_pTempLines + dstPitch, m_pTempLines + 2*dstPitch, m_pTempLines + 3*dstPitch };

	Int nCurDestLine = 1;
	Int nCurSourceLine = 0;
//	Uint32 copyId;

	// First clear the first and the last lines.
	for ( Int i=0; i<cols; i++)
	{
		dst[ i ] = 255;
		dst[ i + (rows-1)*dstPitch ] = 255;
	}

	// Prolog
	ErodeLine_8u( pTempLines[nCurDestLine], src + nCurSourceLine*srcPitch, srcPitch, cols );
	nCurDestLine++;
	nCurSourceLine++;

	ErodeLine_8u( pTempLines[nCurDestLine], src + nCurSourceLine*srcPitch, srcPitch, cols );
	nCurDestLine++;
	nCurSourceLine++;

	// Kernel
	while( nCurDestLine < rows-1 )
	{
		//copyId = StartCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
		// DEBUG:
		QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );

		ErodeLine_8u( pTempLines[ nCurDestLine & 0x03 ], src + nCurSourceLine*srcPitch, srcPitch, cols );
		
		nCurDestLine++;
		nCurSourceLine++;

		//WaitCopy( copyId );
	}

	QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
	nCurDestLine++;

	QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
	nCurDestLine++;


	/*
	Uint8 * pFirstLine = m_pTempLines;
	Uint8 * pSecondLine = m_pTempLines + dstPitch;

	// First clear the first and the last lines.
	for ( Int i=0; i<cols; i++)
	{
		dst[ i ] = 255;
		dst[ i + (rows-1)*dstPitch ] = 255;
	}

	// Prolog
	ErodeLine_8u( pFirstLine, src, srcPitch, cols );

	// Kernel
	for ( Int line=2; line<rows-2; line+=2 )
	{

		ErodeLine_8u( pSecondLine, src + (line-1)*srcPitch, srcPitch, cols );

		WaitCopy( StartCopy( dst + (line-1)*dstPitch, pFirstLine, cols ));

		ErodeLine_8u( pFirstLine, src + (line)*srcPitch, srcPitch, cols );

		WaitCopy( StartCopy( dst + (line)*dstPitch, pSecondLine, cols ));
	}

	// Epilogue
	ErodeLine_8u( pSecondLine, src + (rows-3)*srcPitch, srcPitch, cols );
	WaitCopy( StartCopy( dst + (rows-3)*dstPitch, pFirstLine, cols ));
	WaitCopy( StartCopy( dst + (rows-2)*dstPitch, pSecondLine, cols ));
	*/	


}

// *************************************************************************

void CVisMorphology::DilateImg_8u(	Uint8 * restrict dst, const Uint8 * restrict src, 
									const Int cols, const Int rows, 
									const Int srcPitch,  const Int dstPitch )
{
		Uint8 * pTempLines[4] = { m_pTempLines, m_pTempLines + dstPitch, m_pTempLines + 2*dstPitch, m_pTempLines + 3*dstPitch };

	Int nCurDestLine = 1;
	Int nCurSourceLine = 0;
	Uint32 copyId;

	// First clear the first and the last lines.
	for ( Int i=0; i<cols; i++)
	{
		dst[ i ] = 0;
		dst[ i + (rows-1)*dstPitch ] = 0;
	}

	// Prolog
	DilateLine_8u( pTempLines[nCurDestLine], src + nCurSourceLine*srcPitch, srcPitch, cols );
	nCurDestLine++;
	nCurSourceLine++;

	DilateLine_8u( pTempLines[nCurDestLine], src + nCurSourceLine*srcPitch, srcPitch, cols );
	nCurDestLine++;
	nCurSourceLine++;

	// Kernel
	while( nCurDestLine < rows-1 )
	{
		//copyId = StartCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
		// DEBUG:
		QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );

		DilateLine_8u( pTempLines[ nCurDestLine & 0x03 ], src + nCurSourceLine*srcPitch, srcPitch, cols );
		
		nCurDestLine++;
		nCurSourceLine++;

		//WaitCopy( copyId );
	}

	QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
	nCurDestLine++;

	QuickCopy( dst + (nCurDestLine-2)*dstPitch, pTempLines[ (nCurDestLine-2) & 0x03 ], cols );
	nCurDestLine++;

	/*
	Uint8 * pFirstLine = m_pTempLines;
	Uint8 * pSecondLine = m_pTempLines + dstPitch;

	// First clear the first and the last lines.
	for ( Int i=0; i<cols; i++)
	{
		dst[ i ] = 0;
		dst[ i + (rows-1)*dstPitch ] = 0;
	}

	// Prolog
	DilateLine_8u( pFirstLine, src, srcPitch, cols );

	// Kernel
	for ( Int line=2; line<rows-2; line+=2 )
	{

		DilateLine_8u( pSecondLine, src + (line-1)*srcPitch, srcPitch, cols );

		WaitCopy( StartCopy( dst + (line-1)*dstPitch, pFirstLine, cols ));

		DilateLine_8u( pFirstLine, src + (line)*srcPitch, srcPitch, cols );

		WaitCopy( StartCopy( dst + (line)*dstPitch, pSecondLine, cols ));
	}

	// Epilogue
	DilateLine_8u( pSecondLine, src + (rows-3)*srcPitch, srcPitch, cols );
	WaitCopy( StartCopy( dst + (rows-3)*dstPitch, pFirstLine, cols ));
	WaitCopy( StartCopy( dst + (rows-2)*dstPitch, pSecondLine, cols ));
	*/
}


// *************************************************************************

void CVisMorphology::Difference_8u(	Uint8 * restrict dst, const Uint8 * restrict src1, const Uint8 * restrict src2, 
										const Int cols, const Int rows, 
										const Int srcPitch,  const Int dstPitch )
{
	for ( Int line=0; line<rows; line++ )
		DifferenceLine_8u( dst + line*dstPitch, src1 + line*srcPitch, src2 + line*srcPitch,  cols );
		
		
	// clear the first and the last lines, since the usually don't hold any sensible data.
	for ( Int i=0; i<cols; i++)
	{
		dst[ i ] = 0;
		dst[ i + (rows-1)*dstPitch ] = 0;
	}
}

// *************************************************************************

void CVisMorphology::ErodeLine_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int srcPitch, const Int numPixels )
{
	
	Uint32	row0, row1, row2;
    Uint32  m1, m2;
		
	// Pre-load rows
	row0 = (src[0] << 8) 				| (src[1]);
	row1 = (src[srcPitch] << 8) 		| (src[srcPitch + 1]);
	row2 = (src[2*srcPitch] << 8) 		| (src[2*srcPitch + 1]);

    for ( Int x = 1; x < numPixels-1; x++)
    {
		// shift registers to left; we'll need the temporary column erosion
		// values and the center pixel.
		row0 <<= 8;
		row1 <<= 8;
		row2 <<= 8;

		// load new values
		row0 |= src[x + 1];
		row1 |= src[x + srcPitch + 1];
		row2 |= src[x + 2*srcPitch + 1];
		
		// compute minimum
		m1 = _minu4( row0, row1 );
		m1 = _minu4( m1, row2 );
		m2 = _minu4( m1, m1<<8 );
		m2 = _minu4( m2, m1<<16 );
		
		// Store it.
		dst[ x ] = (m2 & 0x00FF0000) >> 16;		
    }       
    
    dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];
	
/*
	Uint8 i01, i02;
    Uint8 i11, i12;
    Uint8 i21, i22;
	Uint8 c0,  c1,  c2;
	
	Uint8 c;
	
	// Pre-load registers. We only need register cols 1 to 2 to which we load
	// pixel cols 0 and 1, since the first thing we'll do in the kernel is shift
	// register cols 1 and 2 one to the left.
	i01=src[0			]; 		i02=src[1			 ];
    i11=src[srcPitch	]; 		i12=src[srcPitch +	1];
	i21=src[2*srcPitch 	]; 		i22=src[2*srcPitch +	1];

	// compute column 1 and 2
	c1 = min( i01, min( i11, i21 ) );	
	c2 = min( i02, min( i12, i22 ) );
	
    for ( Int x = 1; x < numPixels-1; x++)
    {
		// shift registers to left; we'll need the temporary column erosion
		// values and the center pixel.
		i11 = i12;
		
		c0 = c1;
		c1 = c2;

		// load new values
		i02=src[x + 1];
		i12=src[x + srcPitch + 1];
		i22=src[x + 2*srcPitch + 1];

		// compute new column
		c2 = min( i02, min( i12, i22 ) );

		// Now use the columns to calculate overall erosion
		c = min( c0, min( c1, c2) );
		
		// Store it.
		dst[ x ] = c;		
    }       
    
    dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];
*/	
	/*
	for ( Int x=1; x<numPixels-1; x++ )
	{
		Uint8 res = 255;
		
		res = min( res, src[ x-1 ]				);
		res = min( res, src[ x ]				);
		res = min( res, src[ x+1 ]				);

		res = min( res, src[ x-1 + srcPitch ]	);
		res = min( res, src[ x + srcPitch ]		);
		res = min( res, src[ x+1 + srcPitch ]	);

		res = min( res, src[ x-1 + 2*srcPitch ]	);		
		res = min( res, src[ x + 2*srcPitch ]	);
		res = min( res, src[ x+1 + 2*srcPitch ]	);
		
		dst[ x ] = res;
	}

	dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];
	*/
}	

// *************************************************************************

void CVisMorphology::DilateLine_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int srcPitch, const Int numPixels )
{
	Uint32	row0, row1, row2;
    Uint32  m1, m2;
		
	// Pre-load rows
	row0 = (src[0] << 8) 				| (src[1]);
	row1 = (src[srcPitch] << 8) 		| (src[srcPitch + 1]);
	row2 = (src[2*srcPitch] << 8) 		| (src[2*srcPitch + 1]);

    for ( Int x = 1; x < numPixels-1; x++)
    {
		// shift registers to left; we'll need the temporary column erosion
		// values and the center pixel.
		row0 <<= 8;
		row1 <<= 8;
		row2 <<= 8;

		// load new values
		row0 |= src[x + 1];
		row1 |= src[x + srcPitch + 1];
		row2 |= src[x + 2*srcPitch + 1];
		
		// compute minimum
		m1 = _maxu4( row0, row1 );
		m1 = _maxu4( m1, row2 );
		m2 = _maxu4( m1, m1<<8 );
		m2 = _maxu4( m2, m1<<16 );
		
		// Store it.
		dst[ x ] = (m2 & 0x00FF0000) >> 16;		
    }       
    
    dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];

	/*
	Uint8 i01, i02;
    Uint8 i11, i12;
    Uint8 i21, i22;
	Uint8 c0,  c1,  c2;
	
	Uint8 c;
	
	// Pre-load registers. We only need register cols 1 to 2 to which we load
	// pixel cols 0 and 1, since the first thing we'll do in the kernel is shift
	// register cols 1 and 2 one to the left.
	i01=src[0			]; 		i02=src[1			 ];
    i11=src[srcPitch	]; 		i12=src[srcPitch +	1];
	i21=src[2*srcPitch 	]; 		i22=src[2*srcPitch +	1];

	// compute column 1 and 2
	c1 = max( i01, max( i11, i21 ) );	
	c2 = max( i02, max( i12, i22 ) );
	
    for ( Int x = 1; x < numPixels-1; x++)
    {
		// shift registers to left; we'll need the temporary column erosion
		// values and the center pixel.
		i11 = i12;
		
		c0 = c1;
		c1 = c2;

		// load new values
		i02=src[x + 1];
		i12=src[x + srcPitch + 1];
		i22=src[x + 2*srcPitch + 1];

		// compute new column
		c2 = max( i02, max( i12, i22 ) );

		// Now use the columns to calculate overall erosion
		c = max( c0, max( c1, c2) );
		
		// Store it.
		dst[ x ] = c;		
    }       
    
    dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];
	
/*
	for ( Int x=1; x<numPixels-1; x++ )
	{
		Uint8 res = 0;
		
		res = max( res, src[ x-1 ]				);
		res = max( res, src[ x ]				);
		res = max( res, src[ x+1 ]				);

		res = max( res, src[ x-1 + srcPitch ]	);
		res = max( res, src[ x + srcPitch ]		);
		res = max( res, src[ x+1 + srcPitch ]	);

		res = max( res, src[ x-1 + 2*srcPitch ]	);		
		res = max( res, src[ x + 2*srcPitch ]	);
		res = max( res, src[ x+1 + 2*srcPitch ]	);
		
		dst[ x ] = res;
	}

	dst[0] = dst[1];
	dst[numPixels-1] = dst[numPixels-2];
*/
}

// *************************************************************************

void CVisMorphology::DifferenceLine_8u( Uint8 * restrict dst, const Uint8 * restrict src1, const Uint8 * restrict src2,  const Int numPixels )
{
	Int n = 0;

	while ( n < numPixels - 3 )
	{
		((Uint32*)dst)[n/4] = _subabs4( ((Uint32*)src1)[n/4],  ((Uint32*)src2)[n/4] );
		n += 4;
	}

	while( n < numPixels )
	{
		dst[n] = (Uint8)(abs( (Int)src1[n] - (Int)src2[n] ));
	}

}

// *************************************************************************
