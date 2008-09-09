#ifndef _DSPCOMM_H_
#define _DSPCOMM_H_

#include <windows.h>
#include <winsock2.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <afxmt.h>


class CDSPComm{

private:
	bool connectedToEtrax;
	bool getPreferences();

protected:

public:
	CDSPComm();
	~CDSPComm();
	bool setNewConnection(CString ipAddr, int port);

	int timerenable;
	int showcounters;

	int openConnection();
	void closeConnection();

	bool isConnectedToEtrax();
	CString getIpAddr();

	SOCKET sock;
	SOCKADDR_IN	addr;
	typedef unsigned int Timeout;

	bool SendLine( const CString & cstrLine, int nTimeout_ms );
	bool RecvLine( CString & cstrLine, int nTimeout_ms );

protected:
	int startWinsock();
	bool process(CString strCommand);
	CCriticalSection crSec;



};



#endif _DSPCOMM_H_