
#include "classVisPPUSim.h"

#include "classVisCalibHistogram.h"

// *************************************************************************

CVisPPUSim::CVisPPUSim( const Char * strName, int nNumQuantLevels )
	:	CVisComponent( "PPUSim", strName ),
		m_iportRGBInput("rgb32Input", CVisPort::PDT_32BPP_RGB ),
		m_iportLUT("lutInput", CVisPort::PDT_DATA ),
		m_oportGrayOutput("g8Output", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_LARGE ),
		m_oportLUTOutput("lut8Output", CVisPort::PDT_8BPP_INDEX, CVisOutputPort::OUTPORT_LARGE),
		m_propNumErosions( "numErosions" )
{
	m_iportRGBInput.Init( this );
	m_iportLUT.Init( this );

	m_oportGrayOutput.Init( this );
	m_oportLUTOutput.Init( this );

	m_propNumErosions.Init( this, CVisProperty::PT_INTEGER, &m_nNumErosions );
	m_nNumErosions = 2;

	m_nNumQuantLevels = nNumQuantLevels;	
}

// *************************************************************************

void CVisPPUSim::Prepare()
{
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pRGBLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 3, (Ptr*)&m_pGrayLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 3, (Ptr*)&m_pLUTData, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth , (Ptr*)&m_pErodedLUTData, CVisBufferManager::BUF_FAST );
}

// *************************************************************************

