
#include "classVisColorPick.h"
#include "classVisColor.h"

CVisColorPick::CVisColorPick( const char * strName, const Int nMaxNumObjects )
	:CVisComponent( "ColorPick", strName )
	,m_iportImage( "rgbInput", CVisPort::PDT_32BPP_RGB  )
	,m_iportLabelData( "objLabels", CVisPort::PDT_DATA )
	,m_oportColorData( "objColors", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST )
	,m_propWindowSize( "WindowSize" )
	,m_propMinLuminance( "MinLuminance" )
	,m_propMaxLuminance( "MaxLuminance" )	
{
	m_iportImage.Init( this );
	m_iportLabelData.Init( this );
	m_oportColorData.Init( this );

	
	m_propWindowSize.Init( this, CVisProperty::PT_INTEGER, &m_nWindowSize );
	m_nWindowSize = 3;

	m_propMinLuminance.Init( this, CVisProperty::PT_INTEGER, &m_nMinLuminance );
	m_nMinLuminance = 58;

	m_propMaxLuminance.Init( this, CVisProperty::PT_INTEGER, &m_nMaxLuminance );
	m_nMaxLuminance = 160;

	m_nMaxNumObjects = nMaxNumObjects;

	m_oportColorData.SetBufferSize( sizeof( ColorObject ) * nMaxNumObjects );
}
  
// *************************************************************************

void CVisColorPick::DoProcessing()
{
	Uint32 * pInputImg = (Uint32*)m_iportImage.GetBuffer();
	FastLabelObject * labels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	ColorObject * pColorObjects = (ColorObject*)m_oportColorData.GetBuffer();

	PickColors( pInputImg, labels, pColorObjects );
}
  
// *************************************************************************

void CVisColorPick::PickColors( const Uint32 * pInputImg, const FastLabelObject * labels, ColorObject * pColorObjects )
{
	Int nRadius = (m_nWindowSize-1)/2;

	Int accH, accS, accL, accHDivSat;
	Int divH, divHDivSat;
	
	Uint32 nInputWidth, nInputHeight;
	m_iportImage.GetImageSize( nInputWidth, nInputHeight );

	// Store number of objects.
	pColorObjects[0].nNumObjects = labels[0].unNumObjects;

	// for each label object...
	for ( Uint32 i=1; i<labels[0].unNumObjects; i++ )
	{
		Int mx,my;

		mx = labels[i].unMx;
		my = labels[i].unMy;
		
		// Adjust mx and my so that we're not violating the image's boundaries.
		mx = max( 0+nRadius, mx );
		mx = min( nInputWidth-1-nRadius, mx );
		my = max( 0+nRadius, my );
		my = min( nInputHeight-1-nRadius, my );

		accH = 0;
		accS = 0;
		accL = 0;
		accHDivSat = 0;
		divH = 0;
		divHDivSat = 0;

		for ( Int y= my-nRadius; y<=my+nRadius; y++)
		{
			for ( Int x= mx-nRadius; x<=mx+nRadius; x++)
			{	
				Uint8 H,S,L;
				Uint8 R,G,B;
				Uint32 pixel;

				pixel = pInputImg[ x + m_unResultWidth*y ];

				R = (Uint8)( pixel & 0x0000FF);
				G = (Uint8)((pixel & 0x00FF00) >> 8);
				B = (Uint8)((pixel & 0xFF0000) >> 16);

				CVisColor::RGBToHSL( R,G,B, H,S,L );
				
				if ( ( L>=m_nMinLuminance) && (L<=m_nMaxLuminance) )
				{
					accH += H;
					divH++;

				}
				accS += S;
				accL += L;

				if ( S>0 )
				{
					accHDivSat += (H * 256 ) / S;
					divHDivSat += 256*256 / S;
				}
			}
		}

		int div = (2*nRadius+1)*(2*nRadius+1);

		//LogMsg( "color: h:%d, s:%d, l:%d", accH / div, accS / div, accL / div );

		// Prevent division by zero...
		divH = max( divH, 1 );
		divHDivSat = max( divHDivSat, 1 );

		// Store color info
		pColorObjects[i].Color.unHue = accH/ divH;
		pColorObjects[i].Color.unSat = accS / (m_nWindowSize*m_nWindowSize);
		pColorObjects[i].Color.unLum = accL / (m_nWindowSize*m_nWindowSize);
		pColorObjects[i].Color.unHueDivSat = (accHDivSat*256)/ divHDivSat;

	}
}
 
// *************************************************************************


