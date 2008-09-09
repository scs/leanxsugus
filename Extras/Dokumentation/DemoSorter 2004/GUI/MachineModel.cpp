// MachineModel.cpp : implementation file
//

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "MachineModel.h"
#include "UserMessages.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMachineModel

IMPLEMENT_DYNCREATE(CMachineModel, CWinThread)

CMachineModel::CMachineModel()
{
	isFinished = false;

	m_DSPComm = new CDSPComm();

	counts = new CCounts(m_DSPComm);

	propertyList.insert(pair<CString, CProperty*>("Color0.Hue", pColor0Hue = new CVisionProperty("Color0.Hue", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color0.Count", pColor0Count = new CVisionProperty("Color0.Count", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color0.Eject", pColor0Eject = new CVisionProperty("Color0.Eject", 0, m_DSPComm)));

	propertyList.insert(pair<CString, CProperty*>("Color1.Hue", pColor1Hue = new CVisionProperty("Color1.Hue", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color1.Count", pColor1Count = new CVisionProperty("Color1.Count", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color1.Eject", pColor1Eject = new CVisionProperty("Color1.Eject", 0, m_DSPComm)));

	propertyList.insert(pair<CString, CProperty*>("Color2.Hue", pColor2Hue = new CVisionProperty("Color2.Hue", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color2.Count", pColor2Count = new CVisionProperty("Color2.Count", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color2.Eject", pColor2Eject = new CVisionProperty("Color2.Eject", 0, m_DSPComm)));

	propertyList.insert(pair<CString, CProperty*>("Color3.Hue", pColor3Hue = new CVisionProperty("Color3.Hue", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color3.Count", pColor3Count = new CVisionProperty("Color3.Count", 0, m_DSPComm)));
	propertyList.insert(pair<CString, CProperty*>("Color3.Eject", pColor3Eject = new CVisionProperty("Color3.Eject", 0, m_DSPComm)));

	//propertyList.insert(pair<CString, CProperty*>("Color4.Hue", pColor4Hue = new CVisionProperty("Color4.Hue",0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color4.Count", pColor4Count = new CVisionProperty("Color4.Count",0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color4.Eject", pColor4Eject = new CVisionProperty("Color4.Eject",0, m_DSPComm)));

	//propertyList.insert(pair<CString, CProperty*>("Color5.Hue", pColor5Hue = new CVisionProperty("Color5.Hue", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color5.Count", pColor5Count = new CVisionProperty("Color5.Count", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color5.Eject", pColor5Eject = new CVisionProperty("Color5.Eject", 0, m_DSPComm)));

	//propertyList.insert(pair<CString, CProperty*>("Color6.Hue", pColor6Hue = new CVisionProperty("Color6.Hue", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color6.Count", pColor6Count = new CVisionProperty("Color6.Count", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color6.Eject", pColor6Eject = new CVisionProperty("Color6.Eject", 0, m_DSPComm)));

	//propertyList.insert(pair<CString, CProperty*>("Color7.Hue", pColor7Hue = new CVisionProperty("Color7.Hue", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color7.Count", pColor7Count = new CVisionProperty("Color7.Count", 0, m_DSPComm)));
	//propertyList.insert(pair<CString, CProperty*>("Color7.Eject", pColor7Eject = new CVisionProperty("Color7.Eject", 0, m_DSPComm)));

	propertyList.insert(pair<CString, CProperty*>("mode", pMode = new CModeProperty("mode", 0, m_DSPComm)));

}

CMachineModel::~CMachineModel()
{
	delete counts;

	delete pMode;

	delete pColor0Count;
	delete pColor0Hue;
	delete pColor0Eject;

	delete pColor1Count;
	delete pColor1Hue;
	delete pColor1Eject;

	delete pColor2Count;
	delete pColor2Hue;
	delete pColor2Eject;

	delete pColor3Count;
	delete pColor3Hue;
	delete pColor3Eject;

	//delete pColor4Count;
	//delete pColor4Hue;
	//delete pColor4Eject;

	//delete pColor5Count;
	//delete pColor5Hue;
	//delete pColor5Eject;

	//delete pColor6Count;
	//delete pColor6Hue;
	//delete pColor6Eject;

	//delete pColor7Count;
	//delete pColor7Hue;
	//delete pColor7Eject;

	delete m_DSPComm;
	
	propertyList.clear();
}

BOOL CMachineModel::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CMachineModel::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CMachineModel, CWinThread)
	//{{AFX_MSG_MAP(CMachineModel)
		// NOTE - the ClassWizard will add and remove mapping macros here.
//		ON_MESSAGE(GET_COUNTS_MSG, onGetCounts)	//user defined message mapping to function GetMessage
		ON_MESSAGE(SET_MSG, onSetMessage)
		ON_MESSAGE(GET_MSG, onGetMessage)
		ON_MESSAGE(WM_QUIT, onQuit)
		ON_MESSAGE(RESET_MSG, onResetMessage)
		ON_MESSAGE(COUNTS_MSG, onCountsMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMachineModel message handlers

int CMachineModel::Run()
{	
	TRACE( "MachineModelThread started!\n" );

	// TODO: Add extra initialization here
	//TRACE("isConnectedToEtrax = %d.\n", m_DSPComm->isConnectedToEtrax());


	Message_Set_Reply* msgSetReply = new Message_Set_Reply;	//Message_Get ist Struct

	if ((m_DSPComm->isConnectedToEtrax())==false)
	{
		
		msgSetReply->ID = "Connect on startup";
		msgSetReply->Success = false;

		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)msgSetReply);
	}
	else
	{

		msgSetReply->ID = "Connect on startup";
		msgSetReply->Success = true;

		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)msgSetReply);
	}

	int res = CWinThread::Run();	//call Run function of base class CWinThread


	return res;
}

void CMachineModel::connectGuiModelThread( CWnd* guiThread ) 
{
	_DemoSorterGUIThread = guiThread;
}

CDSPComm* CMachineModel::submitDSPCommP()
{
	return m_DSPComm;
}


afx_msg void CMachineModel::onGetMessage(WPARAM wParam, LPARAM info)
{
	TRACE( "CMachineModel Thread: GET_MSG received\n" );

	Message_Get* msgGet = (Message_Get*) info;	//Message_Get ist Struct
												//LPARAM ist long Typ --> muss gecastet werden für Übertragung

	CString tmpID = msgGet->ID;

	map <CString, CProperty*> :: const_iterator foundProperty;

	foundProperty = propertyList.find(tmpID);

	//error handling - is foundProperty in map
	if (foundProperty == propertyList.end())
	{
		TRACE("CMachineModel Thread: Fatal error - Property or Mode not found in propertyList!!!\n");
		return;
	}

	double returnValue;

	Message_Get_Reply * infoGetReply = new Message_Get_Reply;

	if ((foundProperty->second)->Read(returnValue))	//falls get bzw read erfolgreich, return value true
	{													//returnValue enthält gelesener Wert
		TRACE("Wert erfolgreich gelesen!\n" );
		
		infoGetReply->ID = tmpID;
		infoGetReply->Value = returnValue;

		TRACE("returnValue: %f / infoGetReply->Value: %f.\n", returnValue, infoGetReply->Value);
		
		_DemoSorterGUIThread->PostMessage(GET_REPLY_MSG,0,(long)infoGetReply); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		TRACE( "Wert konnte nicht gelesen werden! No Reply Message to GUI posted!" );
		delete infoGetReply;
	}
	delete msgGet;
}

afx_msg void CMachineModel::onQuit(WPARAM wParam, LPARAM info)
{
	isFinished = true;
	PostQuitMessage(0); 
}

afx_msg void CMachineModel::onSetMessage(WPARAM wParam, LPARAM info)
{
	TRACE( "CMachineModel Thread: SET_MSG received\n" );

	Message_Set* msgSet = (Message_Set*) info;	//Message_Set ist Struct
												//LPARAM ist long Typ --> muss gecastet werden für Übertragung
	CString tmpID = msgSet->ID;

	TRACE("CMachineModel::onSetMessage		msgSet->ID = %s", msgSet->ID);

	map <CString, CProperty*> :: const_iterator foundProperty;

	foundProperty = propertyList.find(tmpID);

	//error handling - if foundProperty is in map
	if (foundProperty == propertyList.end())
	{
		TRACE("CMachineModel Thread: Fatal error - Property or Mode not found in propertyList!!!\n");
		return;
	}
	
	Message_Set_Reply * infoSetReply = new Message_Set_Reply;

	if ((foundProperty->second)->Write(msgSet->Value)) //falls set bzw write erfolgreich: true
	{
		TRACE("Wert erfolgreich gesetzt!" );

		infoSetReply->ID = tmpID;
		infoSetReply->Success = true;
		
		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)infoSetReply); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		TRACE( "Wert konnte nicht gesetzt werden! No Reply Message to GUI posted!" );

		infoSetReply->ID = tmpID;
		infoSetReply->Success = false;

		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)infoSetReply); //pointer braucht für msg-übertragung type cast und recast
	}
	delete msgSet;
		
}

