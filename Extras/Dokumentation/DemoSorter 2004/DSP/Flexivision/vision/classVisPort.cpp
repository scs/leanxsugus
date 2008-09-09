

#include "classVisPort.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

CVisPort::CVisPort( const Char * strName, PortType type, PortDataType dataType )
	: CVisObject( strName, CVisObject::CT_PORT )
{
	m_ptType = type;
	m_pdtDataType = dataType;
	
	m_pBuffer = NULL;
	m_nBufferSize = -1;
	
	m_pComponent = NULL;
	
	m_unWidth = 0;
	m_unHeight = 0;

	// Determine bits per pixel
	m_unBPP = GetBppFromType( m_pdtDataType );

	m_pConnectedPort = NULL;
	
}

// *************************************************************************

void CVisPort::SetDataType( PortDataType type )
{
	m_pdtDataType = type;
}

// *************************************************************************

void CVisPort::Init( CVisComponent * pComp )
{
	m_pComponent = pComp;	
	
	pComp->AddPort( this );
}

// *************************************************************************

CVisComponent * CVisPort::GetComponent()
{
	return m_pComponent;
}

// *************************************************************************

void CVisPort::SetBufferSize( Uint32 unBufSize )
{
	m_nBufferSize = (Int)unBufSize;
}

// *************************************************************************	

void CVisPort::SetImageSize( Uint32 unWidth, Uint32 unHeight )
{
	m_unWidth = unWidth;
	m_unHeight = unHeight;
}

// *************************************************************************	

void CVisPort::SetImageBPP( Uint32 unBpp )
{
	m_unBPP = unBpp;
}

// *************************************************************************	

Uint32 CVisPort::GetImageBPP( )
{
	return m_unBPP;
}

// *************************************************************************	

bool CVisPort::IsIndexed()
{
	switch ( m_pdtDataType )
	{
	case PDT_1BPP:
	case PDT_2BPP_GRAY:
	case PDT_4BPP_GRAY:
	case PDT_8BPP_GRAY:
	case PDT_24BPP_RGB:
	case PDT_32BPP_RGB:
	case PDT_24BPP_HSI:
	case PDT_32BPP_HSI:
	case PDT_DATA:
	default:
		return false;

	case PDT_2BPP_INDEX:
	case PDT_4BPP_INDEX:
	case PDT_8BPP_INDEX:
		return true;
	}
}

// *************************************************************************	
	
Uint32 CVisPort::GetBufferSize()
{
	return m_nBufferSize;
}

// *************************************************************************	
// C++: make this virtual and handle directly in sub classes.	
bool CVisPort::Prepare()
{
	// Handle both port types separately
	if ( GetType() == PT_INPUT )
		return ((CVisInputPort*)this)->Prepare();
		
	else if ( GetType() == PT_OUTPUT )
		return ((CVisOutputPort*)this)->Prepare();
		
	return false;
}
	
// *************************************************************************	

bool CVisPort::Connect( CVisPort * port )
{
	// Check for correct data type
	if ( port->GetDataType() != GetDataType() )
		LogMsg("Should not connect to %s, data types don't match", port->GetName() );

	// Handle both port types separately
	if ( GetType() == PT_INPUT )
	{
		// inputs can only be connected to outputs
		if ( port->GetType() != PT_INPUT )
		{
			((CVisOutputPort*)port)->Connect( (CVisInputPort*)this );
			m_pConnectedPort = port;
			return true;
		}
	}
	
	else if ( GetType() == PT_OUTPUT )
	{
		// inputs can only be connected to outputs
		if ( port->GetType() != PT_OUTPUT )
		{
			((CVisInputPort*)port)->Connect( (CVisOutputPort*)this );
			m_pConnectedPort = port;
			return true;
		}
	}
	
	LogMsg("Could not connect to %s, can only connect input to output ports", port->GetName() );
	
	return false;
}

// *************************************************************************

void CVisPort::Disconnect( )
{
	// Handle both port types separately
	// TODO::: use VIRTUAL instead when C++ is aproved.
	if ( GetType() == PT_INPUT )
	{
		((CVisInputPort*)this)->Disconnect( );
		m_pConnectedPort = NULL;
	}

	else if ( GetType() == PT_OUTPUT )
		LogMsg( "Disconnection of output ports not possible!");	
}

// *************************************************************************

CVisPort::PortType CVisPort::GetType()
{
	return m_ptType;
}

// *************************************************************************

CVisPort::PortDataType CVisPort::GetDataType()
{
	return m_pdtDataType;
}

// *************************************************************************

Uint32 CVisPort::GetBppFromType( PortDataType dataType )
{
	Uint32 bpp = 0;

	switch ( m_pdtDataType )
	{
	case PDT_1BPP:
		bpp = 1;
		break;
		
	case PDT_2BPP_GRAY:
		bpp = 2;
		break;
		
	case PDT_4BPP_GRAY:
		bpp = 4;
		break;

	case PDT_8BPP_GRAY:
		bpp = 8;
		break;
			
	case PDT_2BPP_INDEX:
		bpp = 2;
		break;

	case PDT_4BPP_INDEX:
		bpp = 4;
		break;

	case PDT_8BPP_INDEX:
		bpp = 8;
		break;
			
	case PDT_24BPP_RGB:
		bpp = 24;
		break;

	case PDT_32BPP_RGB:
		bpp = 32;
		break;
			
	case PDT_24BPP_HSI:
		bpp = 24;
		break;

	case PDT_32BPP_HSI:
		bpp = 32;
		break;
			
	case PDT_DATA:
		bpp = 1;
		break;

	default:
		bpp = 0;
		break;
	}

	return bpp;
}

// *************************************************************************

Ptr CVisPort::GetBuffer()
{
	return m_pBuffer;
}

// *************************************************************************

void CVisPort::SetBuffer( Ptr pBuffer )
{
	m_pBuffer = pBuffer;
}
	
// *************************************************************************

bool CVisPort::GetConnectedPort( CVisPort * & port)
{
	port =  m_pConnectedPort;

	if ( port != NULL )
		return true;
	else
		return false;
}
	
// *************************************************************************

void CVisPort::SetConnectedPort( CVisPort * port )
{
	m_pConnectedPort = port;
}
		
// *************************************************************************

void CVisPort::CacheInvalidate()
{
	if ( (m_pBuffer == NULL) || (m_nBufferSize == 0) )
		return;
		
	// TODO: maybe check if the buffer is really in SDRAM....
	
	InvalidateCache( m_pBuffer, m_nBufferSize );
}
	
// *************************************************************************

void CVisPort::CacheWriteback()
{	
	if ( (m_pBuffer == NULL) || (m_nBufferSize == 0) )
		return;
		
	// TODO: maybe check if the buffer is really in SDRAM....
	
	WritebackCache( m_pBuffer, m_nBufferSize );
}

// *************************************************************************

