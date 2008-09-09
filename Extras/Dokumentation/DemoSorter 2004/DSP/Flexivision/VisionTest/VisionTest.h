// VisionTest.h : main header file for the VISIONTEST application
//

#if !defined(AFX_VISIONTEST_H__EFCA223A_48C8_4878_8C13_B2CFB260A710__INCLUDED_)
#define AFX_VISIONTEST_H__EFCA223A_48C8_4878_8C13_B2CFB260A710__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVisionTestApp:
// See VisionTest.cpp for the implementation of this class
//

class CVisionTestApp : public CWinApp
{
public:
	CVisionTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVisionTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVisionTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISIONTEST_H__EFCA223A_48C8_4878_8C13_B2CFB260A710__INCLUDED_)
