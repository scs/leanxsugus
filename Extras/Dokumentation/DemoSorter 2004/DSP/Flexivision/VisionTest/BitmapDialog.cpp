// BitmapDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VisionTest.h"
#include "BitmapDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapDialog dialog

int CBitmapDialog::lastX = 100;
int CBitmapDialog::lastY = 100;

CBitmapDialog::CBitmapDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBitmapDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBitmapDialog)
	//}}AFX_DATA_INIT

	m_pBitmap = NULL;
}
    
// *************************************************************************

void CBitmapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBitmapDialog)
	//}}AFX_DATA_MAP
}
    
// *************************************************************************

BEGIN_MESSAGE_MAP(CBitmapDialog, CDialog)
	//{{AFX_MSG_MAP(CBitmapDialog)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitmapDialog message handlers
    
// *************************************************************************

void CBitmapDialog::SetImage( Uint8 * image, int width, int height )
{
	Uint8 c;
	if (m_pBitmap != NULL)
		delete m_pBitmap;

	m_pBitmap = new CBitmap();
	m_pBitmap->CreateBitmap( width, height, 1, 32, NULL);

	CDC dc;
	dc.CreateCompatibleDC( this->GetDC() );

	dc.SelectObject( m_pBitmap );
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			c = image[x + y*width];
			dc.SetPixel( x,y, RGB(c,c,c) );
		}
	}
}
    
// *************************************************************************

void CBitmapDialog::SetSizeOfWindow(int nXPos, int nYPos)
// Set Window Size so that it fits to the dimension of the image
// The Position of the Window can be choosen by x and y
{
	BITMAP bm;

	if (m_pBitmap == NULL)
		return;

	// Get the loaded bitmap
	m_pBitmap->GetBitmap(&bm);

	CRect lRect,lRectDlg;
    LONG lDiffx, lDiffy;
	// Get Current Size of Window
	GetWindowRect(&lRectDlg);
	lRectDlg.NormalizeRect();
	// Get the current display area available
	GetClientRect(lRect);
	lRect.NormalizeRect();
    // Calculate size of Border
	lDiffx = lRectDlg.Width() - lRect.Width();
    lDiffy = lRectDlg.Height() - lRect.Height();

    // Resize Window so it has the dimension of the image
	lRectDlg.SetRect(nXPos, nYPos, nXPos + bm.bmWidth+lDiffx, nYPos+(bm.bmHeight)+lDiffy);
	MoveWindow(&lRectDlg, TRUE);
}
    
// *************************************************************************
    
// *************************************************************************
    
// *************************************************************************
    
// *************************************************************************
 
void CBitmapDialog::ShowBitmap( CPaintDC * pdc)
{
	// Only if an Object (Bitmap) is attached. It makes sure that there
	// is a Bitmap to show.
    if (m_pBitmap->GetSafeHandle( ))
	{
		BITMAP bm;
		// Get the loaded bitmap
		m_pBitmap->GetBitmap(&bm);
		CDC dcMem;
		// Create a device context to load the bitmap into 
		dcMem.CreateCompatibleDC(pdc);
		// Select the bitmap into the compatible device context
		CBitmap* pOldBitmap = (CBitmap*)dcMem.SelectObject(m_pBitmap);

		CRect lRect;
		// Get the display area available
		GetClientRect(lRect);
		lRect.NormalizeRect();
		
		// Copy the bitmap to the dialog window without resize the bitmap
//		 pdc->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY);

		// Copy and resize the bitmapto the dialog window
		pdc->StretchBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMem, 0, 0,
						 bm.bmWidth, bm.bmHeight, SRCCOPY);
	}
}
    
// *************************************************************************

BOOL CBitmapDialog::Create(CWnd* pParentWnd, CString title, Uint8 * image, int width, int height) 
{
	CDialog::Create(IDD, pParentWnd);

	this->SetWindowText(title);

	SetImage( image, width, height );
	SetSizeOfWindow( lastX, lastY );
	ShowWindow( SW_SHOW );

	lastX += 20;
	lastY += 30;
	
	return TRUE;
}
   
// *************************************************************************

void CBitmapDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	ShowBitmap( &dc );	

	// Do not call CDialog::OnPaint() for painting messages
}
