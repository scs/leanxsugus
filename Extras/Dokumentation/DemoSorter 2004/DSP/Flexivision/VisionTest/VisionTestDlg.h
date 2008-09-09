// VisionTestDlg.h : header file
//

#if !defined(AFX_VISIONTESTDLG_H__63285309_FB2C_4910_9CA4_CC52BD0DB1F0__INCLUDED_)
#define AFX_VISIONTESTDLG_H__63285309_FB2C_4910_9CA4_CC52BD0DB1F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "ImageDialog.h"
#include "../classVision.h"
#include "../vision/classVisObjectManager.h"

/////////////////////////////////////////////////////////////////////////////
// CVisionTestDlg dialog

class CVisionTestDlg : public CDialog
{
	enum {
		MAX_CHANNELS = 4
	};
// Construction
public:
											CVisionTestDlg(CWnd* pParent = NULL);	// standard constructor
											~CVisionTestDlg();

	void									ProcessImage( CString strFile );

// Dialog Data
	//{{AFX_DATA(CVisionTestDlg)
	enum { IDD = IDD_VISIONTEST_DIALOG };
	CListBox	m_lChannelPortList;
	CListBox	m_lChannelList;
	CListBox	m_lPropertyList;
	CListBox	m_lPortList;
	CListBox	m_lComponentList;
	BOOL	m_chkChannelEnable;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVisionTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strLastCalibrationFilename;
	/**
	* Clears the list of image files to process.
	*/
	void									ClearFiles();

	/**
	* Adds a file to the image file list.
	*/
	void									AddFile( CString strFile );

	void									UpdateComponentList();
	void									UpdatePortList();
	void									UpdatePropertyList();
	void									UpdateChannelPortList();

	void									SetupViewport( int channel );
	void									ShowViewports();

	/** THE vision object. */
	CVision *								m_pVision;

	HICON m_hIcon;

	/** The list of input files. */
	CList< CString, CString& >				m_lImagefiles;

	struct ChannelData {
		BOOL			bEnabled;
		unsigned int	unWidth;
		unsigned int	unHeight;
		unsigned int	unBpp;
		bool			bIndexed;
		CString			strComponentName;
		CString			strPortName;
		BYTE *			pBuffer;
		CImageDialog *	pImageDialog;
	};

	ChannelData								m_Channels[MAX_CHANNELS];

	// Generated message map functions
	//{{AFX_MSG(CVisionTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoadimages();
	afx_msg void OnProcess();
	afx_msg void OnLoadimagesequence();
	afx_msg void OnSelchangeComponentlist();
	afx_msg void OnChannelEnable();
	afx_msg void OnSelchangeChannellist();
	afx_msg void OnSelchangeChannelportlist();
	afx_msg void OnDblclkPropertylist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISIONTESTDLG_H__63285309_FB2C_4910_9CA4_CC52BD0DB1F0__INCLUDED_)
