#if !defined(AFX_BITMAPDIALOG_H__8630EECB_9E01_4EED_964E_FC0719506C4A__INCLUDED_)
#define AFX_BITMAPDIALOG_H__8630EECB_9E01_4EED_964E_FC0719506C4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BitmapDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBitmapDialog dialog

#include "Types.h"

class CBitmapDialog : public CDialog
{
// Construction
public:
	CBitmapDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBitmapDialog)
	enum { IDD = IDD_BITMAPDIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	void			SetImage( Uint8 * image, int width, int height );
	void			SetSizeOfWindow(int nXPos, int nYPos);
	void			ShowBitmap( CPaintDC * pdc);

	virtual BOOL	Create(CWnd* pParentWnd, CString title, Uint8 * image, int width, int height);
protected:

	// Generated message map functions
	//{{AFX_MSG(CBitmapDialog)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap *		m_pBitmap;

	static int		lastX;
	static int		lastY;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPDIALOG_H__8630EECB_9E01_4EED_964E_FC0719506C4A__INCLUDED_)
