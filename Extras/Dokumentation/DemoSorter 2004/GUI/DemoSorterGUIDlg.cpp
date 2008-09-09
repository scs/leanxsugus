// DemoSorterGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "DemoSorterGUIDlg.h"
#include "BmpButton.h"
//#include "CDataFile.h"
//#include <stdio.h>
//#include <float.h>	// needed for the FLT_MIN define


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemoSorterGUIDlg dialog

CDemoSorterGUIDlg::CDemoSorterGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemoSorterGUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDemoSorterGUIDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoSorterGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDemoSorterGUIDlg)
	DDX_Control(pDX, IDC_COLOR1, m_color1Button);
	DDX_Control(pDX, IDC_RESET, m_reset);
	DDX_Control(pDX, IDC_COUNT3, m_count3);
	DDX_Control(pDX, IDC_COUNT2, m_count2);
	DDX_Control(pDX, IDC_COUNT1, m_count1);
	DDX_Control(pDX, IDC_COUNT0, m_count0);
	DDX_Control(pDX, IDC_TIMERSEL, m_timerSel);
	DDX_Control(pDX, IDC_COLOR0, m_color0Button);
	DDX_Control(pDX, IDC_COLOR2, m_color2Button);
	DDX_Control(pDX, IDC_COLOR3, m_color3Button);
	DDX_Control(pDX, IDC_BTN_RESET, m_resetButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDemoSorterGUIDlg, CDialog)
	//{{AFX_MSG_MAP(CDemoSorterGUIDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_COLOR0, OnColor0)
	ON_BN_CLICKED(IDC_COLOR1, OnColor1)
	ON_BN_CLICKED(IDC_COLOR2, OnColor2)
	ON_BN_CLICKED(IDC_COLOR3, OnColor3)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_TIMERSEL, OnTimersel)
	ON_BN_CLICKED(IDC_BTN_RESET, OnBtnReset)
	ON_MESSAGE(SET_REPLY_MSG, OnSetReplyMessage)
	ON_MESSAGE(GET_REPLY_MSG, OnGetReplyMessage)
	ON_MESSAGE(COUNTS_REPLY_MSG, OnCountsReplyMessage)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoSorterGUIDlg message handlers

