// VisionTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VisionTest.h"
#include "VisionTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Image.h"
#include "ImageDialog.h"
#include "ImageDialogManager.h"
#include "ModifyPropertyDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CVisionTestDlg dialog

CVisionTestDlg::CVisionTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVisionTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVisionTestDlg)
	m_chkChannelEnable = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strLastCalibrationFilename = "";

}	

CVisionTestDlg::~CVisionTestDlg()
{
	for (int i=0; i<MAX_CHANNELS ; i++ )
	{
		if ( m_Channels[i].pBuffer != NULL)
			delete [] m_Channels[i].pBuffer;
		
		delete m_Channels[i].pImageDialog;
	}

	// Delete the image dialog manager last
	delete CImageDialogManager::Instance();

	delete m_pVision;
}

void CVisionTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVisionTestDlg)
	DDX_Control(pDX, IDC_CHANNELPORTLIST, m_lChannelPortList);
	DDX_Control(pDX, IDC_CHANNELLIST, m_lChannelList);
	DDX_Control(pDX, IDC_PROPERTYLIST, m_lPropertyList);
	DDX_Control(pDX, IDC_PORTLIST, m_lPortList);
	DDX_Control(pDX, IDC_COMPONENTLIST, m_lComponentList);
	DDX_Check(pDX, IDC_CHANNEL_ENABLE, m_chkChannelEnable);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVisionTestDlg, CDialog)
	//{{AFX_MSG_MAP(CVisionTestDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOADIMAGESBUTTON, OnLoadimages)
	ON_BN_CLICKED(IDC_PROCESSALL, OnProcess)
	ON_BN_CLICKED(IDC_LOADIMAGESEQUENCE, OnLoadimagesequence)
	ON_LBN_SELCHANGE(IDC_COMPONENTLIST, OnSelchangeComponentlist)
	ON_BN_CLICKED(IDC_CHANNEL_ENABLE, OnChannelEnable)
	ON_LBN_SELCHANGE(IDC_CHANNELLIST, OnSelchangeChannellist)
	ON_LBN_SELCHANGE(IDC_CHANNELPORTLIST, OnSelchangeChannelportlist)
	ON_LBN_DBLCLK(IDC_PROPERTYLIST, OnDblclkPropertylist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisionTestDlg message handlers

BOOL CVisionTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// First, create the vision object.
	m_pVision = new CVision();
	
	// insert all component to the list
	UpdateComponentList();
	
	for (int i=0; i<MAX_CHANNELS ; i++ )
	{
		// Clear channels
		m_Channels[i].bEnabled = false;
		m_Channels[i].unWidth = 0;
		m_Channels[i].unHeight = 0;
		m_Channels[i].unBpp = 0;
		m_Channels[i].strComponentName = "";
		m_Channels[i].strPortName = "";
		m_Channels[i].pBuffer = NULL;

		CString strTitle;
		strTitle.Format("Channel %d", i);
		m_Channels[i].pImageDialog = new CImageDialog( strTitle, CWnd::GetDesktopWindow() );

		// Insert channel items to channel selection list
		CString str;
		int index;
		str.Format("Channel %d", i);
		index = m_lChannelList.InsertString( -1, str);
		m_lChannelList.SetItemData( index, (DWORD)i );
	}

	m_lChannelList.SetCurSel(0);

	UpdateData( FALSE );

	// Insert all output ports to the channel port selection list
	UpdateChannelPortList();

	m_pVision->ChangeMode( CVision::VM_CLASSIFICATION );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// *************************************************************************

void CVisionTestDlg::ProcessImage( CString strFile )
{
	CImage img( strFile, 0 );

	// Update data so we get the actual check boxes' values.
	UpdateData( TRUE );

	// Return if load failed;
	if (img.GetWidth() == 0)
		return;

	// Alloc input buffer and fill
	DWORD * buf = new DWORD[img.GetWidth() * img.GetHeight() ];
	img.FillRGBBuffer( buf );

	// Process the data
	m_pVision->FeedImage( buf );
	m_pVision->DoProcessing();
	
	// Copy the viewports and show them
	m_pVision->CopyViewPorts();
	ShowViewports();

	delete [] buf;	

}

// *************************************************************************


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.


void CVisionTestDlg::OnPaint() 
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

// *************************************************************************


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVisionTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// *************************************************************************

void CVisionTestDlg::OnProcess() 
{
	POSITION pos;
	CString strFile;

	if ( m_lImagefiles.IsEmpty() )
		return;

	pos = m_lImagefiles.GetHeadPosition();

	while (pos != NULL)
	{
		strFile = m_lImagefiles.GetNext(pos);
		ProcessImage( strFile );
	}
}

// *************************************************************************

void CVisionTestDlg::OnLoadimages() 
{
	char aryFileBuffer[ 16*1024 ];
	unsigned int unBufferLen = 16*1024 ;
	memset( aryFileBuffer, 0,  16*1024 );


	CFileDialog ldFile(TRUE, NULL, "", OFN_ALLOWMULTISELECT | OFN_ENABLESIZING , 
						"Images (bmp, jpg, png)|*.bmp;*.jpg;*.png||");
	
	// Set Buffer for the Filenames (this is only necessary because of Mulitselection)
	ldFile.m_ofn.lpstrFile = aryFileBuffer;
	ldFile.m_ofn.nMaxFile  = unBufferLen;

	if (ldFile.DoModal() != IDOK)
		return;

	CString  strPath, strFileName, strFileTitle, strFileExt;
	POSITION pos;
	
	// Clear the list
	ClearFiles();
	
	// Pick all files and add the to the list
	pos = ldFile.GetStartPosition();
	while (pos != NULL)
	{
		strPath = ldFile.GetNextPathName(pos);
		AddFile( strPath );
	}
}

// *************************************************************************

void CVisionTestDlg::OnLoadimagesequence() 
{
	int			nStartFileExt;
	CString		strFileExt;
	CString		strRoot;
	int			nUnderscorePos;
	int			nNumFramesFound;
	BOOL		bFound;			

		
	CFileDialog ldFile(TRUE, NULL, "", OFN_ENABLESIZING , 
							"Images (bmp, jpg, png)|*.bmp;*.jpg;*.png||");

	if (ldFile.DoModal() != IDOK)
		return;

	CString  strPath, strFileName, strFileTitle;
	
	// Clear the list
	ClearFiles();
	
	strFileName		= ldFile.GetPathName();
	nStartFileExt	= strFileName.ReverseFind('.') + 1;
	strFileExt		= strFileName.Right(strFileName.GetLength() - nStartFileExt);

	nUnderscorePos	= strFileName.ReverseFind('_');
	strRoot			= strFileName.Left(nUnderscorePos);
	

	// Find the number of available files.
	nNumFramesFound = 0;
	do
	{
		CFile file;
		CString strComposite;

		strComposite.Format("%s_%.3d.%s", strRoot, nNumFramesFound+1, strFileExt);
		bFound = file.Open( strComposite, CFile::modeRead );

		if (bFound)
		{
			AddFile(strComposite);
			nNumFramesFound++;

			file.Close();
		}		
	} while (bFound);
}

// *************************************************************************

void CVisionTestDlg::ClearFiles()
{
	while ( ! m_lImagefiles.IsEmpty() )
		m_lImagefiles.RemoveHead();
}

// *************************************************************************

void CVisionTestDlg::AddFile( CString strFile )
{
	// Is this the first element?
	if ( m_lImagefiles.IsEmpty() )
	{
		m_lImagefiles.AddHead( strFile );
		return;
	}

	// Now iterate until we find the correct place
	POSITION pos = m_lImagefiles.GetHeadPosition();
	while ( pos != NULL )
	{
		CString strComp;
		strComp = m_lImagefiles.GetAt( pos );

		// Iterate until the next element is greater than the one we're inserting. Insert
		// the current element BEFORE the one we just found.
		if ( strFile < strComp )
		{
			m_lImagefiles.InsertBefore( pos, strFile );
			return;
		}

		// Go on.
		m_lImagefiles.GetNext( pos );
	}

	m_lImagefiles.AddTail( strFile );
		
}

// *************************************************************************

void CVisionTestDlg::UpdateComponentList()
{
	Int32 id = -1;
	CVisObject * obj;
	CVisComponent * comp;

	m_lComponentList.ResetContent();

	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_COMPONENT, id ) )
	{
		int index;
		CString str;

		comp = (CVisComponent *)obj;

		str.Format( "%s:%s", comp->GetType(), obj->GetName() );
		index = m_lComponentList.InsertString( -1, str );
		m_lComponentList.SetItemDataPtr( index, (void*)obj );
	}
}	

