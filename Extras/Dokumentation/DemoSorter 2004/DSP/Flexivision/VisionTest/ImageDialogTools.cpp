// ImageDialogTools.cpp : implementation file
//

#include "stdafx.h"
#include "VisionTest.h"
#include "ImageDialogTools.h"
#include "ImageDialogManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageDialogTools dialog


CImageDialogTools::CImageDialogTools( CImageDialog * pParent )
	: CDialog(CImageDialogTools::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImageDialogTools)
	m_bLinked = FALSE;
	//}}AFX_DATA_INIT

	
	// Create the dialog
	Create( IDD, pParent );
	ShowWindow( SW_HIDE );

	m_pImageDialog = pParent;

	m_bIsShowing = false;
}


void CImageDialogTools::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageDialogTools)
	DDX_Control(pDX, IDC_SHOWHIDEBUTTON, m_pbShowHideButton);
	DDX_Control(pDX, IDC_IMAGESLIST, m_lImagesList);
	DDX_Check(pDX, IDC_LINKCHECK, m_bLinked);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImageDialogTools, CDialog)
	//{{AFX_MSG_MAP(CImageDialogTools)
	ON_BN_CLICKED(IDC_SHOWHIDEBUTTON, OnShowhidebutton)
	ON_LBN_SELCHANGE(IDC_IMAGESLIST, OnSelchangeImageslist)
	ON_BN_CLICKED(IDC_REWINDBUTTON, OnRewind)
	ON_BN_CLICKED(IDC_FASTFORWARDBUTTON, OnFastforward)
	ON_BN_CLICKED(IDC_FRAMEBACKBUTTON, OnFrameback)
	ON_BN_CLICKED(IDC_FRAMENEXTBUTTON, OnFramenext)
	ON_BN_CLICKED(IDC_PLAYBUTTON, OnPlay)
	ON_BN_CLICKED(IDC_STOPBUTTON, OnStop)
	ON_BN_CLICKED(IDC_TOENDBUTTON, OnToEnd)
	ON_BN_CLICKED(IDC_CLEARBUTTON, OnClearbutton)
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_LINKCHECK, OnLinkcheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDialogTools message handlers

void CImageDialogTools::Show( bool bShow )
{
	m_bIsShowing = bShow;
	if ( m_bIsShowing )
		m_pbShowHideButton.SetWindowText( "<<" );
	else
		m_pbShowHideButton.SetWindowText( ">>" );

	AdjustPosition();
}

// *************************************************************************

bool CImageDialogTools::IsShowing()
{
	return m_bIsShowing;
}

// *************************************************************************

void CImageDialogTools::OnShowhidebutton() 
{
	if ( m_bIsShowing )
		m_pImageDialog->ShowTools( false );
	else	
		m_pImageDialog->ShowTools( true );
}

// *************************************************************************

void  CImageDialogTools::AdjustPosition()
{
	RECT rect;
	RECT rect_tools;

	if ( m_pImageDialog == NULL )
		return;

	m_pImageDialog->GetWindowRect( &rect );
	GetWindowRect( &rect_tools );

	int x,y;
	int dx,dy;

	y = rect.top;
	x = rect.right+1;
	dy = rect_tools.bottom - rect_tools.top;

	if ( m_bIsShowing )
		dx = m_nFullWidth;
	else
		dx = m_nHiddenWidth;
	
	SetWindowPos( m_pImageDialog, x,y, dx, dy, 0 );
}

// *************************************************************************

void  CImageDialogTools::ParentIsShowing( bool bShowing )
{
	if ( bShowing )
		ShowWindow(SW_SHOW);
	else
		ShowWindow(SW_HIDE);
}

// *************************************************************************

void CImageDialogTools::AddImage( CImage * img )
{
	CString str;
	int		count;
	int		index;

	count = m_lImagesList.GetCount();
	str.Format("Image %d", count+1 );

	index = m_lImagesList.AddString( str );
	m_lImagesList.SetItemDataPtr( index, (void*)img );
	m_lImagesList.SetCurSel( index );
}

// *************************************************************************

void CImageDialogTools::ClearImages()
{
	m_lImagesList.ResetContent();
	m_pImageDialog->Invalidate();
}

