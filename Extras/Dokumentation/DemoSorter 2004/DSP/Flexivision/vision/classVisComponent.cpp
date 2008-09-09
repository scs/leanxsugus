
#include "classVisComponent.h"
#include "classVisInputPort.h"

#include <stdio.h>


CVisComponent::CVisComponent( const char * strName, const char * strType )
	: CVisObject( strName, CVisObject::CT_COMPONENT )
{
	Int i;
	
	for (i=0; i<COMP_MAX_PROPERTIES; i++)
		m_aryProperties[i] = NULL;
		
	for (i=0; i<COMP_MAX_PORTS; i++)		
		m_aryPorts[i] = NULL;
		
	SetType( strType );
	
	m_pMainInputPort = NULL;	
	
	m_unResultWidth = 0;
	m_unResultHeight = 0;
	// m_unResultBPP = unResultImageBPP;

	m_eOperationMode = OP_IDLE;
}
 
// *************************************************************************

bool CVisComponent::AddProperty( CVisProperty * pProperty)
{
	return AddObjectToList( (CVisObject**)m_aryProperties, COMP_MAX_PROPERTIES, pProperty );
}
 
// *************************************************************************

bool CVisComponent::AddPort( CVisPort * pPort )
{
	return AddObjectToList( (CVisObject**)m_aryPorts, COMP_MAX_PORTS, pPort );
}

// *************************************************************************

void CVisComponent::SetType( const char * strType )
{
	//StringCopy( strType, m_strType, COMP_TYPE_MAX_CHARS );
	m_strType = strType;
}

// *************************************************************************

const char * CVisComponent::GetType( )
{
	return m_strType;
}
 
// *************************************************************************

bool CVisComponent::GetProperty( const char * strName, CVisProperty ** ppProperty )
{
	return GetObjectFromList( (CVisObject**)m_aryProperties, COMP_MAX_PROPERTIES, (CVisObject**)ppProperty, strName );	
}
 
// *************************************************************************

bool CVisComponent::GetPort( const char * strName, CVisPort ** ppPort )
{
	return GetObjectFromList( (CVisObject**)m_aryPorts, COMP_MAX_PORTS, (CVisObject**)ppPort, strName );
}
 
// *************************************************************************

void CVisComponent::SetMainInputPort( CVisInputPort * pPort )
{
	m_pMainInputPort = pPort;
}

// *************************************************************************

void CVisComponent::Prepare()
{

}

// *************************************************************************

bool CVisComponent::GetMainInputPort( CVisInputPort ** pPort )
{
	// If the main input port has been set, return it.
	if (m_pMainInputPort != NULL)
	{
		*pPort = m_pMainInputPort;
		return true;
	}
		
	// If not, return the first input port		
	CVisInputPort * 	port;
	Int32			id = -1;
	
	while (	GetNextObjectFromList( (CVisObject **)m_aryPorts, COMP_MAX_PORTS, (CVisObject **)&port, id ))
	{
		if (port->GetType() == CVisPort::PT_INPUT )
		{
			*pPort = port;
			return true;
		}
	}
	return false;
}

// *************************************************************************

bool CVisComponent::GetResultImageSize( Uint32 & unWidth, Uint32 & unHeight)
{
	// If the component already knows the size of the output image, return it here.
	if (m_unResultWidth != 0)
	{
		unWidth = m_unResultWidth;
		unHeight = m_unResultHeight;
		
		return true;
	}
	
	// if not, ask the main input port
	CVisInputPort * port;
	
	if ( ! GetMainInputPort( &port ) )
		return false;
		
	port->GetImageSize( m_unResultWidth, m_unResultHeight );

	unWidth = m_unResultWidth;
	unHeight = m_unResultHeight;

	return true;
}

// *************************************************************************

bool CVisComponent::GetResultImageBPP( Uint32 & unBPP )
{
	unBPP = m_unResultBPP;
	return true;
}

// *************************************************************************

bool CVisComponent::Connect( const Char * strLocalPort, CVisComponent * pOtherComp, const Char * strOtherPort )
{
	CVisPort * localPort;
	CVisPort * otherPort;

	if ( pOtherComp == NULL )
	{
		LogMsg("Connecting: Invalid other component.");
		return false;
	}	

	if ( ! GetPort( strLocalPort, &localPort ) )
	{
		LogMsg("Connecting %s.%s and %s.%s: local port not found!", pOtherComp->GetName(), strOtherPort,
											this->GetName(), strLocalPort );		
	
		return false;
	}
		
	if ( ! pOtherComp->GetPort( strOtherPort, &otherPort ) )
	{
		LogMsg("Connecting %s.%s and %s.%s: other port not found!", pOtherComp->GetName(), strOtherPort,
											this->GetName(), strLocalPort );	
		return false;
	}

	return ( localPort->Connect( otherPort ) );	
}

// *************************************************************************

bool CVisComponent::Disconnect( const Char * strLocalPort )
{
	CVisPort * port;
	
	if ( ! GetPort( strLocalPort, &port ) )
		return false;

	port->Disconnect();

	return true;
}

// *************************************************************************

void CVisComponent::SetOperationMode( OperationMode mode )
{
	m_eOperationMode = mode;
}

// *************************************************************************

bool CVisComponent::HavePropertiesChanged()
{
	if ( m_bPropertiesChanged )
	{		
		return true;
	}
	else
		return false;
}

// *************************************************************************

void CVisComponent::ResetPropertiesChanged()
{
	// reset flag
	m_bPropertiesChanged = false;
}

// *************************************************************************

void CVisComponent::PropertyChanges()
{
	m_bPropertiesChanged = true;
}

// *************************************************************************