void CVisPPUSim::DoProcessing()
{
	Int lines;
	
	// Copy ids;
	Uint32		copyRGB;
	Uint32		copyLUT;
	Uint32		copyGray;
	Uint32		copyErodedLUT;
	
	// Acquire the image buffers
	const Uint8 * pLUT			= (Uint8*)m_iportLUT.GetBuffer();
	const Uint32 * pRGBImage	= (const Uint32*)m_iportRGBInput.GetBuffer();
	Uint8 * pGrayImage			= (Uint8*)m_oportGrayOutput.GetBuffer();	
	Uint8 * pLUTImage			= (Uint8*)m_oportLUTOutput.GetBuffer();

	// ***************************************************************
	//  Process the image line by line; First convert to gray and do
	//	the LUT lookup.
	// ***************************************************************
	
	// Prologue; copy the RGB lines from the image
	copyRGB = StartCopy( (Ptr)m_pRGBLine, (Ptr)pRGBImage, m_unResultWidth * 4 );
	copyGray = COPY_DONE;
	copyLUT = COPY_DONE;
	
	// Kernel
	for ( lines = 0; lines < m_unResultHeight; lines++)
	{
		Uint32 offs = lines * m_unResultWidth;

		// Convert RGB to gray
		// -> need RGB input
		// -> need gray
		WaitCopy( copyRGB );
		WaitCopy( copyGray );
		RGBtoGray( m_pRGBLine, (Uint32*)(m_pGrayLine), m_unResultWidth );	
		
	
		// Lookup the RGB pixels; but not on first and last lines.
		// -> done with gray afterwards
		// -> done with RGB afterwards
		// -> done with LUT afterwards
		WaitCopy( copyLUT );
		if ( (lines != 0) && (lines != m_unResultHeight-1) )
			LookupPixels( m_pRGBLine, m_pLUTData, m_pGrayLine, pLUT, m_unResultWidth );
		else
			MemSet( m_pLUTData, 0, m_unResultWidth );

		if ( lines < m_unResultHeight-1)
			copyRGB = StartCopy( (Ptr)m_pRGBLine, (Ptr)(pRGBImage + offs + m_unResultWidth), m_unResultWidth * 4 );
			
		copyGray = StartCopy( (Ptr)(pGrayImage + offs), m_pGrayLine, m_unResultWidth );

		copyLUT = StartCopy( (Ptr)(pLUTImage + offs), m_pLUTData, m_unResultWidth );
	}

	// Epilogue
	//WaitCopy( copyRGB );
	WaitCopy( copyLUT );
	WaitCopy( copyGray );


	// ***************************************************************
	//  Erode the image.
	// ***************************************************************

	for (int i=0; i<m_nNumErosions ; i++)
	{
		// To keep things simple, we'd like to do the erosion in place, 
		// that is the input and output port is the same. That behaviour
		// implies that we have to copy the new input data before writing
		// the output data back to the image.

		// Copy the input data.
		copyLUT = StartCopy(	m_pLUTData, 
								(Ptr)(pLUTImage) , 
								m_unResultWidth * 3);
		WaitCopy( copyLUT );

		for ( lines = 1; lines < m_unResultHeight-1; lines++)
		{
			// Erode a line
			ErodeLine_lut( m_pLUTData, m_pErodedLUTData, m_unResultWidth );

			// Copy the next input data.
			copyLUT = StartCopy(	m_pLUTData, 
									(Ptr)(pLUTImage + lines * m_unResultWidth) , 
									m_unResultWidth * 3 );

			// Wait for the copy to finish, so that the output can then be written.
			WaitCopy( copyLUT );

			// Copy the result data out
			copyErodedLUT = StartCopy(	(Ptr)(pLUTImage + lines*m_unResultWidth) , 
										m_pErodedLUTData,
										m_unResultWidth );

			WaitCopy( copyErodedLUT );
		}
	}


	// Clear the most left and right rows.
	for ( lines = 0; lines < m_unResultHeight; lines++)
	{
		pLUTImage[ lines*m_unResultWidth ] = 0;
		pLUTImage[ lines*m_unResultWidth + m_unResultWidth - 1] = 0;
	}


/*	
	// Convert the image to grayscale
	StartProfileTask( m_nProfile_1 );
	RGBtoGray( pRGBImage, (Uint32*)(pGrayImage), m_unResultWidth*m_unResultHeight );	
	StopProfileTask( m_nProfile_1 );

	// do the LUT conversion 
	StartProfileTask( m_nProfile_2 );
	LookupPixels( pRGBImage, pLUTImage, m_unResultWidth*m_unResultHeight );
	StopProfileTask( m_nProfile_2 );

	// Mask out all grayscale pixels where there is background
	StartProfileTask( m_nProfile_3 );
	for ( Uint32 i=0; i<m_unResultWidth*m_unResultHeight; i++)
		if ( pLUTImage[i] == 0 )
			pGrayImage[i] = 0;
			
	StopProfileTask( m_nProfile_3 );
	*/
/*
	// Do Erosion
	for ( int er=0; er<CVisCalibHistogram::NUM_EROSIONS; er++)
	{
		for ( Uint32 line=1; line<m_unResultHeight-1; line++)
		{
			Uint32 offs = line * m_unResultWidth;
			ErodeLine( pLUTImage + offs, pErodedImage + offs + m_unResultWidth, m_unResultWidth );
		}
		memcpy( pLUTImage, pErodedImage, m_unResultWidth*m_unResultHeight );
	}
	*/
}

// *************************************************************************

void CVisPPUSim::RGBtoGray( const Uint32 * restrict pRGBImage, Uint32 * restrict pGrayImage, const Uint32 numPixels )
{
	// Define a multiplication mask for use with the DOTP intrinsics. Multiply R, G
	// and B by 85 and A by 0. Then, divide it by 256, so that we get a factor of
	// 1/3 ( = 85/256).
	//#define MULTIPLYMASK 0x00555555		

	// Out = R*0.3 + G*0.59 + B*0.11
	#define MULTIPLYMASK 0x001C964C


	Uint32 n1, n2, n3, n4;
	Uint32 m1, m2;

	for( unsigned int i=0; i<numPixels ; i+=4)
	{
		
		n1 = _dotpu4 ( pRGBImage[i], MULTIPLYMASK ) >> 8;
		n2 = _dotpu4 ( pRGBImage[i+1], MULTIPLYMASK ) >> 8;		
		n3 = _dotpu4 ( pRGBImage[i+2], MULTIPLYMASK ) >> 8;
		n4 = _dotpu4 ( pRGBImage[i+3], MULTIPLYMASK ) >> 8;
		
		m1 = _pack2(n2,n1);
		m2 = _pack2(n4,n3);

		pGrayImage[i>>2] = _packl4(m2,m1);
	}
}

