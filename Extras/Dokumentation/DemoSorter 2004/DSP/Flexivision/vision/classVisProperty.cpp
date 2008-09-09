

#include "classVisProperty.h"

CVisProperty::CVisProperty(const Char * strName )
	: CVisObject( strName, CVisObject::CT_PROPERTY )
{

}

// *************************************************************************

void CVisProperty::Init( CVisComponent * pComp,  CVisProperty::PropertyType type, Ptr pVariable, Int nFPFactorBits  )
{
	m_pComponent = pComp;
	m_ptType = type;
	m_pVariable = pVariable;
	
	m_nFactorBits = nFPFactorBits;
	
	m_pComponent->AddProperty(this);
	
	m_bHasChanged = FALSE;
}

// *************************************************************************

void  CVisProperty::Init( CVisComponent * pComp, CVisFixpoint * fp )
{
	m_pComponent = pComp;
	m_ptType = PT_FIXEDPOINTOBJ;
	m_pVariable = (Ptr)fp;
	
	m_nFactorBits = 0;
	
	m_pComponent->AddProperty(this);
	
	m_bHasChanged = FALSE;
}

// *************************************************************************

CVisComponent *	 CVisProperty::GetComponent()
{
	return m_pComponent;
}

// *************************************************************************

bool CVisProperty::SetIntegerValue( Int32 v)
{
	if (m_ptType == PT_INTEGER )
		*((Int32*)m_pVariable) = v;

	else if (m_ptType == PT_FLOAT )
		*((float*)m_pVariable) = (float)v;

	else if (m_ptType == PT_FIXEDPOINT )
		*((Int32*)m_pVariable) = int2fp(v, m_nFactorBits);

	else if (m_ptType == PT_FIXEDPOINTOBJ )
		*(CVisFixpoint*)(m_pVariable) = v;
		
	// Mark changed flag in the property and in the component.
	m_bHasChanged = TRUE;
	m_pComponent->PropertyChanges();
		
	return true;
}

// *************************************************************************

bool CVisProperty::GetIntegerValue( Int32 & v )
{
	if (m_ptType == PT_INTEGER )
		v = *((Int32*)m_pVariable);

	else if (m_ptType == PT_FLOAT )
		v = (Int32)(*((float*)m_pVariable));

	else if (m_ptType == PT_FIXEDPOINT )
		v = fp2int(*((Int32*)m_pVariable), m_nFactorBits);

	else if (m_ptType == PT_FIXEDPOINTOBJ )
		v = (int)(*(CVisFixpoint*)m_pVariable);
		
	return true;
}
	
// *************************************************************************

bool CVisProperty::SetFloatValue( float v)
{
	if (m_ptType == PT_INTEGER )
		*((Int32*)m_pVariable) = (Int)v;

	else if (m_ptType == PT_FLOAT )
		*((float*)m_pVariable) = v;	

	else if (m_ptType == PT_FIXEDPOINT )		
		*((Int32*)m_pVariable) = float2fp(v, m_nFactorBits);

	else if (m_ptType == PT_FIXEDPOINTOBJ )
		*(CVisFixpoint*)(m_pVariable) = v;		
		
	// Mark changed flag in the property and in the component.
	m_bHasChanged = TRUE;
	m_pComponent->PropertyChanges();
	
	return true;
}

// *************************************************************************

bool CVisProperty::GetFloatValue( float & v )
{
	if (m_ptType == PT_INTEGER )
		v = (float)(*((Int32*)m_pVariable));

	else if (m_ptType == PT_FLOAT )
		v = *((float*)m_pVariable);

	else if (m_ptType == PT_FIXEDPOINT )
		v = fp2float(*((Int32*)m_pVariable), m_nFactorBits);
	
	else if (m_ptType == PT_FIXEDPOINTOBJ )
		v = (float)(*(CVisFixpoint*)m_pVariable);
			
	return true;
}

// *************************************************************************

Bool CVisProperty::HasChanged()
{
	Bool b = m_bHasChanged;
	
	m_bHasChanged = FALSE;
	
	return b;
}

// *************************************************************************


