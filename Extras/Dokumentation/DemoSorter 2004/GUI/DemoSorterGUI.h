// DemoSorterGUI.h : main header file for the DEMOSORTERGUI application
//

#if !defined(AFX_DEMOSORTERGUI_H__8ABD9E3D_CA9C_454F_AD16_3C993F2C6C08__INCLUDED_)
#define AFX_DEMOSORTERGUI_H__8ABD9E3D_CA9C_454F_AD16_3C993F2C6C08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDemoSorterGUIApp:
// See DemoSorterGUI.cpp for the implementation of this class
//

class CDemoSorterGUIApp : public CWinApp
{
public:
	CDemoSorterGUIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoSorterGUIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDemoSorterGUIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOSORTERGUI_H__8ABD9E3D_CA9C_454F_AD16_3C993F2C6C08__INCLUDED_)
