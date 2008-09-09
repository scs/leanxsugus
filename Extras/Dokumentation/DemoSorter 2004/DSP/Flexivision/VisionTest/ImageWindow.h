#if !defined(AFX_IMAGEWINDOW_H__E6679F21_E6AB_4E82_9129_EF954E94FB50__INCLUDED_)
#define AFX_IMAGEWINDOW_H__E6679F21_E6AB_4E82_9129_EF954E94FB50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageWindow window

class CImageWindow : public CWnd
{
// Construction
public:
	CImageWindow( CWnd * parent );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageWindow();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImageWindow)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEWINDOW_H__E6679F21_E6AB_4E82_9129_EF954E94FB50__INCLUDED_)
