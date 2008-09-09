
#include "classVisThreshold.h"


// *************************************************************************

CVisThreshold::CVisThreshold( const Char * strName, ThresholdType ttType )
	:	CVisComponent( strName, "Threshold" ),
		m_iportInput( "input", CVisPort::PDT_8BPP_GRAY ),
		m_oportOutput( "output", CVisPort::PDT_8BPP_GRAY, CVisBufferManager::BUF_FAST ),
		m_propThreshold( "Threshold" )
{
	m_iportInput.Init( this );
	m_oportOutput.Init( this );

	m_propThreshold.Init( this, CVisProperty::PT_INTEGER, &m_nThreshold );

	m_nThreshold = 128;

	m_ttType = ttType;
}

// *************************************************************************

CVisThreshold::~CVisThreshold()
{
}

// *************************************************************************

void CVisThreshold::DoProcessing()
{
	m_iportInput.CacheInvalidate();
	m_oportOutput.CacheInvalidate();
	
	Uint8 * pSrc = (Uint8*)m_iportInput.GetBuffer();
	Uint8 * pDst = (Uint8*)m_oportOutput.GetBuffer();

	Threshold_8u( pDst, pSrc, m_unResultWidth, m_unResultHeight, (Uint8)m_nThreshold, m_ttType );
	
	m_oportOutput.CacheWriteback();
	m_oportOutput.CacheInvalidate();
}

// *************************************************************************

void CVisThreshold::Threshold_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int cols, const Int rows, const Uint8 unThreshold, const ThresholdType ttType )
{
	switch( ttType )
	{
	case TT_CLAMP_TO_MAX:
		IMG_thr_gt2max( src, dst, cols, rows, unThreshold );
		break;

	case TT_CLIP_ABOVE:
		IMG_thr_gt2thr( src, dst, cols, rows, unThreshold );
		break;

	case TT_CLAMP_TO_ZERO:
		IMG_thr_le2min( src, dst, cols, rows, unThreshold );
		break;

	case TT_CLIP_BELOW:
		IMG_thr_le2thr( src, dst, cols, rows, unThreshold );
		break;
	}
}

// *************************************************************************
