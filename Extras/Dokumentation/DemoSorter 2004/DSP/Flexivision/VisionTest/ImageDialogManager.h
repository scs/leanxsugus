// ImageDialogManager.h: interface for the CImageDialogManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEDIALOGMANAGER_H__63F0B6FE_DF6A_4376_A9BA_552073411016__INCLUDED_)
#define AFX_IMAGEDIALOGMANAGER_H__63F0B6FE_DF6A_4376_A9BA_552073411016__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

#include "ImageDialog.h"
#include "ImageDialogTools.h"

class CImageDialogManager  
{
protected:
									CImageDialogManager();

public:
	virtual							~CImageDialogManager();

	static CImageDialogManager *	Instance();

	void							AddDialog( CImageDialog * dlg );
	void							RemoveDialog( CImageDialog * dlg );

	void							DeleteAllDialogs();
	void							HideAllDialogs();
	void							ShowAllDialogs();

	void							BroadcastToolCommand( ImageDialogToolCommand cmd, unsigned int unParam );


protected:
	static CImageDialogManager *	sTheInstance;

	CList<CImageDialog*, CImageDialog*&>		m_lDialogs;

};

#endif // !defined(AFX_IMAGEDIALOGMANAGER_H__63F0B6FE_DF6A_4376_A9BA_552073411016__INCLUDED_)
