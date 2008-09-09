
#include "classVisFramegrabber.h"


CVisFramegrabber::CVisFramegrabber( const Char * strName, Uint32 unWidth, Uint32 unHeight, CVisPort::PortDataType pdtOutputType )
	: 	CVisComponent( strName, "Framegrabber" ),
		m_oportImage("output", pdtOutputType, CVisOutputPort::OUTPORT_HOLLOW )
{
	m_oportImage.Init( this );
	
	m_unResultWidth = unWidth;
	m_unResultHeight = unHeight;
	
	m_pImageBuffer = NULL;
}


// *************************************************************************

void CVisFramegrabber::SetInputBuffer( Ptr	pBuffer )
{
	m_pImageBuffer = pBuffer;

	// Set the buffer in the output port and propagate to all connected
	// input ports	
	m_oportImage.SetBuffer( pBuffer );
	m_oportImage.PropagateBuffer();
}

// *************************************************************************

void CVisFramegrabber::Prepare()
{
}

// *************************************************************************
						
void CVisFramegrabber::DoProcessing()
{

	// See if an image has been set.
	if ( m_pImageBuffer == NULL )
		return;

	m_oportImage.SetBuffer( m_pImageBuffer );
	m_oportImage.PropagateBuffer();
	
	// Invalidate this image so that it's not being used twice.
	m_pImageBuffer = NULL;
}

// *************************************************************************

// *************************************************************************

// *************************************************************************

// *************************************************************************



