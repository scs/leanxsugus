
#include "classVisRegionSelect.h"

#include "classVisVector.h"

#include "classVisGraphics.h"


CVisRegionSelect::CVisRegionSelect( const char * strName, CVisPort::PortDataType pType, RegionSelectMode rsMode, const int numLimits )
	:	CVisComponent( "RegionSelect", strName ),
		m_iportInput( "input", pType ),
		m_oportMask( "maskOutput", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_LARGE ),
		m_oportOutput( "output", pType, CVisOutputPort::OUTPORT_LARGE ),
		m_oportLimits( "limit", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST ),
		m_propLimitLeft( "LimitLeft" ),
		m_propLimitRight( "LimitRight" ),
		m_propLimitTop( "LimitTop" ),
		m_propLimitBottom( "LimitBottom" )
{
	m_iportInput.Init( this );
	m_oportMask.Init( this );
	m_oportOutput.Init( this );
	m_oportLimits.Init( this );

	m_rsMode = rsMode;

	if ( m_rsMode == RS_MASKINPUTIMAGE )
	{
		// Won't need the limits buffer
		m_oportLimits.SetBufferSize(0);
	}
	else
	{
		// Won't need the output buffer
		m_oportOutput.SetBufferSize(0);
		
		// Setup the limits buffer
		m_oportLimits.SetBufferSize( numLimits * sizeof(Uint16) * 2 );
	}

	m_propLimitLeft.Init( this, CVisProperty::PT_FLOAT, &m_fLimitLeft );
	m_propLimitRight.Init( this, CVisProperty::PT_FLOAT, &m_fLimitRight );
	m_propLimitTop.Init( this, CVisProperty::PT_FLOAT, &m_fLimitTop );
	m_propLimitBottom.Init( this, CVisProperty::PT_FLOAT, &m_fLimitBottom );
	
	m_fLimitLeft = -0.1;
	m_fLimitRight = 0.1;
	m_fLimitTop = 0.1;
	m_fLimitBottom = -0.1;

	m_pTransform = NULL;
}

// *************************************************************************

void CVisRegionSelect::Prepare()
{
	Int bpp = m_iportInput.GetImageBPP();
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * (bpp/8), (Ptr*)&m_pInputLine, CVisBufferManager::BUF_FAST );
}

// *************************************************************************

void CVisRegionSelect::DoProcessing()
{

	if (m_pTransform == NULL )
		return;

	if ( HavePropertiesChanged() || m_pTransform->HavePropertiesChanged() )
	{
		ResetPropertiesChanged();
		DrawWorldRect();
	}
	
	// Copy and draw the input image if it is requested
	if ( m_rsMode == RS_MASKINPUTIMAGE )
		MaskInputImage();

}

// *************************************************************************

void CVisRegionSelect::MaskInputImage()
{
	// Copy to output
	WaitCopy( StartCopy( 	m_oportOutput.GetBuffer(), m_iportInput.GetBuffer(),
							m_oportOutput.GetBufferSize()) );
	m_oportOutput.CacheInvalidate();

	// Mask
	Uint32 w,h;
	m_oportOutput.GetImageSize(w,h);
	switch ( m_iportInput.GetImageBPP() )
	{
	case 32:
		Mask_32(	(Uint8*)m_oportMask.GetBuffer(), 
					(Uint32*)m_oportOutput.GetBuffer(), 
					w,h	);
		break;
	}
	
	m_oportOutput.CacheWriteback();
}

// *************************************************************************

void CVisRegionSelect::SetTransform( CVisTransform * pTransform )
{
	m_pTransform = pTransform;
}

// *************************************************************************

void CVisRegionSelect::SetLimit( const float fLeft, const float fTop, const float fRight, const float fBottom )
{
	m_fLimitLeft = fLeft;
	m_fLimitRight = fRight; 
	m_fLimitTop = fTop; 
	m_fLimitBottom = fBottom;
}

// *************************************************************************

void CVisRegionSelect::DrawWorldRect()
{
	CVisVector w1, w2, w3, w4;
	CVisVector p1, p2, p3, p4;

	w1(1) = m_fLimitLeft;
	w1(2) = m_fLimitTop;
	w1(3) = 0;
	
	w2(1) = m_fLimitRight;
	w2(2) = m_fLimitTop;
	w2(3) = 0;

	w3(1) = m_fLimitRight;
	w3(2) = m_fLimitBottom;
	w3(3) = 0;

	w4(1) = m_fLimitLeft;
	w4(2) = m_fLimitBottom;
	w4(3) = 0;

	p1 = m_pTransform->TransformToPixel( w1 );
	p2 = m_pTransform->TransformToPixel( w2 );
	p3 = m_pTransform->TransformToPixel( w3 );
	p4 = m_pTransform->TransformToPixel( w4 );

	DrawQuad(	p1(1).GetIntegerValue(), p1(2).GetIntegerValue(),
				p2(1).GetIntegerValue(), p2(2).GetIntegerValue(),
				p3(1).GetIntegerValue(), p3(2).GetIntegerValue(),
				p4(1).GetIntegerValue(), p4(2).GetIntegerValue() );
}

// *************************************************************************

void CVisRegionSelect::DrawQuad(	const int x1, const int y1,
									const int x2, const int y2,
									const int x3, const int y3,
									const int x4, const int y4 )
{
	Uint32 w,h, bpp;
	m_oportMask.GetImageSize( w,h );
	bpp = m_oportMask.GetImageBPP();
	CVisGraphics g( m_oportMask.GetBuffer(), w,h, bpp);

	g.Clear( 0 );
	g.DrawLine( x1,y1, x2,y2, 0xFF );
	g.DrawLine( x2,y2, x3,y3, 0xFF );
	g.DrawLine( x3,y3, x4,y4, 0xFF );
	g.DrawLine( x4,y4, x1,y1, 0xFF );
	g.HorizFlood( (x1+x2+x3+x4)/4, (y1+y2+y3+y4)/4, 0xFF, 0xFF );
}

// *************************************************************************

void CVisRegionSelect::CreateLimitsVectorHoriz()
{
}

// *************************************************************************

void CVisRegionSelect::Mask_32( const Uint8 * restrict pMask, Uint32 * restrict pDest, const int nWidth, const int nHeight)
{

	for (int y=0; y<nHeight; y++)
	{
		Uint32 * pDestLine = pDest + y*nWidth;
		const Uint8 * pMaskLine = pMask + y*nWidth;
		
		for (int x=0; x<nWidth; x++)
		{
			if (pMaskLine[x] == 0)
				pDestLine[x] = 0;
		}
	}
	
	/*
	Uint32 * pInputLine = (Uint32*)m_pInputLine;

	for (int line=0; line<nHeight; line++)
	{
		// Copy line.
		QuickCopy( pInputLine, (Ptr)(pSource + line*nWidth), nWidth*4 );

		// Mask line
		for (int i=0; i<nWidth-3; i+=4 )
		{
			pDest[line*nWidth + i  ]	= pMask[line*nWidth + i]	!= 0 ? pInputLine[i] : 0;
			pDest[line*nWidth + i+1]	= pMask[line*nWidth + i+1]	!= 0 ? pInputLine[i+1] : 0;
			pDest[line*nWidth + i+2]	= pMask[line*nWidth + i+2]	!= 0 ? pInputLine[i+2] : 0;
			pDest[line*nWidth + i+3]	= pMask[line*nWidth + i+3]	!= 0 ? pInputLine[i+3] : 0;
		}

	}
*/
	
}

// *************************************************************************