BOOL CDemoSorterGUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//this->ShowWindow(SW_MAXIMIZE);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	
	m_resetButton.LoadButtonBitmaps(IDB_RESET_UNPRESSED, IDB_RESET_UNPRESSED, IDB_RESET_PRESSED, 0);

	m_color0Button.LoadButtonBitmaps(IDB_YELLOW_UNPRESSED, IDB_YELLOW_UNPRESSED, IDB_YELLOW_PRESSED);
	m_color1Button.LoadButtonBitmaps(IDB_ORANGE_UNPRESSED, IDB_ORANGE_UNPRESSED, IDB_ORANGE_PRESSED);
	m_color2Button.LoadButtonBitmaps(IDB_RED_UNPRESSED, IDB_RED_UNPRESSED, IDB_RED_PRESSED);
	m_color3Button.LoadButtonBitmaps(IDB_GREEN_UNPRESSED, IDB_GREEN_UNPRESSED, IDB_GREEN_PRESSED);
	//m_color4Button.LoadButtonBitmaps(IDB_COLOR4_UNPRESSED, IDB_COLOR4_UNPRESSED, IDB_COLOR4_PRESSED);
	//m_color5Button.LoadButtonBitmaps(IDB_COLOR5_UNPRESSED, IDB_COLOR5_UNPRESSED, IDB_COLOR5_PRESSED);
	//m_color6Button.LoadButtonBitmaps(IDB_COLOR6_UNPRESSED, IDB_COLOR6_UNPRESSED, IDB_COLOR6_PRESSED);
	//m_color7Button.LoadButtonBitmaps(IDB_COLOR7_UNPRESSED, IDB_COLOR7_UNPRESSED, IDB_COLOR7_PRESSED);

	//Kreieren und starten des CMachineModel Threads:
	_MachineModelThread = new CMachineModel(); //referenziere neues MachineModel Objekt mit globalem Pointer dieser Klasse
	_MachineModelThread->connectGuiModelThread( dynamic_cast<CWnd*>(this) ); 
	_MachineModelThread->CreateThread(0,0);	//create and start Thread _MachineModelThread
	
	serviceDlg.submitMachineP(_MachineModelThread);

	this->GetFocus(); 


	



	// Do something with the font just created...
	//CClientDC dc(this);
	//CFont* def_font = dc.SelectObject(&font);
	//dc.TextOut(5, 5, "Hello", 5);
	//dc.SelectObject(def_font);

	// Done with the font.  Delete the font object.
	//font.DeleteObject();


	/*
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

	connect = ExistingDF.GetInt("timerenable", "init");
	if ( connect == INT_MIN )
	{
		Report(E_INFO, "[CDSPComm::getPreferences] Key 'connect' was not found.");
	
	}
	else
	{
		if (connect==1)
		{
			SetTimer(0,500,0); //setze Timer auf 500 ms
			TRACE("Timer set.\n");
		}
		else 
		{
			TRACE("Timer startup disabled.\n");
		}
	}
	*/

	//OnOpenservicedlg();
	//SetTimer(0,500,0); //setze Timer auf 500 ms


	// Hide cursor
	ShowCursor( FALSE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDemoSorterGUIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDemoSorterGUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
int counter = 0;
void CDemoSorterGUIDlg::OnTimer(UINT nIDEvent) 
{
	//this->GetFocus();
	//Function for periodic requests for count properties
	if(_MachineModelThread->m_DSPComm->isConnectedToEtrax())
	{
		/*Message_Get * infoGet[NUMCOLORPROPS];

		CString tmpString;
		
		for (int i = 0; i < NUMCOLORPROPS; i++)
		{
			infoGet[i] = new Message_Get;
			tmpString.Format("Color%d.Count",i);
			infoGet[i]->ID = tmpString;
			_MachineModelThread->PostThreadMessage(GET_MSG, 0, (long)infoGet[i]);
		}*/
		counter++;
		_MachineModelThread->PostThreadMessage(COUNTS_MSG, 0, 0);
		TRACE("%d\n", counter);
		CDialog::OnTimer(nIDEvent);
	}
}

void CDemoSorterGUIDlg::OnSetReplyMessage(WPARAM wParam, LPARAM info)
{
	TRACE("CDemoSorterGUIDlg: Set Reply Message received!!!\n");
	
	Message_Set_Reply* msgSetReply = (Message_Set_Reply*) info;	//Message_Set ist Struct
												//LPARAM ist long Typ --> muss gecastet werden für Übertragung

	TRACE("CDemoSorterGUIDlg: Property: %s / Sendeerfolg: %d!!!\n", msgSetReply->ID, msgSetReply->Success);


	if (msgSetReply->ID == "Connect on startup")
	{
		if(msgSetReply->Success==false)
		{
			CString shithappensStr;
			shithappensStr.Format("The property %s could not be set.\nProbably a connection error.", msgSetReply->ID);
			AfxMessageBox(shithappensStr);	
			delete msgSetReply;
			return;
		}
			
		else
		{
			TRACE("timerenable: %d", _MachineModelThread->m_DSPComm->timerenable);
			TRACE("showcounters: %d", _MachineModelThread->m_DSPComm->showcounters);

			if (_MachineModelThread->m_DSPComm->timerenable==1)
				SetTimer(0,500,0); //setze Timer auf 500 ms

			if (_MachineModelThread->m_DSPComm->showcounters==1)
			{
				CFont * fonti = new CFont;
				VERIFY(fonti->CreateFont(
				20,                        // nHeight
				0,                         // nWidth
				0,                         // nEscapement
				0,                         // nOrientation
				FW_NORMAL,                 // nWeight
				FALSE,                     // bItalic
				FALSE,                     // bUnderline
				0,                         // cStrikeOut
				ANSI_CHARSET,              // nCharSet
				OUT_DEFAULT_PRECIS,        // nOutPrecision
				CLIP_DEFAULT_PRECIS,       // nClipPrecision
				DEFAULT_QUALITY,           // nQuality
				DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
				"Arial"));                 // lpszFacename

				m_count0.SetFont(fonti); 
				m_count1.SetFont(fonti); 
				m_count2.SetFont(fonti); 
				m_count3.SetFont(fonti); 

				m_count0.ShowWindow( SW_SHOW );
				m_count1.ShowWindow( SW_SHOW );
				m_count2.ShowWindow( SW_SHOW );
				m_count3.ShowWindow( SW_SHOW );
			}
		}
	}
	if (msgSetReply->ID == "mode")
	{
		TRACE("CDemoSorterGUIDlg msgSetReply: mode set reply received. Service dialog is opened. Success: %d.\n", msgSetReply->Success);
	
		if (msgSetReply->Success == false) //wiederherstellen des alten Wertes
		{
			serviceDlg.mode = serviceDlg.modeSave;
			serviceDlg.m_debugText.SetWindowText("Could not set another mode type. Old mode reset.");
			switch (serviceDlg.mode)
			{
				case 0:		serviceDlg.m_idleSel.SetCheck(1);
							serviceDlg.m_calibrationSel.SetCheck(0);
							serviceDlg.m_processingSel.SetCheck(0);
							serviceDlg.m_serviceSel.SetCheck(0);
							serviceDlg.m_debugText.SetWindowText("Could not set another mode type. Old mode reset. mode is idle.");
							break;
				case 1:		serviceDlg.m_processingSel.SetCheck(1);
							serviceDlg.m_idleSel.SetCheck(0);
							serviceDlg.m_calibrationSel.SetCheck(0);
							serviceDlg.m_serviceSel.SetCheck(0);
							serviceDlg.m_debugText.SetWindowText("Could not set another mode type. Old mode reset. mode is processing.");
							break;
				case 2:		serviceDlg.m_calibrationSel.SetCheck(1);
							serviceDlg.m_idleSel.SetCheck(0);
							serviceDlg.m_processingSel.SetCheck(0);
							serviceDlg.m_serviceSel.SetCheck(0);
							serviceDlg.m_debugText.SetWindowText("Could not set another mode type. Old mode reset. mode is calibration.");
							break;
				case 3:		serviceDlg.m_calibrationSel.SetCheck(0);
							serviceDlg.m_idleSel.SetCheck(0);
							serviceDlg.m_processingSel.SetCheck(0);
							serviceDlg.m_serviceSel.SetCheck(1);
							serviceDlg.m_debugText.SetWindowText("Could not set another mode type. Old mode reset. mode is service.");
							break;
			}
		}
		else //--> behalte neuen Wert
		{
			serviceDlg.m_debugText.SetWindowText("connected. mode successfull set.");

		}
		
	}
	/*
	else 
	{
		CString testString;
		int i;
		for (i = 0; i < NUMCOLORPROPS+1; i++)
		{
			testString.Format("Color%d.Count",i);
			if (msgSetReply->ID == testString && msgSetReply->Success)
				break;
		}
		switch (i){
		case 0: m_count0.SetWindowText("0 x");
			break;
		case 1: m_count1.SetWindowText("0 x");
			break;
		case 2: m_count2.SetWindowText("0 x");
			break;
		case 3: m_count3.SetWindowText("0 x");
			break;
		case 4: m_count4.SetWindowText("0 x");
			break;
		case 5: m_count5.SetWindowText("0 x");
			break;
		case 6: m_count6.SetWindowText("0 x");
			break;
		case 7: m_count7.SetWindowText("0 x");
			break;
		}
	}
	*/
	delete msgSetReply;
}

void CDemoSorterGUIDlg::OnGetReplyMessage(WPARAM wParam, LPARAM info)
{
	TRACE("CDemoSorterGUIDlg: Get Reply Message received!!!\n");

	Message_Get_Reply* msgGetReply = (Message_Get_Reply*) info;	//Message_Set ist Struct
												//LPARAM ist long Typ --> muss gecastet werden für Übertragung

	TRACE("CDemoSorterGUIDlg msgGetReply: Property: %s / gelesener Wert: %f!!!\n", msgGetReply->ID, msgGetReply->Value);

	if (msgGetReply->ID == "mode")
	{
		TRACE("CDemoSorterGUIDlg: mode get reply received. Service dialog is opened.\n");
		serviceDlg.modeSave = serviceDlg.mode;
		serviceDlg.mode = (int)msgGetReply->Value;
		switch ((int)msgGetReply->Value)
		{
		case 0:		serviceDlg.m_idleSel.SetCheck(1);
					serviceDlg.m_calibrationSel.SetCheck(0);
					serviceDlg.m_processingSel.SetCheck(0);
					serviceDlg.m_serviceSel.SetCheck(0);
					serviceDlg.m_debugText.SetWindowText("connected. mode is idle.");
					break;
		case 1:		serviceDlg.m_processingSel.SetCheck(1);
					serviceDlg.m_idleSel.SetCheck(0);
					serviceDlg.m_calibrationSel.SetCheck(0);
					serviceDlg.m_serviceSel.SetCheck(0);
					serviceDlg.m_debugText.SetWindowText("connected. mode is processing.");
					break;
		case 2:		serviceDlg.m_calibrationSel.SetCheck(1);
					serviceDlg.m_idleSel.SetCheck(0);
					serviceDlg.m_processingSel.SetCheck(0);
					serviceDlg.m_serviceSel.SetCheck(0);
					serviceDlg.m_debugText.SetWindowText("connected. mode is calibration.");
					break;
		case 3:		serviceDlg.m_calibrationSel.SetCheck(0);
					serviceDlg.m_idleSel.SetCheck(0);
					serviceDlg.m_processingSel.SetCheck(0);
					serviceDlg.m_serviceSel.SetCheck(1);
					serviceDlg.m_debugText.SetWindowText("connected. mode is service.");
					break;
		}
	}
	/*
	else 
	{
		CString testString;
		int i = 5;
		for (i = 0; i < NUMCOLORPROPS+1; i++)
		{
			testString.Format("Color%d.Count",i);
			if (msgGetReply->ID == testString)
			{
				testString.Format("%f x", msgGetReply->Value);
				break;
			}
		}
		switch (i){
		case 0: 
			m_count0.SetWindowText(testString);
			TRACE("we are in the switch. color0.count. %s\n", testString);
			break;
		case 1: 
			m_count1.SetWindowText(testString);
			TRACE("we are in the switch. color1.count. %s\n", testString);
			break;
		case 2: 
			m_count2.SetWindowText(testString);
			TRACE("we are in the switch. color2.count. %s\n", testString);
			break;
		case 3: 
			m_count3.SetWindowText(testString);
			TRACE("we are in the switch. color3.count. %s\n", testString);
			break;
		case 4: 
			m_count4.SetWindowText(testString);
			TRACE("we are in the switch. color4.count. %s\n", testString);
			break;
		case 5: 
			m_count5.SetWindowText(testString);
			TRACE("we are in the switch. color5.count. %s\n", testString);
			break;
		case 6: 
			m_count6.SetWindowText(testString);
			TRACE("we are in the switch. color6.count. %s\n", testString);
			break;
		case 7: 
			m_count7.SetWindowText(testString);
			TRACE("we are in the switch. color7.count. %s\n", testString);
			break;
		}
	}
	*/
	delete msgGetReply;
}

void CDemoSorterGUIDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (point.x < 75 && point.y < 75)
	{
		ShowCursor( TRUE );
		serviceDlg.DoModal();
		ShowCursor( FALSE );
	}
	else if (point.x > 725 && point.y < 75)
	{
		int iResults = MessageBox("Möchten Sie wirklich DemoSorter beenden?", "DemoSorter beenden", MB_YESNO | MB_ICONQUESTION);
		if (iResults == IDYES)
			OnOK();

	}

	CDialog::OnLButtonDown(nFlags, point);

	//PostMessage( WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM( point.x, point.y));
}	

