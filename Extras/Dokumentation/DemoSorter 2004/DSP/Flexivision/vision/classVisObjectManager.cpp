
#include "classVisObjectManager.h"
#include "classVisPort.h"

#include <stdio.h>

bool CVisObjectManager::bConstructed = false;
CVisObjectManager CVisObjectManager::TheManager;

CVisObjectManager::CVisObjectManager()
	: CVisObject( "Object Manager", CVisObject::CT_OBJECT_MANAGER )
{
	ClearList( m_aryObjects, OBJ_MAX_OBJECTS );
	bConstructed = true;
	
	m_nNumObjects = 0;
}

// *************************************************************************

CVisObjectManager::~CVisObjectManager()
{
	// DEBUG:
	int i = sizeof(CVisObject);
	i = sizeof(CVisComponent);

}

// *************************************************************************

					
CVisObjectManager * CVisObjectManager::Instance()
{
	return &TheManager;
}

// *************************************************************************

bool CVisObjectManager::AddObject( CVisObject * pObject )
{
	bool res;

	res = AddObjectToList( (CVisObject**)m_aryObjects, OBJ_MAX_OBJECTS, pObject );

	if (!res)
		LogMsg( "Too many Objects!");
		
	m_nNumObjects++;
	
	return res;
}

// *************************************************************************

bool CVisObjectManager::GetNextObject( CVisObject ** ppObject, ClassType type, Int32 & id )
{
	CVisObject * obj;
	
	while ( GetNextObjectFromList((CVisObject**)m_aryObjects, OBJ_MAX_OBJECTS, &obj, id ) )
	{		
		if (obj->GetClassType() == type )
		{
			*ppObject = obj;
			return true;
		}
	}
	
	return false;
}

// *************************************************************************

bool CVisObjectManager::GetObject( const Char * strName, CVisObject ** ppObject, ClassType type)
{
	Int32 id=-1;
	CVisObject * obj;
	
	while ( GetNextObjectFromList((CVisObject**)m_aryObjects, OBJ_MAX_OBJECTS, &obj, id ) )
	{		
		if (obj->GetClassType() == type )
		{
			if ( obj->HasName(strName) )
			{
				*ppObject = obj;
				return true;
			}
		}
	}
	
	return false;
}

// *************************************************************************

bool CVisObjectManager::PreparePorts()
{
	Int32 id=-1;
	CVisObject * obj;
	bool res = true;
	bool b;
	
	while ( GetNextObjectFromList((CVisObject**)m_aryObjects, OBJ_MAX_OBJECTS, &obj, id ) )
	{
		if (obj->GetClassType() == CVisObject::CT_PORT )
		{
			b = ((CVisPort*)obj)->Prepare();

			if ( !b )
			{
				LogMsg("Error preparing obj: %s.%s", ((CVisPort*)obj)->GetComponent()->GetName(), obj->GetName() );
				res = FALSE;
			}
		}
	}
	
	return res;
}

// *************************************************************************

bool CVisObjectManager::PrepareComponents()
{
	Int32 id=-1;
	CVisObject * obj;
	bool res = true;
//	bool b;

	while ( GetNextObject( &obj, CVisObject::CT_COMPONENT, id ) )
	{
		Uint32 w,h,bpp;

		// Get the image size and BPP so that the component is properly initialized, even if
		// it has no output ports (normally, output ports trigger initialization of the component
		// in the preparePorts() function).
		((CVisComponent*)obj)->GetResultImageSize( w, h );
		((CVisComponent*)obj)->GetResultImageBPP( bpp );

		((CVisComponent*)obj)->Prepare();
	}
	
	return res;
}
// *************************************************************************

void CVisObjectManager::SetComponentsOperationMode( CVisComponent::OperationMode mode )
{
	Int32 id=-1;
	CVisObject * obj;
	
	while ( GetNextObjectFromList((CVisObject**)m_aryObjects, OBJ_MAX_OBJECTS, &obj, id ) )
	{
		if (obj->GetClassType() == CVisObject::CT_COMPONENT)
			((CVisComponent*)obj)->SetOperationMode( mode );
	}
}

// *************************************************************************

CVisVision *  CVisObjectManager::GetMainVisionObject()
{
	CVisObject * obj;
	Int id = -1;
	
	if ( GetNextObject( &obj, CT_VISION, id ) )
	{
		return (CVisVision*)obj;
	}
	
	return NULL;
}

// *************************************************************************