bool CMachineModel::setNewAddr(const CString ipAddrArg, int valueArg)
{
	TRACE( "CMachineModel Thread: setNewAddr() called\n" );

	//Message_Set_Comm* msgSet = (Message_Set_Comm*) info;	//Message_Set ist Struct
												//LPARAM ist long Typ --> muss gecastet werden für Übertragung

	if (m_DSPComm->setNewConnection(ipAddrArg, valueArg))
	{
		TRACE("New Connection to %s : %d established!\n", ipAddrArg, valueArg);
		return true;
	}
	else
	{
		TRACE("Could not establish new Connection!\n");
	}

	return false;
}

afx_msg void CMachineModel::onResetMessage(WPARAM wParam, LPARAM info)
{
	Message_Set_Reply * infoSetReply = new Message_Set_Reply;
	infoSetReply->ID = "counts";
	if (counts->Reset())
	{
		infoSetReply->Success = true;
		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)infoSetReply); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		infoSetReply->Success = false;
		_DemoSorterGUIThread->PostMessage(SET_REPLY_MSG,0,(long)infoSetReply); //pointer braucht für msg-übertragung type cast und recast
	}
}

afx_msg void CMachineModel::onCountsMessage(WPARAM wParam, LPARAM info)
{
	TRACE(".\n");

	Message_Counts_Reply * msgCountReply = new Message_Counts_Reply;

	if (counts->Read(msgCountReply->countArr))
		_DemoSorterGUIThread->PostMessage(COUNTS_REPLY_MSG,0,(long)msgCountReply); //pointer braucht für msg-übertragung type cast und recast
	else
		TRACE("gaggi.\n");
}