void CDemoSorterGUIDlg::OnColor0()
{
	Message_Set * infoSet = new Message_Set();

	infoSet->ID = "Color0.Eject";
	
	if (m_color0Button.state)
	{
		infoSet->Value = 1;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		infoSet->Value = 0;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet);
	}	

}

void CDemoSorterGUIDlg::OnColor1()
{
	Message_Set * infoSet = new Message_Set();

	infoSet->ID = "Color1.Eject";
	
	if (m_color1Button.state)
	{
		infoSet->Value = 1;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		infoSet->Value = 0;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet);
	}	
}

void CDemoSorterGUIDlg::OnColor2()
{
	Message_Set * infoSet = new Message_Set();

	infoSet->ID = "Color2.Eject";
	
	if (m_color2Button.state)
	{
		infoSet->Value = 1;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		infoSet->Value = 0;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet);
	}	
}

void CDemoSorterGUIDlg::OnColor3()
{
	Message_Set * infoSet = new Message_Set();

	infoSet->ID = "Color3.Eject";
	
	if (m_color3Button.state)
	{
		infoSet->Value = 1;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet); //pointer braucht für msg-übertragung type cast und recast
	}
	else
	{
		infoSet->Value = 0;
		_MachineModelThread->PostThreadMessage(SET_MSG,0,(long)infoSet);
	}	
}



