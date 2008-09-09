// DspCommand.h: interface for the CDspCommand class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Command.h"
#include <vector>
#include <afxmt.h>

using namespace std;


#if !defined(AFX_DSPCOMMAND_H__DE1256E9_CE4B_42BF_8653_6D2F6870949D__INCLUDED_)
#define AFX_DSPCOMMAND_H__DE1256E9_CE4B_42BF_8653_6D2F6870949D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDSPCommand : public CCommand  
{
public:
						CDSPCommand();
						CDSPCommand(CDSPComm*);
						~CDSPCommand();

	bool				process(CString strCommand);
	CString				GetResponse( int nLine );
	int					GetNumRecvLines();

protected:
	std::vector<CString> recvdLines;
	CCriticalSection crSec;



};

#endif // !defined(AFX_DSPCOMMAND_H__DE1256E9_CE4B_42BF_8653_6D2F6870949D__INCLUDED_)