// *************************************************************************

void CVisPPUSim::LookupPixels( const Uint32 * restrict pRGBImage, Uint8 * restrict pLUTImage, Uint8 * restrict pGrayImage, const Uint8 * restrict unpLUT, const Uint32 numPixels)
{
	// Determine the quantisation division factor
	Uint32 quantdiv = 256 / m_nNumQuantLevels;
	Int quantshift = 0;
	while ( (quantdiv >> quantshift) > 1 )
		quantshift++;
	
	Uint8 lookedup;

	for ( Uint32 i=0; i<numPixels; i++)
	{
		Uint8	R,G,B;
		Uint32	LUTindex;
		Uint32	ByteIndex;
		Uint8	LUTEntry;

		// Extract color values
		R = (Uint8)( pRGBImage[i] & 0x0000FF);
		G = (Uint8)((pRGBImage[i] & 0x00FF00) >> 8);
		B = (Uint8)((pRGBImage[i] & 0xFF0000) >> 16);

		// quantize
		R >>= quantshift;
		G >>= quantshift;
		B >>= quantshift;

		// generate indices
		LUTindex = ( R + G*m_nNumQuantLevels + B*m_nNumQuantLevels*m_nNumQuantLevels ) >> 2;
		ByteIndex = R & 0x03;

		// Read entry
		LUTEntry = unpLUT[ LUTindex ];		

		// Extract 2 bit value
		lookedup = (LUTEntry >> (ByteIndex*2)) & 0x03;
		pLUTImage[i] = lookedup;
		
		// Mask out background in grayscale image
		if (lookedup == 0)
			pGrayImage[i] = 0;
	}
}

// *************************************************************************

void CVisPPUSim::ErodeLine_lut( const Uint8 * restrict grayInput, Uint8 * restrict grayOutput, const Uint32 cols )
{
    Uint32 in;		// Input pixel offset
    Uint32 out;		// Output pixel offset.
	
    Uint8 i01, i02;
    Uint8 i11, i12;
    Uint8 i21, i22;
	Uint8 c0,  c1,  c2;
	
	Uint8 c;
	
	// Pre-load registers. We only need register cols 1 to 2 to which we load
	// pixel cols 0 and 1, since the first thing we'll do in the kernel is shift
	// register cols 1 and 2 one to the left.
	i01=grayInput[0		]; i02=grayInput[1			 ];
    i11=grayInput[cols	]; i12=grayInput[cols +		1];
	i21=grayInput[2*cols]; i22=grayInput[2*cols +	1];

	// compute column 1
	if ( (i01==0) || (i11==0) || (i21==0) )
		c1 = 0;
	else
		c1 = 255;

	// compute column 2
	if ( (i02==0) || (i12==0) || (i22==0) )
		c2 = 0;
	else
		c2 = 255;
	
    for (	in = cols + 1, out = 1;
			out < cols-1;
			in++, out++)
    {
		// shift registers to left; we'll need the temporary column erosion
		// values and the center pixel.
		i11 = i12;
		
		c0 = c1;
		c1 = c2;

		// load new values
		i02=grayInput[in-cols+1];
		i12=grayInput[in     +1];
		i22=grayInput[in+cols+1];

		// compute new column
		if ( (i02==0) || (i12==0) || (i22==0) )
			c2 = 0;
		else
			c2 = 255;

		// Now use the columns to calculate overall erosion
		if (	(c0 == 0)
			||	(c1 == 0)
			||	(c2 == 0))
			c = 0;
		else
			c = i11;

		grayOutput[ out ] = c;		
    }        
    
  
}

// *************************************************************************

// *************************************************************************

// *************************************************************************
