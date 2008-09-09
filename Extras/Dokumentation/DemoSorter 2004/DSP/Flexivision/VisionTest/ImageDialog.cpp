// ImageDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VisionTest.h"
#include "ImageDialog.h"
#include "CxImage\CxImage\ximage.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ImageDialogManager.h"

/////////////////////////////////////////////////////////////////////////////
// CImageDialog dialog

// *************************************************************************

CImageDialog::CImageDialog( CString strTitle, CWnd* pParent /*=NULL*/)
	: CDialog(CImageDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImageDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_strTitle = strTitle;
	m_pToolDialog = NULL;
	m_pImage = NULL;

	m_dZoom = 1.0;
	m_nImagePosX = 0;
	m_nImagePosY = 0;

	m_bAdjustingScrollbars = false;

	m_hIcon = AfxGetApp()->LoadIcon(IDI_IMAGEDIALOG);

	// Create the dialog
	Create( IDD, pParent );

	// Register it at the manager
	CImageDialogManager::Instance()->AddDialog(this);

	// Set the window title
	SetWindowText( m_strTitle );

	// Setup the tools dialog
	m_pToolDialog = new CImageDialogTools( this );
	m_bToolsShown = false;
	m_pToolDialog->ShowWindow( SW_HIDE );
	m_pToolDialog->AdjustPosition();
}

// *************************************************************************

CImageDialog::~CImageDialog()
{
	CImageDialogManager::Instance()->RemoveDialog( this );
	/*
	if ( m_pImage != NULL )
		delete m_pImage;
*/

	delete m_pToolDialog;

	ClearImages();
}

// *************************************************************************

BOOL CImageDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// *************************************************************************

void CImageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

}
// *************************************************************************


BEGIN_MESSAGE_MAP(CImageDialog, CDialog)
	//{{AFX_MSG_MAP(CImageDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDialog message handlers

void CImageDialog::OnPaint() 
{
	CPaintDC dc(this); 

	// Don't paint if we've got no image
	if ( m_pImage == NULL )
		return;

	unsigned int zoom_w, zoom_h;

	zoom_w = (unsigned int)((double)m_pImage->GetWidth() * m_dZoom);
	zoom_h = (unsigned int)((double)m_pImage->GetHeight() * m_dZoom);

	if (m_dZoom == 1.0)
		//ima->Draw(dc->GetSafeHdc());
		m_pImage->Draw(dc, -m_nImagePosX, -m_nImagePosY );
	else
		m_pImage->Draw(/*pDC->GetSafeHdc()*/dc, 
						CRect(	-m_nImagePosX, -m_nImagePosY, 
								zoom_w - m_nImagePosX, zoom_h - m_nImagePosY )
						);
}

// *************************************************************************

void CImageDialog::LoadImage( CString strFile )
{
	bool res;

	// Create window if necessary
	if ( m_pImage == NULL )
		m_pImage = new CImage();
	
	res = m_pImage->Load( strFile );	

	SetZoomFactor( 1.0 );
	FitWindowToImageSize();
}

// *************************************************************************
	
void CImageDialog::SetImage( CImage image )
{
	ClearImages();

	// Create window if necessary
	if ( m_pImage == NULL )
		m_pImage = new CImage();

	m_pImage->Copy( image );
	
	SetZoomFactor( 1.0 );
	FitWindowToImageSize();
}

// *************************************************************************

void CImageDialog::AddImage( CImage image )
{
	// Create a new image and copy the data
	CImage * img;
	img = new CImage();
	img->Copy( image );

	// make this the current image 
	m_pImage = img;

	// If this is the first image to reach this dialog, adjust the zoom factor
	// and the dialog's size so that everything is looking fine.
	if ( m_lImages.IsEmpty() )
	{
	
		// Adjust zoom factor
		float zoom = 1;
		int width = m_pImage->GetWidth();
		int height = m_pImage->GetHeight();
		while ( (width > 800) || (height > 600) )
		{
			width /= 2;
			height /= 2;
			zoom /= 2;
		}
		SetZoomFactor( zoom );
		
		// Adjust the window size
		FitWindowToImageSize();
	}

	// Paint new image
	Invalidate( );

	// Add the pointer to the list
	m_lImages.AddTail( img );


	// Show the image in the tool dialog
	if ( m_pToolDialog != NULL)
		m_pToolDialog->AddImage( img );
}

// *************************************************************************

void CImageDialog::ChoseCurrentImage( CImage * image )
{
	POSITION pos;

	pos = m_lImages.Find( image );

	// If the image is not in the list, return doing nothing.
	if ( pos == NULL )
		return;

	// Since the image is in the list, we can just select it.
	m_pImage = image;
	Invalidate();
}

// *************************************************************************

void CImageDialog::ClearImages(  )
{
	m_pImage = NULL;

	while ( ! m_lImages.IsEmpty() )
	{
		CImage * img;
		img = m_lImages.GetHead();
		m_lImages.RemoveHead();
		delete img;
	}
}

