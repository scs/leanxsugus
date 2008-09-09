// DspCommand.cpp: implementation of the CDspCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "tcpTest3.h"
#include "DspCommand.h"
//#include <list>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDSPCommand::CDSPCommand(CDSPComm* commArg)
{
	comm = commArg;

}

CDSPCommand::CDSPCommand(){}

CDSPCommand::~CDSPCommand()
{
	//recvdLines.~vector<CString>(); //scheint es nicht zu brauchen, da keine Memory leaks mehr...
}


bool CDSPCommand::process(CString strCommand)
{
	CString replyCString;
	int nTimeout_ms = 100;

	int countFailed = 0;

	// Evtl: lock communicator
	crSec.Lock();

	// Send 
	strCommand = "dsp<< " + strCommand + "\n";
	bool shit = comm->SendLine( strCommand , 100 );
	//if (comm->SendLine( strCommand , 100 ))
		//TRACE("Send successfull!\n");
	//else
		//TRACE("Send not successfull!\n");

	
	// Receive - we know that we will receive something!!!
	do //--> momentan wird empfangen, bis 'done' kommt - auch wenn timeout erreicht! -> maximal 10 failed lines...
	{
		if (comm->RecvLine(replyCString, 60000)) //momentan noch 60 sek timeout!!!
		{
			//TRACE("empfangen erfolgreich\n");

			//entferne "dsp>> "
			if (replyCString.GetLength()<6)
			{
				//TRACE("Command line to short for processing! Continue with next line...\n");
				continue;
			}
			int n = replyCString.Delete(0, 6);

			//entferne newline character
			replyCString.TrimRight('\n');

   
			//TRACE("Folgende Zeile wurde prozessiert: %s.\n", replyCString);
				
			if ( replyCString != "Done" )
				recvdLines.insert(recvdLines.end(), replyCString);
		}
		else 
		{
			countFailed++;
			if (countFailed == 10)
				return false;
		}

	} while (replyCString != "Done");

	// Evtl: unlock communicator
	crSec.Unlock();					
	
	return true;
}

CString CDSPCommand::GetResponse( int nLine )
{
	return recvdLines.at(nLine);
}

int CDSPCommand::GetNumRecvLines()
{
	return recvdLines.size();
}

