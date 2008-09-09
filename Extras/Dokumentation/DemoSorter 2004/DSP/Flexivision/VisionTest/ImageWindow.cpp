// ImageWindow.cpp : implementation file
//

#include "stdafx.h"
#include "VisionTest.h"
#include "ImageWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageWindow

CImageWindow::CImageWindow( CWnd * parent )
{
	Create( NULL, "Image", WS_CHILD | WS_VISIBLE, CRect(0,0, 100, 100), parent, 4567 );

//	CreateEx( WS_EX_CLIENTEDGE, NULL, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hwndParent, HMENU nIDorHMenu, LPVOID lpParam = NULL );
}


CImageWindow::~CImageWindow()
{
}


BEGIN_MESSAGE_MAP(CImageWindow, CWnd)
	//{{AFX_MSG_MAP(CImageWindow)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CImageWindow message handlers
