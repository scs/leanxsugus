#if !defined(AFX_IMAGEDIALOGTOOLS_H__9C630E76_CA88_4714_AE91_C64C1F520BC2__INCLUDED_)
#define AFX_IMAGEDIALOGTOOLS_H__9C630E76_CA88_4714_AE91_C64C1F520BC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageDialogTools.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageDialogTools dialog

class CImageDialogTools;

enum ImageDialogToolCommand {
	CMD_LINK,
	CMD_GOTO,
	CMD_REWIND,
	CMD_FRAMEBACK,
	CMD_FRAMENEXT,
	CMD_TOEND,
	CMD_CLEAR
};


#include "Image.h"
#include "ImageDialog.h"



class CImageDialogTools : public CDialog
{
// Construction
public:
	CImageDialogTools(CImageDialog * pParent );   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImageDialogTools)
	enum { IDD = IDD_IMAGEDIALOGTOOLS };
	CButton	m_pbShowHideButton;
	CListBox	m_lImagesList;
	BOOL	m_bLinked;
	//}}AFX_DATA

	/**
	* Either show or hides the tools dialog. Hiding it means that it is compacted
	* in size so that only the expand button is visible.
	*/
	void						Show( bool bShow );

	/**
	* Returns whether the tool dialog is being shown in full size or not.
	*/
	bool						IsShowing();

	/**
	* Adjusts the tool dialog's position according to its parent. This function
	* has to be called by the parent whenever it is changeing size or position.
	*/
	void						AdjustPosition();

	/**
	* Completely hides the tools dialog. This function has to be called whenever
	* the parent window is being hidden or shown.
	*/
	void						ParentIsShowing( bool bShowing );

	/**
	* Adds an image to the tools dialog's image list.
	*/
	void						AddImage( CImage * img );

	/**
	* Clears the list of images and deletes them.
	*/
	void						ClearImages();

	void						IssueCommand( ImageDialogToolCommand cmd, unsigned int unParam = 0 );
	void						ExecCommand( ImageDialogToolCommand cmd, unsigned int unParam = 0);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageDialogTools)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	


// Implementation
protected:
	void						ShowSelectedImage();

	CImageDialog *				m_pImageDialog;

	bool						m_bIsShowing;

	int							m_nFullWidth;
	int							m_nHiddenWidth;

	// Generated message map functions
	//{{AFX_MSG(CImageDialogTools)
	afx_msg void OnShowhidebutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeImageslist();
	afx_msg void OnRewind();
	afx_msg void OnFastforward();
	afx_msg void OnFrameback();
	afx_msg void OnFramenext();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnToEnd();
	afx_msg void OnClearbutton();
	afx_msg void OnLinkcheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEDIALOGTOOLS_H__9C630E76_CA88_4714_AE91_C64C1F520BC2__INCLUDED_)
