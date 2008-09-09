
#include "classVisOutputPort.h"

#include "classVisBufferManager.h"


CVisOutputPort::CVisOutputPort( const Char * strName, CVisPort::PortDataType dataType, Uint32 unFlags )
	:	CVisPort( strName, CVisPort::PT_OUTPUT, dataType )
{
	for (Int i=0; i<OUTPORT_MAX_CONNECTEDINPUTS; i++)
		m_aryInputPorts[i] = NULL;	
		
	m_unFlags = unFlags;

	if ( m_unFlags & OUTPORT_DOUBLEBUFFER )
		m_bDoubleBuffered = TRUE;
	else
		m_bDoubleBuffered = FALSE;
		
	if ( m_unFlags & OUTPORT_HOLLOW )
		m_bHollow = TRUE;
	else
		m_bHollow = FALSE;

	m_pBackBuffer = NULL;
}
	
// *************************************************************************
	
void CVisOutputPort::Connect( CVisInputPort * pInputPort )
{
	pInputPort->Connected( this );
	
	Connected( pInputPort );
}

// *************************************************************************
	
void CVisOutputPort::Connected( CVisInputPort * pInputPort )
{
	Int i=0;

	while ( m_aryInputPorts[i] != NULL )
	{
		i++;
		if ( i == OUTPORT_MAX_CONNECTEDINPUTS )
		{
			LogMsg( "ERROR: port list full!" );
			return;
		}
	}
	
	m_aryInputPorts[i] = pInputPort;

	// Propagate the buffer pointer and size; in case this is a re-connect
	if ( m_nBufferSize != -1 )
		PropagateBuffer();
}
// *************************************************************************

void CVisOutputPort::Disconnected( CVisInputPort * pInputPort )
{
	Int i=0;

	while ( m_aryInputPorts[i] != pInputPort )
	{
		i++;
		if ( i == OUTPORT_MAX_CONNECTEDINPUTS )
			return;
	}
	
	m_aryInputPorts[i] = NULL;
}

// *************************************************************************

void CVisOutputPort::PropagateBuffer( )
{
	// Propagate the buffer pointer to all connected input ports
	for (Int i=0; i<OUTPORT_MAX_CONNECTEDINPUTS; i++)
	{
		if (m_aryInputPorts[i] != NULL)
		{
			m_aryInputPorts[i]->SetBuffer( m_pBuffer );
			m_aryInputPorts[i]->SetBufferSize( m_nBufferSize );
		}
	}
}

// *************************************************************************

bool CVisOutputPort::Prepare()
{
	// Determine image size if not yet known. We'll accept a false as return value.
	if ( m_unWidth == 0 )
		m_pComponent->GetResultImageSize( m_unWidth, m_unHeight);
		
	// Determine BPP if not yet known
	if ( m_unBPP == 0)
		m_pComponent->GetResultImageBPP( m_unBPP );
	
	// If the buffer size is already known, use it. This will be the case with non-image
	// ports. If that's not true, calc the buffer size from the image's dimension.
	if ( m_nBufferSize == 0 )
	{
		LogMsg("bufsize = 0!");
	}
	if ( m_nBufferSize == -1 )
	{
		m_nBufferSize = (m_unWidth * m_unHeight * m_unBPP / 8);
		
		// round it
		m_nBufferSize = (m_nBufferSize + 7) & ~0x0007;
	}

	Uint32 reqSize = m_nBufferSize;

	// If this is a double buffered port, we need double the amount of memory
	// Note: the actual buffer size stays the same, we only allocate twice the memory to be able
	//		 to switch buffers later.
	if ( m_bDoubleBuffered )
		reqSize *= 2;

	// If this is a hollow buffer, don't allocate any memory for it! Likewise, if the buffer is empty
	// we don't need to allocate any memory.
	if ( !m_bHollow && (m_nBufferSize > 0))	
	{
		// Request the memory 
		if ( ! CVisBufferManager::Instance()->RequestBuffer( this, reqSize, &m_pBuffer, m_unFlags ) )
			return false;
	}

	// The second half of the allocated memory goes to the double buffer.
	if ( m_bDoubleBuffered )
		m_pBackBuffer = (Ptr)((Uint8*)m_pBuffer + m_nBufferSize);

	// And at last: propagate the buffer pointer to all connected ports.
	PropagateBuffer();
		
	return true;	
}

// *************************************************************************
	
bool CVisOutputPort::GetImageSize( Uint32 & unWidth, Uint32 & unHeight )
{
	// Check if the image size has already been set.
	if ( m_unWidth > 0 )
	{
		unWidth = m_unWidth;
		unHeight = m_unHeight;
		
		return true;
	}
	
	// If not, we have to ask this port's component.
	else if ( m_pComponent!= NULL )
	{
		// Ask it
		if ( ! m_pComponent->GetResultImageSize( m_unWidth, m_unHeight) )
			return false;

		// Store the result
		unWidth = m_unWidth;
		unHeight = m_unHeight;

		// Calculate the buffersize and round it up
		m_nBufferSize = (m_unWidth * m_unHeight * m_unBPP / 8);
		m_nBufferSize = (m_nBufferSize + 7) & ~0x0007;
		return true;
	} 
	else
		return false;
}

// *************************************************************************

void CVisOutputPort::SwapBuffers()
{
	Ptr p;

	if ( ! m_bDoubleBuffered )
		return;

	// Swap buffers
	p = m_pBackBuffer;
	m_pBackBuffer = m_pBuffer;
	m_pBuffer = p;

	// Let all connected ports know.
	PropagateBuffer();
}

// *************************************************************************

Ptr CVisOutputPort::GetBackBuffer()
{
	return m_pBackBuffer;
}

// *************************************************************************
