#include "stdafx.h"
#include "DSPComm.h"
#include "CDataFile.h"


CDSPComm::CDSPComm()
{
	
	connectedToEtrax = getPreferences(); //if true: no error, but has not to be connected by startup anyway
		
}

CDSPComm::~CDSPComm()
{
	//aufräumen... socket zerstören...
	if (connectedToEtrax)
		closeConnection();
}

bool CDSPComm::isConnectedToEtrax()
{
	return connectedToEtrax;
}

CString CDSPComm::getIpAddr()
{
	CString tmp;

	tmp.Format("%s : %d", inet_ntoa(addr.sin_addr), addr.sin_port);

	return tmp;

}


bool CDSPComm::process(CString strCommand)
{
	CString replyCString;
	int nTimeout_ms = 100;

	int countFailed = 0;

	// Evtl: lock communicator
	crSec.Lock();

	// Send 
	strCommand = "dsp<< " + strCommand + "\n";
	bool shit = this->SendLine( strCommand , 100 );
	//if (comm->SendLine( strCommand , 100 ))
		//TRACE("Send successfull!\n");
	//else
		//TRACE("Send not successfull!\n");

	
	// Receive - we know that we will receive something!!!
	do //--> momentan wird empfangen, bis 'done' kommt - auch wenn timeout erreicht! -> maximal 10 failed lines...
	{
		if (this->RecvLine(replyCString, 2000)) //momentan noch 2 sek timeout!!!
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

bool CDSPComm::setNewConnection(CString ipAddr, int port)
{
	if (isConnectedToEtrax())
		closeConnection(); //zuerst aufräumen, dann von vorne beginnen --> darf man das so??

	unsigned long ipReturn = inet_addr(ipAddr);

	if (ipReturn == INADDR_NONE)
	{
		TRACE("IP address could not be created\n");
		connectedToEtrax = false;
		return false;
	}
	
	//init der Serveradresse
	addr.sin_addr.S_un.S_addr = ipReturn;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	
	if (startWinsock() == SOCKET_ERROR)
	{
		TRACE( "Winsock startup failed\n" );
		connectedToEtrax = false;
		return false;
	}

	//socket erstellen
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		TRACE( "Socket creation failed\n" );
		connectedToEtrax = false;
		return false;
	}

	if (openConnection() == -1)
	{
		TRACE("open connection failed\n");
		connectedToEtrax = false;
		return false;	
	}

	connectedToEtrax = true;

	TRACE("connectedToEtrax: %d\n", connectedToEtrax);

	if(this->process("debug off"))
		return true;

	return false;//true;
}

int CDSPComm::startWinsock()
{
	int rc;
	WSADATA wsaData;
	rc=WSAStartup(MAKEWORD(2,0),&wsaData);
	if(rc==SOCKET_ERROR){
		TRACE("Error while starting WinSock\n");
		return rc;
	}
	return 0;
}

int CDSPComm::openConnection()
{
	//verbinden
	int connectState = connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	if (connectState == SOCKET_ERROR){
		TRACE("connecting failed\n");
		return -1;
	}
	return 0;
}

void CDSPComm::closeConnection()
{
	closesocket(sock);
	WSACleanup();
	connectedToEtrax = false;
}

bool CDSPComm::SendLine( const CString & cstrLine, int nTimeout_ms )
{
		//******* senden ********

	const char* buf = LPCTSTR(cstrLine);

	int sendState = send(sock, buf, strlen(buf), 0);

	if (sendState == SOCKET_ERROR)
	{
		TRACE("Error while sending\n");
		return false;
	}
	else
	{
		//TRACE("%s gesendet\n", buf);
		return true;
	}

}



bool CDSPComm::RecvLine( CString & cstrLine, int nTimeout_ms )
{
	//****** empfangen ***********

	struct timeval t = { nTimeout_ms / 1000L, 1000 * (nTimeout_ms % 1000L) };
	int length = 100;
	char* recvBuf = new char[length]; //müsste reichen für eine Zeile
	int received = 0; //empfangene Bytes
	CString str;
	int strTestNewline = -1;
	unsigned int n;


	while (received != length)
	{
		//init fd_set for select
		fd_set rd_set;
		FD_ZERO(&rd_set);
		FD_SET(sock, &rd_set);

		//select with timeout
		n = select(sock + 1, &rd_set, 0, 0, &t);
		if (n == SOCKET_ERROR)
		{
			TRACE("Socket error while select()\n");
			delete recvBuf;
			return false;
		}
		if (n == 0)
		{
			TRACE("Timeout reached\n");
			delete recvBuf;
			return false;
		}
		
		int result = recv(sock, recvBuf + received, 1, 0); //third argument == 0: non-blocking

			
		//TRACE("Anzahl Bytes received: %d\n", result);
		//TRACE("Länge des Buffers: %d\n", strlen(recvBuf)); --> strlen searches 0 character!!!

		if (result == SOCKET_ERROR)
		{
			TRACE("Socket error while recv()\n");
			delete recvBuf;
			return false;
		}

		// temporarily terminate string!!!
		recvBuf[ received + result ] = '\0';

		str.Format(recvBuf);
		
		strTestNewline = str.Find('\n', 0);
		

		if (strTestNewline != -1)
		{
			//TRACE("newline character found at position %d.\n", strTestNewline);
			cstrLine = str;
			delete recvBuf;
			return true;
		}

		received += result;

	}
	delete recvBuf;
	return false;
}

bool CDSPComm::getPreferences()
{

	/// Section One ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	// In this section, we are going to create a CDataFile object by opening
	// an allready existing file.  If the file is sucessfully opened, the
	// CDataFile will be populated with it's key/value pair contents.
	////////////////////////////////////////////////////////////////////////////

	// Create the object, passing in the file name. The file will be loaded 
	// (if it can be) and the objects keys set to the contents of the file. 
	// The file is then closed.
	CDataFile ExistingDF("preferences.ini");

	Report(E_INFO, "[CDSPComm::getPreferences] The file <preferences.ini> contains %d sections, & %d keys.",
				   ExistingDF.SectionCount(), ExistingDF.KeyCount());

	// Querry the CDataFile for the values of some keys ////////////////////////
	////////////////////////////////////////////////////////////////////////////
	// Keys that are not found in the object will have the following values;
	//
	//   String based keys: t_Str("")
	//   Int based keys   : INT_MIN
	//   Float based keys : FLT_MIN

	////////////////////////////////////////////////////////////////////////////
	t_Str ip_str = t_Str("");
	int connect	= 0;
	int	port = 0;

	timerenable = 0;
	showcounters = 0;


	timerenable = ExistingDF.GetInt("timerenable", "init");
	if ( timerenable == INT_MIN )
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'timerenable' was not found.");
		//return false;
	}
	else
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'timerenable' contains the value '%d'",
		               timerenable);
		//return true;
	}

	showcounters = ExistingDF.GetInt("showcounters", "init");
	if ( showcounters == INT_MIN )
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'showcounters' was not found.");
		//return false;
	}
	else
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'showcounters' contains the value '%d'",
		               showcounters);
		//return true;
	}

	connect = ExistingDF.GetInt("connect", "init");
	if ( connect == INT_MIN )
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'connect' was not found.");
		return false;
	}
	else
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'connect' contains the value '%d'",
		               connect);
		if (connect == 1)
		{
			ip_str = ExistingDF.GetString("ip", "address");
			if ( ip_str.size() == 0 ){
				Report(E_INFO, "[CDSPComm::getPreferences] Key 'ip' was not found.");
				return false;
			}
			else{
				Report(E_INFO, "[CDSPComm::getPreferences] Key 'ip' contains the value '%s'",
								ip_str.c_str());
			}
			
			port  = ExistingDF.GetInt("port", "address");
			if ( port == INT_MIN ){
				Report(E_INFO, "[CDSPComm::getPreferences] Key 'port' was not found.");
				return false;
			}
			else{
				Report(E_INFO, "[CDSPComm::getPreferences] Key 'port' contains the value '%d'", port);
			}
			CString tmpAddr;
			tmpAddr.Format("%s", ip_str.c_str());
			TRACE("DSPComm: " + tmpAddr + " %d", port); 
			if (setNewConnection(tmpAddr, port))
			{
				TRACE("\nsetNewConnection successfull");
				return true;
			}
			else
			{
				TRACE("\nsetNeweConnection not successfull\n");
			}
		}
		else
		{
			return false;
		}
	}

	return false;


}
