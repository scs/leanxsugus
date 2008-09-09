// ModeProperty.h: interface for the CModeProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEPROPERTY_H__0C161DEA_56B9_4CF3_8D9E_0AEDC3C6BC04__INCLUDED_)
#define AFX_MODEPROPERTY_H__0C161DEA_56B9_4CF3_8D9E_0AEDC3C6BC04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Property.h"

class CModeProperty : public CProperty  
{
public:
	CModeProperty(CString IDArg, int valueArg, CDSPComm* commArg);
	CModeProperty();
	virtual ~CModeProperty();

protected:
	bool PerformRead( double & dValue );
	bool PerformWrite( const double dValue );

};

#endif // !defined(AFX_MODEPROPERTY_H__0C161DEA_56B9_4CF3_8D9E_0AEDC3C6BC04__INCLUDED_)