// *************************************************************************

void CVisionTestDlg::UpdatePortList()
{
	CVisObject * obj;
	CVisPort *	port;
	CVisComponent * comp;

	if ( m_lComponentList.GetCurSel() == LB_ERR )
		return;

	// Get selected object
	
	Int32 id = -1;
	comp = (CVisComponent*) (m_lComponentList.GetItemDataPtr( m_lComponentList.GetCurSel() ) );
	
	m_lPortList.ResetContent();


	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PORT, id ) )
	{
		port = (CVisPort*)obj;
		if ( port->GetComponent() == comp )
		{
			int index;
			CString str;
			CVisPort * otherPort;
			CString strOtherPort;

			// Do we have info about the connected port?
			if ( port->GetConnectedPort( otherPort ) )
				strOtherPort.Format( "%s.%s", otherPort->GetComponent()->GetName(), otherPort->GetName() );
			else
				strOtherPort = "-";
			
			// Format the string
			if (port->GetType() == CVisPort::PT_INPUT)
				str.Format("%s <- %s",  port->GetName(), strOtherPort );
			else
				str.Format("%s -> %s", port->GetName(), strOtherPort );

			// Insert string
			index = m_lPortList.InsertString( -1, str );
			m_lPortList.SetItemDataPtr( index, (void*)port );
		}
	}
}

