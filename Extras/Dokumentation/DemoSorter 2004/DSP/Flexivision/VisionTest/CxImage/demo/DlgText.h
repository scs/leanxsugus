#if !defined(AFX_DLGTEXT_H__1FEA8102_CF4A_11D6_BB84_8B1C37831B77__INCLUDED_)
#define AFX_DLGTEXT_H__1FEA8102_CF4A_11D6_BB84_8B1C37831B77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgText.h : header file
//

#include "xTargetButton.h"

/////////////////////////////////////////////////////////////////////////////
// DlgText dialog
class DlgText : public CDialog
{
// Construction
public:
	DlgText(CWnd* pParent = NULL);   // standard constructor
	LOGFONT m_font;
	COLORREF m_color;
// Dialog Data
	//{{AFX_DATA(DlgText)
	enum { IDD = IDD_TEXT };
	CxTargetButton	m_ok;
	CxTargetButton	m_canc;
	CxTargetButton	m_bfont;
	CString	m_text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgText)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgText)
	virtual BOOL OnInitDialog();
	afx_msg void OnFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTEXT_H__1FEA8102_CF4A_11D6_BB84_8B1C37831B77__INCLUDED_)