void CDemoSorterGUIDlg::OnDestroy() 
{
	
	//if(_MachineModelThread->m_DSPComm->isConnectedToEtrax())
	//	KillTimer(0);

	//delete serviceDlg;
	//_MachineModelThread->PostThreadMessage(WM_QUIT, 0,0);
	//while (!_MachineModelThread->isFinished)
	//{
	//	Sleep(100);// 100ms 
	//}
	//Sleep(1000);
	//delete _MachineModelThread; //Destruktor wird durch 'auto'-Funktion automatisch aufgerufen
	
	CDialog::OnDestroy();

}


void CDemoSorterGUIDlg::OnReset() 
{
	/*
	Message_Set * infoSet[NUMCOLORPROPS];

	CString tmpString;
	
	for (int i = 0; i < NUMCOLORPROPS; i++)
	{
		infoSet[i] = new Message_Set;
		tmpString.Format("Color%d.Count",i);
		infoSet[i]->ID = tmpString;
		infoSet[i]->Value = 0;
		_MachineModelThread->PostThreadMessage(SET_MSG, 0, (long)infoSet[i]);
	}
	*/
	if(_MachineModelThread->m_DSPComm->isConnectedToEtrax())
	{
		_MachineModelThread->PostThreadMessage(RESET_MSG, 0, 0);
	}
}