// *************************************************************************

void CVisionTestDlg::UpdatePropertyList()
{
	CVisObject * obj;
	CVisProperty *	prop;
	CVisComponent * comp;

	if ( m_lComponentList.GetCurSel() == LB_ERR )
		return;

	// Get selected object
	
	Int32 id = -1;
	comp = (CVisComponent*) (m_lComponentList.GetItemDataPtr( m_lComponentList.GetCurSel() ) );
	
	m_lPropertyList.ResetContent();


	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PROPERTY, id ) )
	{
		prop = (CVisProperty*)obj;
		if ( prop->GetComponent() == comp )
		{
			int index;
			float value;
			CString str;
			
			prop->GetFloatValue( value );
			str.Format("%s = %.3f", prop->GetName() , value );

			index = m_lPropertyList.InsertString( -1, str );
			m_lPropertyList.SetItemDataPtr( index, (void*)prop );
		}
	}
}


// *************************************************************************

void CVisionTestDlg::UpdateChannelPortList()
{
	CVisObject *	obj;
	CVisPort *		port;
	CVisComponent *	comp;
	Int32			id = -1;

	m_lChannelPortList.ResetContent();

	// Enumerate through all ports
	while( CVisObjectManager::Instance()->GetNextObject( &obj, CVisObject::CT_PORT, id ) )
	{
		// Get the port
		port = (CVisPort*)obj;

		// Must be an output port! Input ports do not hold image data and thus cannot
		// be used as viewport.
		if ( port->GetType() == CVisPort::PT_OUTPUT )
		{
			comp = port->GetComponent();

			// Format string
			CString str;
			str.Format("%s.%s", comp->GetName(), port->GetName() );

			// Add entry
			int index;
			index = m_lChannelPortList.InsertString( -1, str );
		}
	}
}

// *************************************************************************

void CVisionTestDlg::SetupViewport( int channel )
{
	// First, delete the buffer if there is any
	if ( m_Channels[channel].pBuffer != NULL )
		delete [] m_Channels[channel].pBuffer;
	m_Channels[channel].pBuffer = NULL;

	// Now enable the viewport at the vision object
	if ( ! m_pVision->EnableViewPort( channel, m_Channels[channel].strComponentName, m_Channels[channel].strPortName ) )
	{
		MessageBox( "Could not enable viewport!", "Error", MB_ICONEXCLAMATION | MB_OK );
		return;
	}

	// Get the image size and bpp
	bool bData;
	m_pVision->GetViewPortImageInfo(	channel,
									m_Channels[channel].unWidth, 
									m_Channels[channel].unHeight, 
									m_Channels[channel].unBpp,
									bData,
									m_Channels[channel].bIndexed);

	// Allocate the new buffer
	int size;
	size = m_Channels[channel].unWidth * m_Channels[channel].unHeight * m_Channels[channel].unBpp / 8;
	m_Channels[channel].pBuffer = new BYTE[size];

	// And feed it to the vision object
	m_pVision->FeedViewPortBuffer( channel, m_Channels[channel].pBuffer );
}

