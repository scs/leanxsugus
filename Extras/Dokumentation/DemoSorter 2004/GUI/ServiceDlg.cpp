// ServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "ServiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServiceDlg dialog


CServiceDlg::CServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServiceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
void CServiceDlg::submitMachineP(CMachineModel * _MachineModelThread)
{
	machineModel = _MachineModelThread;
}

void CServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServiceDlg)
	DDX_Control(pDX, IDC_SERVICESEL, m_serviceSel);
	DDX_Control(pDX, IDC_PROCESSINGSEL, m_processingSel);
	DDX_Control(pDX, IDC_IDLESEL, m_idleSel);
	DDX_Control(pDX, IDC_CALIBRATIONSEL, m_calibrationSel);
	DDX_Control(pDX, IDC_DEBUGTEXT, m_debugText);
	DDX_Control(pDX, IDC_STATUS, m_statusGUI);
	DDX_Control(pDX, IDC_PORT, m_port);
	DDX_Control(pDX, IDC_IPADDR, m_ipAddr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServiceDlg, CDialog)
	//{{AFX_MSG_MAP(CServiceDlg)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_IDLESEL, OnIdlesel)
	ON_BN_CLICKED(IDC_PROCESSINGSEL, OnProcessingsel)
	ON_BN_CLICKED(IDC_CALIBRATIONSEL, OnCalibrationsel)
	ON_BN_CLICKED(IDC_SERVICESEL, OnServicesel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServiceDlg message handlers

void CServiceDlg::OnConnect() 
{
	m_statusGUI.SetWindowText("connecting ......");

	Message_Get * infoGet = new Message_Get;

	CString ipStringOld = ipString;
	CString portStringOld = portString;

	if (m_ipAddr.GetCurSel() != LB_ERR && m_port.GetWindowTextLength() < 6 && m_port.GetWindowTextLength() > 1)
	{
		m_ipAddr.GetWindowText(ipString);
		m_port.GetWindowText(portString);
		m_statusGUI.SetWindowText("attempting to connect with " + ipString + " : " + portString + " ...");
		
		const char* portNumber = LPCTSTR(portString);
	
		if (machineModel->setNewAddr(ipString, atoi(portNumber)))
		{
			m_statusGUI.SetWindowText("connected to " + ipString + " : " + portString + " .");
			infoGet->ID = "mode";
			machineModel->PostThreadMessage(GET_MSG, 0, (long)infoGet);
		}
		else
		{
			m_statusGUI.SetWindowText("connection failed. no connection.");
			delete infoGet;
		}
	}
	else
	{
		TRACE("invalid port and/or ip address!\n");
		delete infoGet;
	}
			
}

BOOL CServiceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	this->ShowWindow(SW_MAXIMIZE);

	Message_Get * infoGet = new Message_Get;

	infoGet->ID = "mode";


	commP = machineModel->submitDSPCommP();

	m_ipAddr.AddString("212.254.229.180");
	m_ipAddr.AddString("127.0.0.1");

	m_port.SetWindowText("4444");

	if (commP->isConnectedToEtrax())
	{
		machineModel->PostThreadMessage(GET_MSG, 0, (long)infoGet);

		m_statusGUI.SetWindowText("connected to " + commP->getIpAddr() + " .");
	}
	else
	{
		m_statusGUI.SetWindowText("no connection.");
		m_debugText.SetWindowText("no connection. no mode could be loaded.");
		delete infoGet;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServiceDlg::OnIdlesel() 
{
	m_serviceSel.SetCheck(!m_idleSel.GetCheck());
	m_processingSel.SetCheck(!m_idleSel.GetCheck());
	m_calibrationSel.SetCheck(!m_idleSel.GetCheck());

	Message_Set * infoSet = new Message_Set;

	infoSet->ID = "mode";
	
	if (this->m_idleSel.GetCheck())
	{
		infoSet->Value = 0;
		if (commP->isConnectedToEtrax())
		{
			machineModel->PostThreadMessage(SET_MSG, 0, (long)infoSet);
		}
		else
		{
			delete infoSet;
			this->m_debugText.SetWindowText("not connected. mode control not available!");
		}
	}
}

void CServiceDlg::OnProcessingsel() 
{
	m_serviceSel.SetCheck(!m_processingSel.GetCheck());
	m_idleSel.SetCheck(!m_processingSel.GetCheck());
	m_calibrationSel.SetCheck(!m_processingSel.GetCheck());

	Message_Set * infoSet = new Message_Set;

	infoSet->ID = "mode";
	
	if (this->m_processingSel.GetCheck())
	{
		infoSet->Value = 1;
		if (commP->isConnectedToEtrax())
		{
			machineModel->PostThreadMessage(SET_MSG, 0, (long)infoSet);
		}
		else
		{
			delete infoSet;
			this->m_debugText.SetWindowText("not connected. mode control not available!");
		}
	}
}

void CServiceDlg::OnCalibrationsel() 
{
	m_serviceSel.SetCheck(!m_calibrationSel.GetCheck());
	m_processingSel.SetCheck(!m_calibrationSel.GetCheck());
	m_idleSel.SetCheck(!m_calibrationSel.GetCheck());

	Message_Set * infoSet = new Message_Set;

	infoSet->ID = "mode";
	
	if (this->m_calibrationSel.GetCheck())
	{
		infoSet->Value = 2;
		if (commP->isConnectedToEtrax())
		{
			machineModel->PostThreadMessage(SET_MSG, 0, (long)infoSet);
		}
		else
		{
			delete infoSet;
			this->m_debugText.SetWindowText("not connected. mode control not available!");
		}
	}
}

void CServiceDlg::OnOK() 
{
	// TODO: Add extra validation here

	CDialog::OnOK();
}

void CServiceDlg::OnServicesel() 
{
	m_calibrationSel.SetCheck(!m_serviceSel.GetCheck());
	m_processingSel.SetCheck(!m_serviceSel.GetCheck());
	m_idleSel.SetCheck(!m_serviceSel.GetCheck());

	Message_Set * infoSet = new Message_Set;

	infoSet->ID = "mode";
	
	if (this->m_serviceSel.GetCheck())
	{
		infoSet->Value = 3;
		if (commP->isConnectedToEtrax())
		{
			machineModel->PostThreadMessage(SET_MSG, 0, (long)infoSet);
		}
		else
		{
			delete infoSet;
			this->m_debugText.SetWindowText("not connected. mode control not available!");
		}
	}
}
