#if !defined(AFX_MACHINEMODEL_H__ADAFF8F3_8F24_4EBF_8F9F_070350C3247E__INCLUDED_)
#define AFX_MACHINEMODEL_H__ADAFF8F3_8F24_4EBF_8F9F_070350C3247E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MachineModel.h : header file
//

#include "stdafx.h"
#include <map>
#include "Property.h"
#include "ModeProperty.h"
#include "VisionProperty.h"
#include "DSPComm.h"
#include "UserMessages.h"
#include "CountsProperty.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CMachineModel thread

class CMachineModel : public CWinThread
{
	DECLARE_DYNCREATE(CMachineModel)
public:
	CMachineModel();           // protected constructor used by dynamic creation
	virtual ~CMachineModel();  // destructor


// Attributes
public:

// Operations
public:
	void CMachineModel::connectGuiModelThread( CWnd* guiThread );

	CDSPComm* submitDSPCommP();

	CDSPComm* m_DSPComm;


	bool isFinished;

	bool setNewAddr(CString ipAddrArg, int valueArg);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMachineModel)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run(); //override Run function
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWnd * _DemoSorterGUIThread;
	
	afx_msg void onSetMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void onGetMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void onQuit(WPARAM wParam, LPARAM lParam);
	afx_msg void onResetMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void onCountsMessage(WPARAM wParam, LPARAM lParam);


	// Generated message map functions
	//{{AFX_MSG(CMachineModel)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	map<CString, CProperty*> propertyList;

	CCounts* counts;

	CProperty* pColor0Hue;
	CProperty* pColor0Count;
	CProperty* pColor0Eject;
	
	CProperty* pColor1Hue;
	CProperty* pColor1Count;
	CProperty* pColor1Eject;
	
	CProperty* pColor2Hue;
	CProperty* pColor2Count;
	CProperty* pColor2Eject;

	CProperty* pColor3Hue;
	CProperty* pColor3Count;
	CProperty* pColor3Eject;

	//CProperty* pColor4Hue;
	//CProperty* pColor4Count;
	//CProperty* pColor4Eject;

	//CProperty* pColor5Hue;
	//CProperty* pColor5Count;
	//CProperty* pColor5Eject;

	//CProperty* pColor6Hue;
	//CProperty* pColor6Count;
	//CProperty* pColor6Eject;

	//CProperty* pColor7Hue;
	//CProperty* pColor7Count;
	//CProperty* pColor7Eject;

	CProperty* pMode;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACHINEMODEL_H__ADAFF8F3_8F24_4EBF_8F9F_070350C3247E__INCLUDED_)
