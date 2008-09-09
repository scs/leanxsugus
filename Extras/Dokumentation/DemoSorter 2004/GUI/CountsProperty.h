// CountsProperty.h: interface for the CCountsProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COUNTSPROPERTY_H__F7D90A1C_F604_4375_A933_1E4636AA2B6D__INCLUDED_)
#define AFX_COUNTSPROPERTY_H__F7D90A1C_F604_4375_A933_1E4636AA2B6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSPComm.h"

class CCounts
{

protected:
	CDSPComm * comm;
	double * values;
public:
	CCounts();
	~CCounts();
	bool Reset();
	bool Read( double dValue [] );
	CCounts(CDSPComm* commArg);

};

#endif // !defined(AFX_COUNTSPROPERTY_H__F7D90A1C_F604_4375_A933_1E4636AA2B6D__INCLUDED_)
