
#include "classVisGradient.h"


// IMGLIB has to be include using C calls.
extern "C" 
{
#include "img_sobel.h"
}

// *************************************************************************

CVisGradient::CVisGradient( const Char * strName )
	: 	CVisComponent( strName, "Gradient" ),
		m_iportInput("g8Input", CVisPort::PDT_8BPP_GRAY ),
		m_oportResult("g8Output", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_LARGE )
{	
	m_iportInput.Init( this );
	m_oportResult.Init( this );

}

// *************************************************************************
						
void CVisGradient::Prepare()
{
}

// *************************************************************************
						
void CVisGradient::DoProcessing()
{
	Uint8 * pInputImage = (Uint8*)m_iportInput.GetBuffer();
	Uint8 * pGradientImage = (Uint8*)m_oportResult.GetBuffer();

	// Use the IMGLIB for this.
	IMG_sobel( pInputImage, pGradientImage, m_unResultWidth, m_unResultHeight);

}

// *************************************************************************

// *************************************************************************

// *************************************************************************

// *************************************************************************

