// ImageDialogManager.cpp: implementation of the CImageDialogManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisionTest.h"
#include "ImageDialogManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CImageDialogManager * CImageDialogManager::sTheInstance = NULL;

// *************************************************************************

CImageDialogManager::CImageDialogManager()
{

}

// *************************************************************************

CImageDialogManager::~CImageDialogManager()
{
	DeleteAllDialogs();

	sTheInstance = NULL;
}

// *************************************************************************

CImageDialogManager * CImageDialogManager::Instance()
{
	if ( sTheInstance == NULL )
		sTheInstance = new CImageDialogManager();

	return sTheInstance;
}

// *************************************************************************

void CImageDialogManager::AddDialog( CImageDialog * dlg )
{
	m_lDialogs.AddTail( dlg );
}

// *************************************************************************

void CImageDialogManager::RemoveDialog( CImageDialog * dlg )
{
	POSITION pos;

	// Try to find the dialog
	pos = m_lDialogs.Find( dlg );

	if ( pos == NULL)
		return;

	// Remove it
	m_lDialogs.RemoveAt( pos );
}

// *************************************************************************

void CImageDialogManager::DeleteAllDialogs()
{
	while ( ! m_lDialogs.IsEmpty() )
	{
		CImageDialog * dlg;

		dlg = m_lDialogs.GetHead();
		m_lDialogs.RemoveHead();
		dlg->DestroyWindow();
		delete dlg;		
	}	
}

// *************************************************************************

void CImageDialogManager::HideAllDialogs()
{
	POSITION pos = m_lDialogs.GetHeadPosition();

	for (int i=0;i < m_lDialogs.GetCount();i++)
	{
		CImageDialog * dlg;
		dlg = m_lDialogs.GetNext(pos);

		dlg->ShowWindow( SW_HIDE );
	}
}

// *************************************************************************

void CImageDialogManager::ShowAllDialogs()
{
	POSITION pos = m_lDialogs.GetHeadPosition();

	for (int i=0;i < m_lDialogs.GetCount();i++)
	{
		CImageDialog * dlg;
		dlg = m_lDialogs.GetNext(pos);

		dlg->ShowWindow( SW_SHOW );
	}
}
	
// *************************************************************************

void CImageDialogManager::BroadcastToolCommand( ImageDialogToolCommand cmd, unsigned int unParam )
{
	POSITION pos = m_lDialogs.GetHeadPosition();

	for (int i=0;i < m_lDialogs.GetCount();i++)
	{
		CImageDialog * dlg;
		dlg = m_lDialogs.GetNext(pos);

		dlg->ExecToolCommand( cmd, unParam );
	}
}

// *************************************************************************

// *************************************************************************

// *************************************************************************
