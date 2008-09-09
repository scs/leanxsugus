
#include "classVisBorderPaint.h"


// *************************************************************************

CVisBorderPaint::CVisBorderPaint( const Char * strName )
	:	CVisComponent( strName, "BorderPaint" ),
		m_iportImage( "input", CVisPort::PDT_8BPP_GRAY  ),
		m_iportBorderData( "dataBorder", CVisPort::PDT_DATA ),
		m_oportImage( "output", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_HOLLOW ),

		m_propBrushSize( "BrushSize" ),
		m_propColor( "Color" )
		
{
	// Set the main input port, which defines the size and type of the output port.
	SetMainInputPort( &m_iportImage );

	m_iportImage.Init( this );
	m_iportBorderData.Init( this );
	m_oportImage.Init( this );

	m_propBrushSize.Init( this, CVisProperty::PT_INTEGER, &m_nBrushSize );
	m_nBrushSize = 1;

	m_propColor.Init( this, CVisProperty::PT_INTEGER, & m_nColor );
	m_nColor = 0;	
}

// *************************************************************************

void CVisBorderPaint::DoProcessing()
{
	const BorderData *	pBorder = (BorderData*)m_iportBorderData.GetBuffer();
	Uint8 *				pImage = (Uint8 *)m_iportImage.GetBuffer();

	// Propagate the buffer to the output port and all connected input ports
	m_oportImage.SetBuffer( pImage );
	m_oportImage.PropagateBuffer();

	// Now differentiate between various brush sizes: 1, 3 and larger.
	if ( m_nBrushSize == 1 )
		Paint1( pBorder, pImage, m_nColor );
	else if ( m_nBrushSize == 3 )
		Paint3( pBorder, pImage, m_nColor );
	else
		Paint( pBorder, pImage, m_nBrushSize, m_nColor );

}

// *************************************************************************

void CVisBorderPaint::Paint1( const BorderData * restrict pBorder, Uint8 * restrict pImage, const Uint8 unColor )
{
	Int nNumPoints = pBorder->unNumPoints;
	for ( Int i=0; i<nNumPoints; i++ )
	{
		Bool bPaint;
		Int16 x,y;

		x = pBorder->unPosX[i];
		y = pBorder->unPosY[i] ;
		
		bPaint = TRUE;
		
		if ( ( x < 0 ) || (y < 0 ) || ( x >= m_unResultWidth) || (y >= m_unResultHeight) )
			bPaint = FALSE;
			
		if ( bPaint )
			pImage[ pBorder->unPosX[i] + pBorder->unPosY[i] * m_unResultWidth ] = unColor;
	}
}
	
// *************************************************************************

void CVisBorderPaint::Paint3( const BorderData * restrict pBorder, Uint8 * restrict pImage, const Uint8 unColor )
{
	Int nNumPoints = pBorder->unNumPoints;
	for ( Int i=0; i<nNumPoints; i++ )
	{
		Bool bPaint;
		Uint16 x,y;

		x = pBorder->unPosX[i];
		y = pBorder->unPosY[i] ;
		
		bPaint = TRUE;
				
		if ( ( x < 1 ) || (y < 1 ) || ( x >= m_unResultWidth-1) || (y >= m_unResultHeight-1) )
			bPaint = FALSE;
			
		if ( bPaint )
		{
			pImage[ (x-1)	+ (y-1) * m_unResultWidth ] = unColor;
			pImage[ x		+ (y-1) * m_unResultWidth ] = unColor;
			pImage[ (x+1)	+ (y-1) * m_unResultWidth ] = unColor;
			pImage[ (x-1)	+ y * m_unResultWidth ] = unColor;
			pImage[ x		+ y * m_unResultWidth ] = unColor;
			pImage[ (x+1)	+ y * m_unResultWidth ] = unColor;
			pImage[ (x-1)	+ (y+1) * m_unResultWidth ] = unColor;
			pImage[ x		+ (y+1) * m_unResultWidth ] = unColor;
			pImage[ (x+1)	+ (y+1) * m_unResultWidth ] = unColor;
		}
	}
}
	
// *************************************************************************

void CVisBorderPaint::Paint( const BorderData * restrict pBorder, Uint8 * restrict pImage, Int nWidth, const Uint8 unColor )
{
	Int nNumPoints = pBorder->unNumPoints;
	for ( Int i=0; i<nNumPoints; i++ )
	{
		Uint16 x,y;

		x = pBorder->unPosX[i];
		y = pBorder->unPosY[i] ;
	
		for ( Int i= -(nWidth/2); i<=(nWidth/2); i++ )
			for ( Int j= -(nWidth/2); j<=(nWidth/2); j++ )
				pImage[ (x+j) + (y+i)*m_unResultWidth ] = unColor;
	}
}
	
// *************************************************************************


