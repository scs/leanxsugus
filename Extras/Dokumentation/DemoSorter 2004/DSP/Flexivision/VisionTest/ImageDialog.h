#if !defined(AFX_IMAGEDIALOG_H__8E12A33C_F9EE_4BDB_B3A5_C08A347D534E__INCLUDED_)
#define AFX_IMAGEDIALOG_H__8E12A33C_F9EE_4BDB_B3A5_C08A347D534E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CImageDialog;

#include <afxtempl.h>

class CImageDialog;

#include "Image.h"
#include "ImageDialogTools.h"

/////////////////////////////////////////////////////////////////////////////
// CImageDialog dialog

class CImageDialog : public CDialog
{
// Construction
public:
							CImageDialog(CString strTitle, CWnd* pParent = NULL);   // standard constructor
							~CImageDialog();

	/**
	* Loads an image from disk. The image's type is determined by its filename.
	*/
	void					LoadImage( CString strFile );

	/**
	* Copies the given image to the dialog. The data is fully copied so that the image
	* may be deleted afterwards.
	*/
	void					SetImage( CImage image );

	/**
	* Adds an image to the dialog. In contrast to SetImage(), this does not clear any of the previously
	* added images. Like in setImage(), the image's data is fully copied, though.
	*
	* Added images may then be displayed using the ChoseCurrentImage() function.
	*/
	void					AddImage( CImage image );

	/**
	* Displays one of the images in the dialog's list. If the image could not be found, nothing is done.
	*/
	void					ChoseCurrentImage( CImage * image );

	/**
	* Clears all added images.
	*/
	void					ClearImages( );
	

	void					SetRGBImage( unsigned int unWidth, unsigned int unHeight, unsigned int * buffer );

	void					ShowTools( bool bShow = TRUE ); 

	void					ExecToolCommand( ImageDialogToolCommand cmd, unsigned int unParam );

	/**
	* Fits the window's size to that of the image.
	*/
	void					FitWindowToImageSize();

	/**
	* Sets the zoom factor of the image.
	*/
	void					SetZoomFactor( double dZoom );

	/**
	* Zooms a certain point in. The zoom factor is doubled and the specified point is centered.
	*/
	void					ZoomIn( int x, int y );

	/**
	* Zooms a certain point out. The zoom factor is divided by 2 and the specified point is centered.
	*/
	void					ZoomOut( int x, int y );

protected:
	void					CenterPoint( int x, int y );
	void					MoveImage( int x, int y );
	void					AdjustImagePosition();
	void					DoZoom( int x, int y, double zoom );
	void					AdjustScrollbars();


// Dialog Data
	//{{AFX_DATA(CImageDialog)
	enum { IDD = IDD_IMAGEDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString					m_strTitle;
	CImage *				m_pImage;

	CImageDialogTools *		m_pToolDialog;
	bool					m_bToolsShown;

	int						m_nImagePosX;
	int						m_nImagePosY;

	double					m_dZoom;

	bool					m_bAdjustingScrollbars;

	HICON					m_hIcon;

	CList<CImage*, CImage*&>	m_lImages;

	// Generated message map functions
	//{{AFX_MSG(CImageDialog)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEDIALOG_H__8E12A33C_F9EE_4BDB_B3A5_C08A347D534E__INCLUDED_)
