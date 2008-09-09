#include "stdafx.h"
#include "Property.h"


CProperty::CProperty(CString IDArg, int valueArg,  CDSPComm* commArg)
{
	ID = IDArg;
	value = valueArg;
	comm =  commArg;
}

CProperty::CProperty()

{
	ID = "not initialized";
	value = 0;
	comm = NULL;
}

CProperty::~CProperty()
{

}

bool CProperty::Read( double & dValue )
{
	double d = dValue;

	if ( PerformRead( d ) )
	{
		//TRACE("in Read: PerformRead(d) with d: %f.\n", d);
		dValue = d;

		return true;
	}
	else
	{
		return false;
	}
	
	// Error handling
}

bool CProperty::Write( const double dValue )
{
	double d = dValue;

	if ( PerformWrite( d ) )
	{
		//TRACE("in Write: PerformWrite(d) with d: %f.\n", d);

		return true;
	}
	else
	{
		return false;
	}
}

void CProperty::update(const double dValue)
{
	value = dValue;
}