void CDemoSorterGUIDlg::OnTimersel() 
{
//	if (m_timerSel.GetCheck())
//	{
//		SetTimer(0,500,0); //setze Timer auf 500 ms
//		TRACE("Timer set.\n");
//	else if (!m_timerSel.GetCheck())
//	{
//		KillTimer(0);
//		TRACE("Timer killed.\n");
//	}
}

afx_msg void CDemoSorterGUIDlg::OnCountsReplyMessage(WPARAM wParam, LPARAM info)
{
	Message_Counts_Reply* msgCountsReply = (Message_Counts_Reply*) info;

	CString testString;

	testString.Format("%d x", (int)msgCountsReply->countArr[0]);
	m_count0.SetWindowText(testString);
	testString.Format("%d x", (int)msgCountsReply->countArr[1]);
	m_count1.SetWindowText(testString);
	testString.Format("%d x", (int)msgCountsReply->countArr[2]);
	m_count2.SetWindowText(testString);
	testString.Format("%d x", (int)msgCountsReply->countArr[3]);
	m_count3.SetWindowText(testString);
	//testString.Format("%d x", (int)msgCountsReply->countArr[4]);
	//m_count4.SetWindowText(testString);
	//testString.Format("%d x", (int)msgCountsReply->countArr[5]);
	//m_count5.SetWindowText(testString);
	//testString.Format("%d x", (int)msgCountsReply->countArr[6]);
	//m_count6.SetWindowText(testString);
	//testString.Format("%d x", (int)msgCountsReply->countArr[7]);
	//m_count7.SetWindowText(testString);

	delete msgCountsReply;
}

void CDemoSorterGUIDlg::OnOK() 
{
	if(_MachineModelThread->m_DSPComm->isConnectedToEtrax())
	KillTimer(0);

	delete serviceDlg;
	_MachineModelThread->PostThreadMessage(WM_QUIT, 0,0);
	//while (!_MachineModelThread->isFinished)
	//{
	//	Sleep(100);// 100ms 
	//}
	Sleep(1000);
	//delete _MachineModelThread; //Destruktor wird durch 'auto'-Funktion automatisch aufgerufen
	exit(0);
	//CDialog::OnOK();
}


void CDemoSorterGUIDlg::OnBtnReset() 
{
	OnReset();
}



	


void CDemoSorterGUIDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDialog::OnMouseMove(nFlags, point);
}
