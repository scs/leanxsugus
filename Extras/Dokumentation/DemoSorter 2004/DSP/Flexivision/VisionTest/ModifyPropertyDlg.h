#if !defined(AFX_MODIFYPROPERTYDLG_H__35A58AA9_862E_43AE_954F_A9F1875AD87A__INCLUDED_)
#define AFX_MODIFYPROPERTYDLG_H__35A58AA9_862E_43AE_954F_A9F1875AD87A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyPropertyDlg.h : header file
//

#include "../vision/classVisProperty.h"

class CModifyPropertyDlg : public CDialog
{
// Construction
public:
	CModifyPropertyDlg(CWnd* pParent, CVisProperty * prop );   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyPropertyDlg)
	enum { IDD = IDD_MODIFYPROPERTYDIALOG };
	CString	m_strPropertyEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyPropertyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVisProperty *			m_pProperty;

	// Generated message map functions
	//{{AFX_MSG(CModifyPropertyDlg)
	virtual void OnOK();
	afx_msg void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYPROPERTYDLG_H__35A58AA9_862E_43AE_954F_A9F1875AD87A__INCLUDED_)