// *************************************************************************

void CImageDialog::SetRGBImage( unsigned int unWidth, unsigned int unHeight, unsigned int * buffer )
{
}

// *************************************************************************

void CImageDialog::ShowTools( bool bShow )
{
	m_bToolsShown = bShow;
	m_pToolDialog->Show( bShow );
}

// *************************************************************************

void CImageDialog::ExecToolCommand( ImageDialogToolCommand cmd, unsigned int unParam )
{
	if ( m_pToolDialog != NULL )
		m_pToolDialog->ExecCommand( cmd, unParam );
}

// *************************************************************************

void CImageDialog::FitWindowToImageSize()
{
	RECT client;
	RECT window;

	// Abort if no window present
	if ( m_pImage == NULL )
		return;
	
	GetClientRect( &client );
	GetWindowRect( &window );

	int add_w = (int)((float)m_pImage->GetWidth() * m_dZoom) - client.right;
	int add_h = (int)((float)m_pImage->GetHeight() * m_dZoom) - client.bottom;

	SetWindowPos( NULL, 0, 0, 
			window.right - window.left + add_w,
			window.bottom - window.top + add_h,
			SWP_NOMOVE | SWP_NOOWNERZORDER );

	// Move the image to top-left
	m_nImagePosX = 0;
	m_nImagePosY = 0;

	AdjustScrollbars();
}

// *************************************************************************

void CImageDialog::SetZoomFactor( double dZoom )
{
	DoZoom(m_nImagePosX, m_nImagePosY, dZoom);
}

// *************************************************************************

void CImageDialog::ZoomIn( int x, int y )
{
	DoZoom( x, y, m_dZoom * 2.0);
}

// *************************************************************************

void CImageDialog::ZoomOut( int x, int y )
{
	DoZoom( x, y, m_dZoom / 2.0);
}

// *************************************************************************
	
void CImageDialog::CenterPoint( int x, int y )
{
	RECT client;

	// Get the client area
	GetClientRect( &client );

	// Move the selected spot to the center of the window
	MoveImage( x - client.right/2, y - client.bottom/2 );	
}

// *************************************************************************	

void CImageDialog::MoveImage( int x, int y )
{
	RECT client;

	// Abort if no window present
	if ( m_pImage == NULL )
		return;

	// Get the client area
	GetClientRect( &client );

	m_nImagePosX = x;
	m_nImagePosY = y;	

	// Don't let the image move out of view, if the viewport is smaller than the image
	// and check if the image is smaller than the window, in which case the image must
	// be centered. This must be done whith each axis.
	int zoom_w, zoom_h;
	bool bChanged = false;
	zoom_w = (int)((double)m_pImage->GetWidth() * m_dZoom );
	zoom_h = (int)((double)m_pImage->GetHeight() * m_dZoom );

	if ( client.right > zoom_w )
	{
		m_nImagePosX =  zoom_w/2 - client.right/2;
	}
	else
	{
		if ( m_nImagePosX < 0 )
			m_nImagePosX = 0;

		if ( m_nImagePosX + client.right >  zoom_w )
			m_nImagePosX = zoom_w - client.right;
	}

	if ( client.bottom > zoom_h )
	{
		m_nImagePosY =  zoom_h/2 - client.bottom/2;
	}
	else
	{
		if ( m_nImagePosY < 0 )
			m_nImagePosY = 0;

		if ( m_nImagePosY + client.bottom > zoom_h )
			m_nImagePosY = zoom_h - client.bottom;
	}

	AdjustScrollbars();
	Invalidate();	
}
// *************************************************************************	

void CImageDialog::AdjustImagePosition()
{
	MoveImage( m_nImagePosX, m_nImagePosY );
}

// *************************************************************************	

void CImageDialog::DoZoom( int x, int y, double zoom )
{
	RECT client;
	GetClientRect( &client );

	int new_x, new_y;

	// First, calculate the point to center after the zoom step
	new_x = (int)( (double)(x + m_nImagePosX) / m_dZoom * zoom );
	new_y = (int)( (double)(y + m_nImagePosY) / m_dZoom * zoom );
	
	// the, adjust the zoom
	m_dZoom = zoom;

	if ( m_dZoom > 1)
		m_dZoom = ceil( m_dZoom );
	else
		m_dZoom = 1 / ceil( 1 / m_dZoom );

	// and center the point which will also redraw the window and adjust the scrolls.
	CenterPoint( new_x, new_y );

	// Update the window title
	CString str;
	str.Format("%s - %.2f%%", m_strTitle, m_dZoom*100.0 );
	SetWindowText( str );
}

// *************************************************************************

