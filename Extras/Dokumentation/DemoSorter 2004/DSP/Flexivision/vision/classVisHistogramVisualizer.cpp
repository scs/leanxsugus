
#include "classVisHistogramVisualizer.h"

CVisHistogramVisualizer::CVisHistogramVisualizer( const char * strName, const Int nHistogramWidth, const Int nHeight )
	:	CVisComponent( strName, "HistVisualizer" ),
		m_iportHistogram( "input", CVisPort::PDT_DATA ),
		m_oportOutput( "output", CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_LARGE ),
		m_propColor( "Color" ),
		m_propLog( "Log" )
{
	m_iportHistogram.Init( this );
	m_oportOutput.Init( this );
	m_oportOutput.SetImageBPP( 32 );
	m_oportOutput.SetImageSize( nHistogramWidth, nHeight );

	m_unResultWidth = nHistogramWidth;
	m_unResultHeight = nHeight;

	m_propColor.Init( this, CVisProperty::PT_INTEGER, &m_nColor );
	m_propLog.Init( this, CVisProperty::PT_INTEGER, &m_nLog );

	m_nColor = 0x005050FF;
	m_nBackgroundColor = 0x00E0E0E0;
	m_nLog = 0;

}

					
// *************************************************************************

void CVisHistogramVisualizer::DoProcessing()
{
	Uint32 * pHistogram = (Uint32*)m_iportHistogram.GetBuffer();
	Uint32 * pImage = (Uint32*)m_oportOutput.GetBuffer();

	VisualizeDecimal( pHistogram, pImage );
	
	m_oportOutput.CacheWriteback();
}
					
// *************************************************************************

void CVisHistogramVisualizer::VisualizeDecimal( const Uint32 * restrict pHistogram, Uint32 * restrict pImage )
{
	unsigned int x,y;
	Uint32 unMax;
	float fMul;

	// Clear image.
	for ( y=0; y<m_unResultHeight; y++ )
		for ( x=0; x<m_unResultWidth; x++ )
			pImage[x + y*m_unResultWidth] = m_nBackgroundColor;

	// Find maximum value
	unMax = 0;
	for ( x=0; x<m_unResultWidth; x++ )
		unMax = max ( pHistogram[x], unMax );
	fMul = (float)m_unResultHeight / (float)unMax;

	// Draw histogram
	for ( x=0; x<m_unResultWidth; x++ )
	{
		float fValue = (float)pHistogram[x] * fMul;
		int nValue = (int)fValue;

		for ( y=m_unResultHeight-1; y > max(0, ((signed)m_unResultHeight-nValue-1)) ; y-- )
			pImage[ x + y*m_unResultWidth ] = m_nColor;
	}
}
					
// *************************************************************************
