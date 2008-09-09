

#include "classVisHistogram.h"


// *************************************************************************

CVisHistogram::CVisHistogram( const char * strName, CVisPort::PortDataType pdtInputType, const int nNumQuantLevels )
	:	CVisComponent( strName, "Histogram" ),
		m_iportInput("input", pdtInputType ),
		m_iportMaskInput("maskInput", CVisPort::PDT_8BPP_GRAY ),
		m_oportHistogram("outputHistogram", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_LARGE ),
		m_propNumPixelsInHist( "NumPixelsInHist" )
{
	m_iportInput.Init( this );
	m_iportMaskInput.Init( this );
	m_oportHistogram.Init( this );

	// Init properties
	m_propNumPixelsInHist.Init( this, CVisProperty::PT_INTEGER, &m_nNumPixelsInHist );

	// Determine the shift amount due to quantization
	m_nNumQuantShift = 0;
	while ( ((256 / nNumQuantLevels) >> m_nNumQuantShift) != 1 )
		m_nNumQuantShift++;

	// Store the quantisation levels
	m_nNumQuantLevels = nNumQuantLevels;

	// Determine the histogram type
	m_bRGB = (		(pdtInputType == CVisPort::PDT_24BPP_RGB)
				||	(pdtInputType == CVisPort::PDT_32BPP_RGB) ) ? true : false ;

	// Determine histogram size. Note: this function depends on all the other member variables
	// being set, so call it at the end of the constructor.
	m_oportHistogram.SetBufferSize( GetHistogramSize() );
}

// *************************************************************************

void CVisHistogram::Prepare()
{
	HistogramData * pHistogram = (HistogramData*)m_oportHistogram.GetBuffer();

	// Clear the buffer
	ClearCalibrationData();
}

// *************************************************************************

void CVisHistogram::DoProcessing()
{
	Uint8 *			pMask = (Uint8*)m_iportMaskInput.GetBuffer();
	Uint32 *		pRGBImage = (Uint32*)m_iportInput.GetBuffer();
	HistogramData * pHistogram = (HistogramData*)m_oportHistogram.GetBuffer();

	// Write the quantization info to the histogram
	pHistogram->unQuantLevels = m_nNumQuantLevels;
	pHistogram->unQuantShift = m_nNumQuantShift;
	pHistogram->unOffsetShift = 8-m_nNumQuantShift;
	
	// See if we need the masked or the unmasked histogram function, depending on 
	// whether there is a port connected to the mask input port.
	CVisPort * port;
	if ( m_iportMaskInput.GetConnectedPort( port ) )
	{
		BuildRGBHistogram_msk( pRGBImage, pMask, pHistogram, m_unResultWidth*m_unResultHeight );
	}
	else
	{
		BuildRGBHistogram( pRGBImage, pHistogram, m_unResultWidth*m_unResultHeight );
	}

	m_nNumPixelsInHist = pHistogram->unNumPixels;
}

// *************************************************************************

void CVisHistogram::BuildRGBHistogram(	const Uint32 * restrict pRGBImg, 												
										HistogramData * restrict hist, 
										const Uint32 numPixels )
{
	Uint32 rgbpix;
	Uint16 * histpix;
	const int quantShift = hist->unQuantShift;
	
	for ( Uint32 i =0; i<numPixels; i++)
	{		
		rgbpix = pRGBImg[i];
	
		// Get the pointer		
		int R,G,B;
		R = ((rgbpix & 0x0000FF) >> (quantShift));
		G = ((rgbpix & 0x00FF00) >> (8+quantShift));
		B = ((rgbpix & 0xFF0000) >> (16+quantShift));
			
		histpix = &(hist->unpHistogram[ hist->GetIndex(R,G,B) ] );
	
		// See if the pixel is already saturated
		if ( *histpix != 0xFFFF )
			(*histpix) += 1;

		// Add it to the total pixel count
		hist->unNumPixels += 1;
	}
}
	
// *************************************************************************


void CVisHistogram::BuildRGBHistogram_msk(	const Uint32 * restrict pRGBImg, 
											const Uint8 * restrict pGrayscaleMask, 
											HistogramData * restrict hist, 
											const Uint32 numPixels )
{
	Uint32 rgbpix;
	Uint16 * histpix;
	Uint16 add;
	const int quantShift = hist->unQuantShift;

	for ( Uint32 i =0; i<numPixels; i++)
	{
		
		rgbpix = pRGBImg[i];
	
		// Check the mask
		add = ( pGrayscaleMask[i] != 0x00) ? 1 : 0;
		
		// Get the pointer
		int R,G,B;
		R = ((rgbpix & 0x0000FF) >> (quantShift));
		G = ((rgbpix & 0x00FF00) >> (8+quantShift));
		B = ((rgbpix & 0xFF0000) >> (16+quantShift));
			
		histpix = &(hist->unpHistogram[ hist->GetIndex(R,G,B) ] );
	
		// See if the pixel is already saturated
		if ( *histpix != 0xFFFF )
			(*histpix) += add;

		// Add it to the total pixel count
		hist->unNumPixels += add;
	}
}
				
