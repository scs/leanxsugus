
#include "classVisInputPort.h"


CVisInputPort::CVisInputPort( const Char * strName, CVisPort::PortDataType dataType )
	:	CVisPort( strName, PT_INPUT, dataType ),
		m_pOutputPort( NULL )
{
}

// *************************************************************************
	
void CVisInputPort::Connect( CVisOutputPort * pPort )
{
	pPort->Connected( this );
	
	m_pOutputPort = pPort;

	// Get the buffer from the output port in case the ports are already prepared
	// at the time of the connection
	m_pBuffer = m_pOutputPort->GetBuffer();	
}

// *************************************************************************

void CVisInputPort::Disconnect( )
{
	if ( m_pOutputPort == NULL )
		return;

	m_pOutputPort->Disconnected( this );
}

// *************************************************************************

bool CVisInputPort::GetImageSize( Uint32 & unWidth, Uint32 & unHeight)
{
	// Check if the image size has already been set.
	if ( m_unWidth > 0 )
	{
		unWidth = m_unWidth;
		unHeight = m_unHeight;
		
		return true;
	}
	
	// If not, we have to ask the output port this input port is connected to.
	else if ( m_pOutputPort!= NULL )
	{
		// First get the image dimension and then the buffer size
		if ( ! m_pOutputPort->GetImageSize( unWidth, unHeight) )
			return false;

		m_nBufferSize = m_pOutputPort->GetBufferSize();

		return true;
	}
	else
		return false;
}

// *************************************************************************

void CVisInputPort::Connected( CVisOutputPort * pPort )
{
	m_pOutputPort = pPort;
}
	
// *************************************************************************

// *************************************************************************

// *************************************************************************

// *************************************************************************	

