
#include "classVisVision.h"
#include "classVisObjectManager.h"
#include "classVisPort.h"


// *************************************************************************

CVisVision::CVisVision( const Char * strName )
	: CVisObject( strName, CT_VISION )
{
	Int i;
	for (i=0; i<VIS_MAX_CHANNELS; i++)
		m_aryChannels[i].pPort = NULL;
		
	for (i=0; i<VIS_MAX_EVENTS; i++)
		m_aryEvents[i] = FALSE;
		
	m_unCurrentImageTime = 0;
}

// *************************************************************************

void CVisVision::FeedImage( Ptr pBuffer )
{
}

// *************************************************************************

void CVisVision::SetCurrentImageTime( Uint32 unTime )
{
	m_unCurrentImageTime = unTime;
}

// *************************************************************************

bool CVisVision::EnableViewPort( Uint32 unChannel, const Char * strComponentName, const Char * strPortName)
{
	CVisObject *	obj;
	CVisComponent * comp;
	CVisPort *		port;
	
	if (unChannel >= VIS_MAX_CHANNELS)
		return false;
	
	if ( ! CVisObjectManager::Instance()->GetObject( strComponentName, &obj, CT_COMPONENT ) )
		return false;
	comp = (CVisComponent *)obj;
		
	if ( ! comp->GetPort( strPortName, &port ) )
		return false;
		
	if ( ! port->GetType() == CVisPort::PT_OUTPUT )
		return false;
		
	m_aryChannels[unChannel].pPort = (CVisOutputPort*)port;
	m_aryChannels[unChannel].pBuffer = NULL;
	
	return true;
}
	
// *************************************************************************

void CVisVision::DisableViewPort( Uint32 unChannel )
{
	if (unChannel >= VIS_MAX_CHANNELS)
		return;
		
	m_aryChannels[unChannel].pPort = NULL;
}
	
// *************************************************************************

void CVisVision::FeedViewPortBuffer( Uint32 unChannel, Ptr pBuffer )
{
	if (unChannel >= VIS_MAX_CHANNELS)
		return;
		
	m_aryChannels[unChannel].pBuffer = pBuffer;
}


// *************************************************************************

bool CVisVision::GetViewPortImageInfo( Uint32 unChannel, Uint32 & width, Uint32 & height, Uint32 & bpp, bool & bData, bool & bIndexed)
{
	CVisPort::PortDataType type;
		
	if (unChannel >= VIS_MAX_CHANNELS)
		return false;
		
	if (m_aryChannels[unChannel].pPort == NULL)
		return false;
		
	// Extract type from port
	type = m_aryChannels[unChannel].pPort->GetDataType();
			
	// Distinguish between data buffers and Image buffers.
	if ( type == CVisPort::PDT_DATA )
	{
		// Make the port look like an image with a width of the size of the buffer
		width = m_aryChannels[unChannel].pPort->GetBufferSize();
		height = 1;
		bpp = 8;
		bData = TRUE;
		bIndexed = FALSE;
	}
	else
	{
		// Retrieve image info from port.
		if ( ! m_aryChannels[unChannel].pPort->GetImageSize( width, height ) )
			return false;
		
		bpp = m_aryChannels[unChannel].pPort->GetImageBPP( );

		bData = FALSE;
		bIndexed = m_aryChannels[unChannel].pPort->IsIndexed();
	}
	
	return true;
}

// *************************************************************************
	
bool CVisVision::Prepare()
{
	Bool b = TRUE;

	if ( ! CVisObjectManager::Instance()->PreparePorts() )
		b = FALSE;

	if ( ! CVisObjectManager::Instance()->PrepareComponents() )
		b = FALSE;

	return (b == TRUE);
}

// *************************************************************************
/*	
void CVisVision::ChangeMode( VisionMode newMode ) 
{
}
*/
// *************************************************************************
	
void CVisVision::SetComponentsOperationMode( CVisComponent::OperationMode mode )
{
	CVisObjectManager::Instance()->SetComponentsOperationMode( mode );
}

// *************************************************************************
	
void CVisVision::DoProcessing()
{
}
	
// *************************************************************************

void CVisVision::CopyViewPorts()
{
	for (Int i=0; i<VIS_MAX_CHANNELS; i++)
	{
		// If the channel is opened, copy it.
		if (m_aryChannels[i].pPort != NULL)
		{
			Uint32 copyId;
			
			m_aryChannels[i].pPort->CacheWriteback();
			
			copyId = StartCopy( m_aryChannels[i].pBuffer, m_aryChannels[i].pPort->GetBuffer(), m_aryChannels[i].pPort->GetBufferSize() );
			WaitCopy( copyId );
		}
	}
}
	
// *************************************************************************

Bool CVisVision::EnumOutputPorts( Int & nIndex, const Char * & strComponent, const Char * & strPort )
{
	CVisObject * obj;
	CVisPort * port;
	
	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PORT, nIndex ) )
	{
		port = (CVisPort *)obj;
		
		if ( port->GetType() == CVisPort::PT_OUTPUT )
		{
			// found one.
			strComponent = port->GetComponent()->GetName();
			strPort = port->GetName();
			return TRUE;
		}		
	}
	
	return FALSE;
}
	
// *************************************************************************

Bool CVisVision::SetProperty( const Char * strComponent, const Char * strProperty, float fValue )
{
	return AccessProperty( TRUE, strComponent, strProperty, fValue );
}


// *************************************************************************

Bool CVisVision::GetProperty( const Char * strComponent, const Char * strProperty, float & fValue )
{
	return AccessProperty( FALSE, strComponent, strProperty, fValue );
}

// *************************************************************************

Bool CVisVision::AccessProperty( const Bool bSet, const Char * strComponent, const Char * strProperty, float & fValue )
{
	Int 			nIndex = -1;
	CVisObject * 	obj;
	CVisProperty *	prop;
	
	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PROPERTY, nIndex ) )
	{
		prop = (CVisProperty*)obj;
		
		if ( prop->HasName( strProperty ) && prop->GetComponent()->HasName( strComponent ) )
		{
			// found it.
			if ( bSet )
				prop->SetFloatValue( fValue );
			else
				prop->GetFloatValue( fValue );
			
			return TRUE;
		}				
	}
	
	return FALSE;
}

// *************************************************************************

Bool CVisVision::EnumProperties( Int & nIndex, const Char * & strComponent, const Char * & strProperty, float & fValue )
{
	CVisObject * obj;
	CVisProperty * prop;
	
	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PROPERTY, nIndex ) )
	{
		prop = (CVisProperty *)obj;
		
		strComponent = prop->GetComponent()->GetName();
		strProperty = prop->GetName();
		prop->GetFloatValue(fValue);
		
		return TRUE;		
	}
	
	return FALSE;
}
	
// *************************************************************************

Bool CVisVision::EventOccured( Int nEventId )
{
	Bool b;
	
	if ( (nEventId <0 ) || (nEventId >= VIS_MAX_EVENTS) )
		return FALSE;
	
	// Clear and return the event
	b = m_aryEvents[nEventId];
	m_aryEvents[nEventId] = FALSE;
		
	return b;
}
	
// *************************************************************************

void CVisVision::SetEvent( Int nEventId )
{
	if ( (nEventId <0 ) || (nEventId >= VIS_MAX_EVENTS) )
		return;
		
	m_aryEvents[nEventId] = TRUE;
}
	
// *************************************************************************

void CVisVision::ClearEvents( )
{
	for ( int i=0; i<VIS_MAX_EVENTS; i++ )
		m_aryEvents[i] = FALSE;
}
