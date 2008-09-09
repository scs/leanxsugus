// VisionProperty.h: interface for the CVisionProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISIONPROPERTY_H__0D55799C_0BC8_470C_81C2_427D4BB812C7__INCLUDED_)
#define AFX_VISIONPROPERTY_H__0D55799C_0BC8_470C_81C2_427D4BB812C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Property.h"

class CVisionProperty : public CProperty  
{
public:
protected:
	bool PerformRead( double & dValue );
	bool PerformWrite( const double dValue );


public:
	CVisionProperty(CString IDArg, int valueArg, CDSPComm* commArg);
	CVisionProperty();
	virtual ~CVisionProperty();

};

#endif // !defined(AFX_VISIONPROPERTY_H__0D55799C_0BC8_470C_81C2_427D4BB812C7__INCLUDED_)
