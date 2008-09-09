#if !defined(AFX_SERVICEDLG_H__86F24B07_9D86_4F19_A37F_90B36C3B6A19__INCLUDED_)
#define AFX_SERVICEDLG_H__86F24B07_9D86_4F19_A37F_90B36C3B6A19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ServiceDlg.h : header file
//

#include "MachineModel.h"
#include "DSPComm.h"
#include "UserMessages.h"

/////////////////////////////////////////////////////////////////////////////
// CServiceDlg dialog

class CServiceDlg : public CDialog
{
// Construction
public:
	CServiceDlg(CWnd* pParent = NULL);   // standard constructor
	
	void submitMachineP(CMachineModel * _MachineModelThread); 
	
	int mode;
	int modeSave;

	CDSPComm* commP;

// Dialog Data
	//{{AFX_DATA(CServiceDlg)
	enum { IDD = IDD_SERVICEDLG };
	CButton	m_serviceSel;
	CButton	m_processingSel;
	CButton	m_idleSel;
	CButton	m_calibrationSel;
	CEdit	m_debugText;
	CEdit	m_statusGUI;
	CEdit	m_port;
	CComboBox	m_ipAddr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServiceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString ipString, portString;

	CMachineModel* machineModel;

	// Generated message map functions
	//{{AFX_MSG(CServiceDlg)
	afx_msg void OnConnect();
	virtual BOOL OnInitDialog();
	afx_msg void OnIdlesel();
	afx_msg void OnProcessingsel();
	afx_msg void OnCalibrationsel();
	virtual void OnOK();
	afx_msg void OnServicesel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVICEDLG_H__86F24B07_9D86_4F19_A37F_90B36C3B6A19__INCLUDED_)