// *************************************************************************

void CVisHistogram::ClearCalibrationData()
{
	HistogramData * pHistogram;
	
	// Writeback cache since we're going to access the buffer directly.
	m_oportHistogram.CacheWriteback();
	m_oportHistogram.CacheInvalidate();

	// Get the buffer
	pHistogram = (HistogramData*)m_oportHistogram.GetBuffer();
	
	// Clear the whole buffer
	MemSet( pHistogram, 0, m_oportHistogram.GetBufferSize() );

	// Write the quantization info to the histogram
	pHistogram->unQuantLevels = m_nNumQuantLevels;
	pHistogram->unQuantShift = m_nNumQuantShift;
	pHistogram->unOffsetShift = 8-m_nNumQuantShift;
	pHistogram->unNumChannels = m_bRGB ? 3 : 1;
		
	// And reset the pixel counting property
	m_nNumPixelsInHist = 0;
}

// *************************************************************************

Uint32 CVisHistogram::GetCalibrationData( Ptr pBuffer )
{
	// Write back cache
	m_oportHistogram.CacheWriteback();	
	
	WaitCopy( StartCopy( pBuffer, m_oportHistogram.GetBuffer(), m_oportHistogram.GetBufferSize() ));

	return GetCalibrationDataSize();
}

// *************************************************************************

Uint32 CVisHistogram::SetCalibrationData( const Ptr pBuffer )
{
	// Invalidate cache
	m_oportHistogram.CacheInvalidate();
	
	WaitCopy( StartCopy( m_oportHistogram.GetBuffer(), pBuffer, m_oportHistogram.GetBufferSize() ));
	m_oportHistogram.CacheInvalidate();
	
	// Map the number of pixels to the property.
	m_nNumPixelsInHist = ((HistogramData*)m_oportHistogram.GetBuffer())->unNumPixels;
	
	// Check if the new histogram has the same properties (quantization etc.) than this software
	// requires. Changes in quantization levels needs the software to be re-compiled and renders
	// the previous calibration data useless.
	HistogramData * pHistogram = (HistogramData*)m_oportHistogram.GetBuffer();
	if (	( pHistogram->unQuantLevels != m_nNumQuantLevels )
		||	( pHistogram->unQuantShift != m_nNumQuantShift )
		||	( pHistogram->unOffsetShift != 8-m_nNumQuantShift )
		||	( pHistogram->unNumChannels != m_bRGB ? 3 : 1 ) )
	{
		LogMsg( "Incompatible calibration data! quant: %d, shift: %d, offset: %d, chan: %d",
			pHistogram->unQuantLevels,
			pHistogram->unQuantShift,
			pHistogram->unOffsetShift,
			pHistogram->unNumChannels);
	}

	// Consistency check; we just sum up all pixels in the histogram, which should amount
	// to the number stored in numPixels. But, if some of the bins are saturated, that
	// of course won't be the case anymore.
	Uint32 numPix = 0;
	Uint32 numPixSaturated = 0;
	Uint32 nStep = 256 / m_nNumQuantLevels;
	for ( Int R=0; R<m_nNumQuantLevels; R += nStep)
		for ( Int G=0; G<m_nNumQuantLevels; G += nStep)
			for ( Int B=0; B<m_nNumQuantLevels; B += nStep)
				if ( pHistogram->GetValue( R,G,B ) == 0xFFFF )
				{
					numPixSaturated++;
					//LogMsg( "r: %d, g: %d, b: %d", i*4, j*4, k*4 );
				}
	
	if ( numPix != pHistogram->unNumPixels )
	{
		LogMsg( "Consistency error! Counted: %d, should: %d", numPix, pHistogram->unNumPixels );
		LogMsg( "%d saturated bins", numPixSaturated );
	}

	return GetCalibrationDataSize();
}

// *************************************************************************

Uint32 CVisHistogram::GetCalibrationDataSize()
{
	return GetHistogramSize();
}

// *************************************************************************

Uint32 CVisHistogram::GetHistogramSize( )
{
	int numChannels = m_bRGB ? 3 : 1;
	
	// Calculate the size of the array
	int size = m_nNumQuantLevels;
	for ( int i=1; i<numChannels; i++)
		size *= m_nNumQuantLevels;

	// We currently use 16 bit words
	size *= sizeof(Uint16);

	// Add the struct's header data
	size += sizeof(HistogramData);

	return size;
}

// *************************************************************************
