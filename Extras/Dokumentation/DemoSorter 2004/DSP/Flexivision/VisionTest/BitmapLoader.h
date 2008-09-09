// BitmapLoader.h: interface for the CBitmapLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPLOADER_H__4B6034AC_98F9_4734_9DA6_068512E3BE6C__INCLUDED_)
#define AFX_BITMAPLOADER_H__4B6034AC_98F9_4734_9DA6_068512E3BE6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Types.h"

class CBitmapLoader  
{
public:
				CBitmapLoader();
	virtual		~CBitmapLoader();

	bool		LoadBitmapDialog();
	bool		LoadBitmap(CString filename);

	CBitmap *	GetBitmap();

	Uint8 *		GetNextLine();
	void		ResetLine();

	Uint8 *		GetImage();

	bool		GetImageSize(Uint32 & unXSize, Uint32 & unYSize);
	
protected:
	void		ExtractImageFromBmp();
	void		UpdateBitmapBits(Uint8 *pBitmapBits);
	bool		ConvertBitmap2GreyImage(Uint8 * pImage);
	bool		ConvertGreyImage2Bitmap(const Uint8 *pImage, Uint8 *pBitmapBits);

	CBitmap *	m_pBitmap;
	Uint8 *		m_pImage;
	Uint8 *		m_pCurrentLine;
	CString		m_strLastFileName;

};

#endif // !defined(AFX_BITMAPLOADER_H__4B6034AC_98F9_4734_9DA6_068512E3BE6C__INCLUDED_)
