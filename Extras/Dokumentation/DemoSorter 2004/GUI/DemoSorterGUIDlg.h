// DemoSorterGUIDlg.h : header file 
//

#include "MachineModel.h"
#include "DSPComm.h"
#include "UserMessages.h"
#include "ServiceDlg.h"
#include "BmpButton.h"


#if !defined(AFX_DEMOSORTERGUIDLG_H__BEC7C416_0A4E_458F_AFBE_D33D3B1D5B8F__INCLUDED_)
#define AFX_DEMOSORTERGUIDLG_H__BEC7C416_0A4E_458F_AFBE_D33D3B1D5B8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDemoSorterGUIDlg dialog

class CDemoSorterGUIDlg : public CDialog
{
// Construction
public:
	CDemoSorterGUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDemoSorterGUIDlg)
	enum { IDD = IDD_DEMOSORTERGUI_DIALOG };
	CButton	m_reset;
	CButton	m_button1;
	CStatic	m_count3;
	CStatic	m_count2;
	CStatic	m_count1;
	CStatic	m_count0;
	CButton	m_timerSel;
	CBmpButton	m_resetButton;
	CBmpButton	m_color0Button;
	CBmpButton	m_color2Button;
	CBmpButton	m_color3Button;
	CBmpButton	m_color1Button;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoSorterGUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CServiceDlg serviceDlg;

	//bool serviceDlgOpen; //seems not necessary

	HICON m_hIcon;

	afx_msg void CDemoSorterGUIDlg::OnSetReplyMessage(WPARAM wParam, LPARAM info);

	afx_msg void CDemoSorterGUIDlg::OnGetReplyMessage(WPARAM wParam, LPARAM info);

	afx_msg void CDemoSorterGUIDlg::OnCountsReplyMessage(WPARAM wParam, LPARAM info);

	// Generated message map functions
	//{{AFX_MSG(CDemoSorterGUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnColor0();
	afx_msg void OnColor1();
	afx_msg void OnColor2();
	afx_msg void OnColor3();
	afx_msg void OnDestroy();
	afx_msg void OnReset();
	afx_msg void OnTimersel();
	virtual void OnOK();
	afx_msg void OnBtnReset();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CMachineModel * _MachineModelThread; //globaler Pointer auf Thread CMachineModel Objekt

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOSORTERGUIDLG_H__BEC7C416_0A4E_458F_AFBE_D33D3B1D5B8F__INCLUDED_)