void CImageDialog::AdjustScrollbars()
{
	// Abort if no window present
	if ( m_pImage == NULL )
	{
		// Hide scrollbars and return
		ShowScrollBar( SB_HORZ, false );
		ShowScrollBar( SB_VERT, false );
		return;
	}

	RECT client;

	GetClientRect( &client );

	// Break recursion, since the following action will trigger a RESIZE message, which in turn
	// will call AdjustScrollbars.
	m_bAdjustingScrollbars = true;

	SCROLLINFO info;

	info.cbSize = sizeof(SCROLLINFO);     
	info.fMask = SIF_ALL;     
	info.nMin = 0;     
	info.nMax = (int)((double)m_pImage->GetWidth() * m_dZoom); 
	//info.nPage = min( client.right, info.nMax );
	info.nPage = client.right;
	info.nPos = m_nImagePosX;    
	info.nTrackPos = m_nImagePosX; 
	SetScrollInfo( SB_HORZ, &info, true );

	info.cbSize = sizeof(SCROLLINFO);     
	info.fMask = SIF_ALL;     
	info.nMin = 0;     
	info.nMax = (int)((double)m_pImage->GetHeight() * m_dZoom); 
	//info.nPage = min( client.bottom, info.nMax );
	info.nPage = client.bottom;
	info.nPos = m_nImagePosY;    
	info.nTrackPos = m_nImagePosY; 
	SetScrollInfo( SB_VERT, &info, true );

	m_bAdjustingScrollbars = false;
}

// *************************************************************************

void CImageDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ZoomIn( point.x, point.y);
	CDialog::OnLButtonDown(nFlags, point);
}

// *************************************************************************

void CImageDialog::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ZoomOut( point.x, point.y );
	CDialog::OnRButtonDown(nFlags, point);
}

// *************************************************************************

void CImageDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if ( ! m_bAdjustingScrollbars )
		AdjustScrollbars();

	// Adjust the image position
	AdjustImagePosition( );	

	// Move the tools
	if ( m_pToolDialog != NULL)
		m_pToolDialog->AdjustPosition();
}

// *************************************************************************

void CImageDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// Get Scroll info
	SCROLLINFO info;
	GetScrollInfo( SB_HORZ, &info, SIF_ALL );

	RECT client;
	GetClientRect( &client );

	int nPrevPos = info.nPos;
	int nTrackPos = info.nTrackPos;
	int nCurPos = nPrevPos;

	// decide what to do for each diffrent scroll event
	switch(nSBCode)
	{

		case SB_LEFT:
		case SB_RIGHT:	
			ASSERT(0);
			break;

		case SB_LINELEFT:
			nCurPos = nPrevPos - client.right/10;
			break;

		case SB_LINERIGHT:		
			nCurPos = nPrevPos + client.right/10;
			break;

		case SB_PAGELEFT:	
			nCurPos = nPrevPos - client.right;
			break;

		case SB_PAGERIGHT:		
			nCurPos = nPrevPos + client.right;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:  
			nCurPos = nTrackPos;
			break;
	}	

	// Set the new scroll position at the scrollbar's
	SetScrollPos(SB_HORZ, nCurPos);
	
	// Calculate the image's position
	m_nImagePosX = nCurPos;

	// Scroll the window content
	ScrollWindow(nPrevPos - nCurPos, 0) ;
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// *************************************************************************

void CImageDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// Get Scroll info
	SCROLLINFO info;
	GetScrollInfo( SB_VERT, &info, SIF_ALL );

	RECT client;
	GetClientRect( &client );

	int nPrevPos = info.nPos;
	int nTrackPos = info.nTrackPos;
	int nCurPos = nPrevPos;

	// decide what to do for each diffrent scroll event
	switch(nSBCode)
	{

		case SB_LEFT:
		case SB_RIGHT:	
			ASSERT(0);
			break;

		case SB_LINELEFT:
			nCurPos = nPrevPos - client.bottom/10;
			break;

		case SB_LINERIGHT:		
			nCurPos = nPrevPos + client.bottom/10;
			break;

		case SB_PAGELEFT:	
			nCurPos = nPrevPos - client.bottom;
			break;

		case SB_PAGERIGHT:		
			nCurPos = nPrevPos + client.bottom;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:  
			nCurPos = nTrackPos;
			break;
	}	

	// Set the new scroll position at the scrollbar's
	SetScrollPos(SB_VERT, nCurPos);
	
	// Calculate the image's position
	m_nImagePosY = nCurPos;

	// Scroll the window content
	ScrollWindow(0, nPrevPos - nCurPos) ;
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// *************************************************************************

// *************************************************************************

// *************************************************************************

void CImageDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDialog::PostNcDestroy();

}

void CImageDialog::OnClose() 
{
	// Hide toolbox
	if ( m_pToolDialog != NULL)
		m_pToolDialog->ParentIsShowing( false );

	CDialog::OnClose();
}


void CImageDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	if ( m_pToolDialog != NULL)
		m_pToolDialog->AdjustPosition();
	
}

void CImageDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if ( m_pToolDialog != NULL)
		m_pToolDialog->ParentIsShowing( bShow != 0 );	
}

void CImageDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: implement a tooltip color picker
	CDialog::OnMouseMove(nFlags, point);
}
