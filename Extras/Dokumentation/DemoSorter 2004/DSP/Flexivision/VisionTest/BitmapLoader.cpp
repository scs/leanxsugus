// BitmapLoader.cpp: implementation of the CBitmapLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisionTest.h"
#include "BitmapLoader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
     
// *************************************************************************

CBitmapLoader::CBitmapLoader()
{
	m_pBitmap = new CBitmap();
	m_pImage = NULL;
	m_strLastFileName = "";
	m_pCurrentLine = NULL;
}
     
// *************************************************************************

CBitmapLoader::~CBitmapLoader()
{
	delete m_pBitmap;

	if (m_pImage != NULL)
		delete [] m_pImage;

}
     
// *************************************************************************

bool CBitmapLoader::LoadBitmapDialog()
{
	// Dialog for choosing a bitmap
	CFileDialog ldFile(TRUE, ".bmp", m_strLastFileName, NULL, "Bitmap|*.bmp||");
	if (ldFile.DoModal() == IDOK)
	{		
		// Get the filename selected
		CString sPath = ldFile.GetPathName();
		
		return LoadBitmap( sPath );
	}	
	else
		return FALSE;
}
     
// *************************************************************************

bool CBitmapLoader::LoadBitmap(CString filename)
{
	bool bSuccessful = TRUE;
	
	// Load bitmap
	HBITMAP hBitmap = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(),
											filename, IMAGE_BITMAP, 0,0,
											LR_LOADFROMFILE);

	// Do we have a valid handle for the loaded image?
	if (hBitmap)
	{
		// Delete the current bitmap
		if (m_pBitmap->DeleteObject())
		{
			// If there was a bitmap, detach it
			m_pBitmap->Detach();
		}
		// Attach the currently loaded bitmap to the bitmap object
		m_pBitmap->Attach(hBitmap);
	}
	else
	{
		MessageBox(NULL, "There was a problem loading the image!", NULL, MB_ICONERROR);
		bSuccessful = FALSE;
	}

	ExtractImageFromBmp();
	
	return bSuccessful;
}

Uint8 * CBitmapLoader::GetNextLine()
{
	Uint32	width, height;
	Uint32	totalsize;
	Uint8 * res;
	
	GetImageSize( width, height );
	totalsize = width*height;
	
	if ( m_pCurrentLine == NULL )
		m_pCurrentLine = m_pImage;

	if  ( m_pCurrentLine >= (m_pImage + totalsize) )
		return NULL;

	res = m_pCurrentLine;

	m_pCurrentLine += width;

	return res;
}
     
// *************************************************************************

void CBitmapLoader::ResetLine()
{
	m_pCurrentLine = NULL;
}
     
// *************************************************************************

Uint8 * CBitmapLoader::GetImage()
{
	return m_pImage;
}

     
// *************************************************************************

bool CBitmapLoader::GetImageSize(Uint32 & unXSize, Uint32 & unYSize)
// Get size of image
{
	BITMAP bm;
	// Get the loaded bitmap
	if (m_pBitmap->GetBitmap(&bm))
	{
		unXSize = bm.bmWidth;
		unYSize = bm.bmHeight;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
     
// *************************************************************************

void CBitmapLoader::ExtractImageFromBmp()
{
	BITMAP bm;
	Uint8 * temp;
	Uint8 c;

	// Get the loaded bitmap
	if ( m_pBitmap->GetBitmap(&bm) == 0)
		return;

	// Delete old buffer and create new one
	if (m_pImage != NULL)
		delete [] m_pImage;
	m_pImage = new Uint8[ bm.bmWidth * bm.bmHeight ];
	temp = new Uint8[bm.bmWidth * 4 * bm.bmHeight ];

	// Extract bits
	m_pBitmap->GetBitmapBits(bm.bmWidth * 4 * bm.bmHeight, temp );

	// Convert
	int i=0;
	for (i=0; i<bm.bmWidth * bm.bmHeight ; i++ )
	{
		c = temp[i*4];
		m_pImage[i] = c;
	}		
	
}
     
// *************************************************************************

void CBitmapLoader::UpdateBitmapBits(Uint8 *pBitmapBits)
// Update the Bitmap Bits and the View.
// Use this function if the size of the image does not change. The new Bitmap Bits
// are copied from the buffer pBitmapBits to the Bitmap.
{
	Uint32 unXSize, unYSize;
	GetImageSize(unXSize, unYSize);
	m_pBitmap->SetBitmapBits( unXSize * unYSize * 4/*BYTESPERPIXEL*/, pBitmapBits );
}
     
// *************************************************************************

bool CBitmapLoader::ConvertBitmap2GreyImage(Uint8 * pImage)
{
return true;
}
     
// *************************************************************************

bool CBitmapLoader::ConvertGreyImage2Bitmap(const Uint8 *pImage, Uint8 *pBitmapBits)
{
	Uint32 unXSize = 0;
	Uint32 unYSize = 0;

	GetImageSize(unXSize, unYSize);

	Uint32 i=0;

	for (i=0; i<unXSize * unYSize ; i++)
	{
		pBitmapBits[i*4]	=  pImage[i]; 
		pBitmapBits[i*4+1]	=  pImage[i]; 
		pBitmapBits[i*4+2]	=  pImage[i]; 
		pBitmapBits[i*4+3]	=  pImage[i]; 
	}

	return TRUE;
}
     
// *************************************************************************

