
#include "classVisClip.h"


CVisClip::CVisClip( const Char * strName, CVisPort::PortDataType pdtType, Uint32 unOffsX, Uint32 unOffsY, Uint32 unWidth, Uint32 unHeight )
		:	CVisComponent( strName, "Clipper" ),
			m_iportImage( "input", pdtType ),
			m_oportImage( "output", pdtType,  CVisOutputPort::OUTPORT_LARGE)
{
	m_iportImage.Init( this );
	m_oportImage.Init( this );
	
	m_oportImage.SetImageSize( unWidth, unHeight );
	
	
	m_unOffsX = unOffsX;
	m_unOffsY = unOffsY;
	m_unWidth = unWidth;
	m_unHeight = unHeight;
}

// *************************************************************************

CVisClip::~CVisClip()
{
}


// *************************************************************************

void CVisClip::DoProcessing()
{
	Uint32 unInWidth;
	Uint32 unInHeight;
	Uint32 unInBytesPerPixel;
	
	const Uint32 * pInputImage	= (const Uint32*)m_iportImage.GetBuffer();
	Uint32 * pOutputImage		= (Uint32*)m_oportImage.GetBuffer();
	
	m_iportImage.GetImageSize( unInWidth, unInHeight );
	unInBytesPerPixel = m_iportImage.GetImageBPP() / 8;
	
	// Special case: the width of the input image is equal to the width of the output image.
	// in this case, we only have to propagate the buffer pointer. Otherwise, we have to
	// copy the image.
	if ( m_unWidth == unInWidth )
	{
		m_oportImage.SetBuffer( (Ptr)( (Uint8*)pInputImage + m_unWidth*m_unOffsY*unInBytesPerPixel ) );
		m_oportImage.PropagateBuffer();
	}
	else
	{
		Ptr pSrc = (Ptr)((Uint8*)pInputImage + (unInWidth * m_unOffsY)*unInBytesPerPixel + m_unOffsX*unInBytesPerPixel);
		WaitCopy( StartCopy2D( COPY_2D1D,	(Ptr)pOutputImage, pSrc , 
											m_unWidth * unInBytesPerPixel, m_unHeight , unInWidth * unInBytesPerPixel ) );
	}
}

// *************************************************************************