// *************************************************************************

void CVisionTestDlg::ShowViewports()
{
	for (int i=0; i<MAX_CHANNELS ; i++ )
	{
		if ( m_Channels[i].bEnabled )
		{
			CImage * img;
			img = new CImage();

			switch ( m_Channels[i].unBpp )
			{
			case 1:
				// TODO:
				break;

			case 8:
				if (m_Channels[i].bIndexed)
					img->CreateFromIndexed8BPP( m_Channels[i].unWidth, m_Channels[i].unHeight, m_Channels[i].pBuffer );
				else
					img->CreateFromGray( m_Channels[i].unWidth, m_Channels[i].unHeight, m_Channels[i].pBuffer );
				break;

			case 32:
				img->CreateFromRGB( m_Channels[i].unWidth, m_Channels[i].unHeight, m_Channels[i].pBuffer );
				break;
			}
		
			// Copy the image to the dialog and show it
			m_Channels[i].pImageDialog->AddImage( *img );
			m_Channels[i].pImageDialog->ShowWindow( SW_SHOW );

			// Since the image is fully copied, we won't need it anymore.
			delete img;
		}
	}
}

// *************************************************************************

void CVisionTestDlg::OnSelchangeComponentlist() 
{
	UpdatePortList();	
	UpdatePropertyList();
}
// *************************************************************************

void CVisionTestDlg::OnChannelEnable() 
{
	int channel;

	UpdateData(true);
	
	// Return if there is no selection in the channel list
	if ( m_lChannelList.GetCurSel() == LB_ERR )
		return;

	// Get the selected channel
	channel = (int)m_lChannelList.GetItemData( m_lChannelList.GetCurSel() );

	m_Channels[channel].bEnabled = m_chkChannelEnable;
}

// *************************************************************************

void CVisionTestDlg::OnSelchangeChannellist() 
{
	int channel;
	
	// Return if there is no selection in the channel list
	if ( m_lChannelList.GetCurSel() == LB_ERR )
		return;

	// Get the selected channel
	channel = (int)m_lChannelList.GetItemData( m_lChannelList.GetCurSel() );

	// Update the data on screen
	CString str;

	m_chkChannelEnable = m_Channels[channel].bEnabled;
	str.Format( "%s.%s", m_Channels[channel].strComponentName, m_Channels[channel].strPortName );
	if ( m_lChannelPortList.SelectString( -1, str ) == LB_ERR)
		m_lChannelPortList.SetCurSel(-1);

	UpdateData( false );
}

// *************************************************************************

void CVisionTestDlg::OnSelchangeChannelportlist() 
{
	int		channel;
	CString strComplete;
	
	// Return if there is no selection in the channel list
	if ( m_lChannelList.GetCurSel() == LB_ERR )
		return;

	// Get the selected channel
	channel = (int)m_lChannelList.GetItemData( m_lChannelList.GetCurSel() );

	m_lChannelPortList.GetText(m_lChannelPortList.GetCurSel(), strComplete );

	// Extract the port and component strings
	m_Channels[channel].strComponentName = strComplete.Left( strComplete.Find(".") );
	m_Channels[channel].strPortName = strComplete.Right( strComplete.GetLength() - strComplete.Find(".") - 1 );

	// Now enable the viewport at the vision object
	SetupViewport( channel );
}

// *************************************************************************

void CVisionTestDlg::OnDblclkPropertylist() 
{
	CVisProperty * prop;
	CString strComplete;
	
	// Return if there is no selection in the property list
	if ( m_lPropertyList.GetCurSel() == LB_ERR )
		return;

	// Get the selected property
	prop = (CVisProperty*)m_lPropertyList.GetItemData( m_lPropertyList.GetCurSel() );

	// Open the modify dialog
	CModifyPropertyDlg dlg( this, prop );
	dlg.DoModal();

	// Refresh the property list
	UpdatePropertyList();
}

// *************************************************************************
