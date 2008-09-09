// Command.h: interface for the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "DSPComm.h"


#if !defined(AFX_COMMAND_H__063BCADE_9068_46D0_84E9_0BD5FE3A5A77__INCLUDED_)
#define AFX_COMMAND_H__063BCADE_9068_46D0_84E9_0BD5FE3A5A77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCommand  
{
public:
	CCommand();
	CCommand( CDSPComm* );

	virtual ~CCommand();

protected:
	CDSPComm* comm;


};

#endif // !defined(AFX_COMMAND_H__063BCADE_9068_46D0_84E9_0BD5FE3A5A77__INCLUDED_)