// *************************************************************************

BOOL CImageDialogTools::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// calculate full sizes
	RECT rect;
	GetWindowRect( &rect );
	m_nFullWidth = rect.right - rect.left;

	// calculate hidden size
	RECT buttonrect;
	m_pbShowHideButton.GetWindowRect( &buttonrect );
	m_nHiddenWidth = 2*(buttonrect.left - rect.left)		// The part from the border to the button; on both sides
				+	 (buttonrect.right - buttonrect.left);	// The button...


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// *************************************************************************

void CImageDialogTools::IssueCommand( ImageDialogToolCommand cmd, unsigned int unParam )
{
	UpdateData(true);

	if (m_bLinked)
	{
		CImageDialogManager::Instance()->BroadcastToolCommand( cmd, unParam );
	}
	else
	{
		ExecCommand( cmd, unParam );
	}
}

// *************************************************************************

void CImageDialogTools::ExecCommand( ImageDialogToolCommand cmd, unsigned int unParam )
{
	int			index;

	// Get the current index
	index = m_lImagesList.GetCurSel();

	// Switch for each command.
	switch (cmd)
	{
	case CMD_LINK:
		m_bLinked = ( unParam != 0) ? true : false;
		UpdateData( false );
		break;

	case CMD_GOTO:
		m_lImagesList.SetCurSel( unParam );
		ShowSelectedImage();
		break;

	case CMD_REWIND:
		m_lImagesList.SetCurSel( 0 );	
		ShowSelectedImage();
		break;

	case CMD_FRAMEBACK:
		if ( (index != 0) && (index != LB_ERR) )
		{
			m_lImagesList.SetCurSel( index - 1 );	
			ShowSelectedImage();
		}
		break;

	case CMD_FRAMENEXT:
		if ( ( index !=  m_lImagesList.GetCount()-1 ) && (index != LB_ERR) )
		{
			m_lImagesList.SetCurSel( index + 1 );	
			ShowSelectedImage();
		}
		break;

	case CMD_TOEND:
		m_lImagesList.SetCurSel( m_lImagesList.GetCount() - 1 );	
		ShowSelectedImage();
		break;

	case CMD_CLEAR:
		m_pImageDialog->ClearImages();
		ClearImages();
		break;	
	}
}


// *************************************************************************

void CImageDialogTools::ShowSelectedImage()
{
	int			index;
	CImage *	img;

	index = m_lImagesList.GetCurSel();

	if ( index == LB_ERR )
		return;

	// Get the image reference and set it at the parent
	img = (CImage*)m_lImagesList.GetItemDataPtr( index );
	m_pImageDialog->ChoseCurrentImage( img );
}

// *************************************************************************

void CImageDialogTools::OnSelchangeImageslist() 
{
	UpdateData();
	IssueCommand( CMD_GOTO, m_lImagesList.GetCurSel() );
}

// *************************************************************************

void CImageDialogTools::OnRewind() 
{
	IssueCommand( CMD_REWIND );
}

// *************************************************************************

void CImageDialogTools::OnFastforward() 
{
	// TODO::
	OnSelchangeImageslist();
}

// *************************************************************************

void CImageDialogTools::OnFrameback() 
{
	IssueCommand( CMD_FRAMEBACK );
}

// *************************************************************************

void CImageDialogTools::OnFramenext() 
{
	IssueCommand( CMD_FRAMENEXT );
}

// *************************************************************************

void CImageDialogTools::OnPlay() 
{
	// TODO: Add your control notification handler code here
	OnSelchangeImageslist();
}

// *************************************************************************

void CImageDialogTools::OnStop() 
{
	// TODO: Add your control notification handler code here
	OnSelchangeImageslist();
}

// *************************************************************************

void CImageDialogTools::OnToEnd() 
{
	IssueCommand( CMD_TOEND );
}

// *************************************************************************

void CImageDialogTools::OnClearbutton() 
{
	IssueCommand( CMD_CLEAR );
}

// *************************************************************************

void CImageDialogTools::OnLinkcheck() 
{
	UpdateData();

	// Broadcast this command in any case
	CImageDialogManager::Instance()->BroadcastToolCommand( CMD_LINK, (m_bLinked ? 1 : 0) );	
}
