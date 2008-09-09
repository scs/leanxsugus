// Image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__6DEE95AF_3D67_4D17_A932_9030516EF143__INCLUDED_)
#define AFX_IMAGE_H__6DEE95AF_3D67_4D17_A932_9030516EF143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CxImage\CxImage\ximage.h"

class CImage : public CxImage  
{
public:
	CImage(DWORD imagetype = 0)													
		: CxImage(imagetype) {};

	CImage(DWORD dwWidth, DWORD dwHeight, DWORD wBpp, DWORD imagetype = 0)		
		: CxImage(dwWidth, dwHeight, wBpp, imagetype ) {};

	CImage(const CxImage &src, bool copypixels = true, bool copyselection = true, bool copyalpha = true)
		: CxImage(src, copypixels, copyselection, copyalpha ) {};

	CImage(const char * filename, DWORD imagetype)
		: CxImage(filename, imagetype) {};

	CImage(FILE * stream, DWORD imagetype)
		: CxImage(stream, imagetype) {};	

	CImage(CxFile * stream, DWORD imagetype)
		: CxImage(stream, imagetype) {};

	CImage(BYTE * buffer, DWORD size, DWORD imagetype)
		: CxImage(buffer, size, imagetype) {};

public:
	void						CreateFromIndexed8BPP( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer );
	void						CreateFromRGB( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer );
	void						CreateFromGray( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer );
	// void						CreateFromPalletized()

	void						FillARGBBuffer( DWORD * argbBuffer );
	void						FillRGBBuffer( DWORD * rgbBuffer );
	void						FillGrayBuffer( BYTE * grayBuffer );

};

#endif // !defined(AFX_IMAGE_H__6DEE95AF_3D67_4D17_A932_9030516EF143__INCLUDED_)
