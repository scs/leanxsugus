// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisionTest.h"
#include "Image.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// *************************************************************************

void CImage::CreateFromIndexed8BPP( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer )
{
	// First, create a pallette
    BYTE * aryPalletteRed = new BYTE[256];
    BYTE * aryPalletteGreen = new BYTE[256];
    BYTE * aryPalletteBlue = new BYTE[256];

	unsigned int i;

    for ( i=0; i<256; i++)
    {
      aryPalletteRed[i] = (BYTE)((i*77) & 0xFF);
      aryPalletteGreen[i] = (BYTE)(( i*19) & 0xFF);
      aryPalletteBlue[i] = (BYTE)(( i*155) & 0xFF);
    }

    // Set the first three color entries to show properly in blue, red and green.
    aryPalletteRed[1] =   0;
    aryPalletteGreen[1] = 0;
    aryPalletteBlue[1] =  (BYTE)255;

    aryPalletteRed[2] =   (BYTE)255;
    aryPalletteGreen[2] = 0;
    aryPalletteBlue[2] =  0;

    aryPalletteRed[3] =   0;
    aryPalletteGreen[3] = (BYTE)255;
    aryPalletteBlue[3] =  0;

	UINT * rgbBuffer = new UINT[ dwWidth * dwHeight];

	for ( i=0; i<dwWidth*dwHeight; i++)
	{
		rgbBuffer[i] = RGB( aryPalletteRed[pBuffer[i]],
							aryPalletteGreen[pBuffer[i]],
							aryPalletteBlue[pBuffer[i]] );
	}
	
	CreateFromRGB( dwWidth, dwHeight, (BYTE*)rgbBuffer );

	delete [] rgbBuffer;
	delete [] aryPalletteRed;
    delete [] aryPalletteGreen;
    delete [] aryPalletteBlue;
}

// *************************************************************************

void CImage::CreateFromGray( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer )
{
	BYTE * argbBuffer = new BYTE[ 4 * dwWidth * dwHeight];

	for ( unsigned int x=0; x<dwWidth; x++)
	{
		for ( unsigned int y=0; y<dwHeight; y++)
		{
			int i = x+(dwHeight-y-1)*dwWidth;
			int j = x+y*dwWidth;
			i=j;

			argbBuffer[i*4]		= pBuffer[j];
			argbBuffer[i*4+1]	= pBuffer[j];
			argbBuffer[i*4+2]	= pBuffer[j];
			argbBuffer[i*4+3]	= 255;
		}
	}

	CreateFromARGB( dwWidth, dwHeight, argbBuffer );

	delete [] argbBuffer;
}
// *************************************************************************

void CImage::CreateFromRGB( DWORD dwWidth, DWORD dwHeight, BYTE * pBuffer )
{
	BYTE * argbBuffer = new BYTE[ 4 * dwWidth * dwHeight];

	for ( unsigned int x=0; x<dwWidth; x++)
	{
		for ( unsigned int y=0; y<dwHeight; y++)
		{
			int i = x+(dwHeight-y-1)*dwWidth;
			int j = x+y*dwWidth;
			i=j;

			argbBuffer[i*4]		= pBuffer[j*4 + 2];		// b
			argbBuffer[i*4+1]	= pBuffer[j*4 + 1];		// g
			argbBuffer[i*4+2]	= pBuffer[j*4];			// r
			argbBuffer[i*4+3]	= 255;					// a
		}
	}

	CreateFromARGB( dwWidth, dwHeight, (BYTE*)argbBuffer );

	delete [] argbBuffer;
}

// *************************************************************************

void CImage::FillARGBBuffer( DWORD * argbBuffer )
{
	int height = GetHeight();

	for (unsigned int x=0; x<GetWidth(); x++)
	{
		for (unsigned int y=0; y<GetHeight(); y++)
		{
			DWORD rgb;
			RGBQUAD q = GetPixelColor( x,y );

			rgb = (DWORD)q.rgbBlue 
					| ((DWORD)q.rgbGreen << 8)
					| ((DWORD)q.rgbRed << 16);

			argbBuffer[ x + (height - y - 1) * GetWidth() ] = rgb;
		}
	}
}

// *************************************************************************

void CImage::FillRGBBuffer( DWORD * rgbBuffer )
{
	int height = GetHeight();

	for (unsigned int x=0; x<GetWidth(); x++)
	{
		for (unsigned int y=0; y<GetHeight(); y++)
		{
			DWORD rgb;
			RGBQUAD q = GetPixelColor( x,y );

			rgb = (DWORD)q.rgbRed
					| ((DWORD)q.rgbGreen << 8)
					| ((DWORD)q.rgbBlue << 16);

			rgbBuffer[ x + (height - y - 1)*GetWidth() ] = rgb;
		}
	}
}

// *************************************************************************

void CImage::FillGrayBuffer( BYTE * grayBuffer )
{
	int height = GetHeight();

	for (unsigned int x=0; x<GetWidth(); x++)
	{
		for (unsigned int y=0; y<GetHeight(); y++)
		{
			BYTE gray = GetPixelGray( x, y);

			grayBuffer[ x + (height - y - 1)*GetWidth() ] = gray;
		}
	}
}
	
// *************************************************************************

// *************************************************************************
