

#include "classVisModelVisualizer.h"

#include "classVisGraphics.h"

#include "classVisMatrix.h"
#include "classVisVector.h"
#include "classVisFixpoint.h"

CVisModelVisualizer::CVisModelVisualizer( const Char * strName, Int scale, Uint32 unFlags )
	:CVisComponent( strName, "ColorPickVisualizer" )
	,m_iportImage( "rgbInput", CVisPort::PDT_32BPP_RGB )
	,m_oportOutput( "output",	  CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_LARGE
								| ((unFlags & OUTPUT_ON_INPUTPORT) ? CVisOutputPort::OUTPORT_HOLLOW : 0) )
{
	m_iportImage.Init( this );
	m_oportOutput.Init( this );
	
	m_nScale = scale;

	m_unFlags = unFlags;

	m_pTransform = NULL;
	m_pModel = NULL;

	this->SetMainInputPort( &m_iportImage );
}
			
// *************************************************************************

void CVisModelVisualizer::DoProcessing()
{
	// If we use the same buffer for output and for input, we must propagate the pointer
	// each time before processing it. 	
	if ( m_unFlags & OUTPUT_ON_INPUTPORT )
	{
		m_oportOutput.SetBuffer( m_iportImage.GetBuffer() );
		m_oportOutput.PropagateBuffer();
	}
	
	// Acquire buffers
	Uint32*				pInputImg = (Uint32*)m_iportImage.GetBuffer();
	Uint32*				pOutputImg = (Uint32*)m_oportOutput.GetBuffer();		

	// If we have different buffers for input and output, we need to copy the image first.
	// We can do that in the background. The image needs to be ready when before the objects
	// are actually cut.
	int nCopy = COPY_DONE;
	if ( (m_unFlags & OUTPUT_ON_INPUTPORT) == 0 )
		nCopy = StartCopy( pOutputImg, pInputImg, m_iportImage.GetBufferSize() ); 

	m_iportImage.CacheInvalidate();
	
	// Draw model
	if ( m_pTransform != NULL )
		DrawModel( pOutputImg, m_pTransform );
		
	m_oportOutput.CacheWriteback();
	
}
			
// *************************************************************************

void CVisModelVisualizer::SetTransform( CVisTransform * transform )
{
	m_pTransform = transform;
}

// *************************************************************************

void CVisModelVisualizer::SetModel( CVisDemoSorterModel * model )
{
	m_pModel = model;
}

// *************************************************************************

void CVisModelVisualizer::DrawModel( Uint32 * outputImg, CVisTransform * transform )
{
	#define ORIGIN_SIZE 0.040f

	if ( (m_pTransform == NULL ) || (m_pModel == NULL) )
		return;
	
	CVisGraphics g( outputImg, m_unResultWidth, m_unResultHeight, 32 );

	CVisFixpoint left, right, top, bottom;	
	CVisFixpoint pos, y1, y2;
	CVisFixpoint f0;
	f0 = 0;

	

	// Draw border
	m_pModel->GetTotalArea( left, right, top, bottom );
	DrawLine( &g, transform, left, 	bottom, 	f0, 	right, 		bottom, 	f0, 	0x00FF00FF );
	DrawLine( &g, transform, right, bottom, 	f0, 	right, 		top, 		f0, 	0x00FF00FF );
	DrawLine( &g, transform, right, top, 		f0, 	left, 		top, 		f0, 	0x00FF00FF );
	DrawLine( &g, transform, left, 	top, 		f0, 	left, 		bottom, 	f0, 	0x00FF00FF );
	y1 = bottom;

	// Draw valid zone
	m_pModel->GetValidArea( left, right, top, bottom );
	DrawLine( &g, transform, left, 	bottom, 	f0, 	right, 		bottom, 	f0, 	0x008F008F );
	DrawLine( &g, transform, right, bottom, 	f0, 	right, 		top, 		f0, 	0x008F008F );
	DrawLine( &g, transform, right, top, 		f0, 	left, 		top, 		f0, 	0x008F008F );
	DrawLine( &g, transform, left, 	top, 		f0, 	left, 		bottom, 	f0, 	0x008F008F );
	y2 = bottom;
	
	// Draw Zero
	DrawLine( &g, transform, 0, 0, 0, 0, ORIGIN_SIZE, 0, 0x00FFFF00 );
	DrawLine( &g, transform, 0, 0, 0, ORIGIN_SIZE, 0, 0, 0x00FFFF00 );

	// Draw jets
	for ( int i=0; i<m_pModel->GetNumJets() ; i++ )
	{
		pos = m_pModel->GetJetPosition( i );		
		DrawLine( &g, transform, pos, y1, f0, pos, y2, f0, 0x00FFFFFF );
	}
}

// *************************************************************************

void CVisModelVisualizer::DrawLine( CVisGraphics * g, CVisTransform * transform, 
									const float x1, const float y1, const float z1,
									const float x2, const float y2, const float z2,
									Uint32 unColor )
{
	CVisVector v1, v2;
	v1(1) = x1;
	v1(2) = y1;
	v1(3) = z1;
	v1(4) = 0;

	v2(1) = x2;
	v2(2) = y2;
	v2(3) = z2;
	v2(4) = 0;
	
	CVisVector p1, p2;
	p1 = transform->TransformToPixel( v1 );
	p2 = transform->TransformToPixel( v2 );

	g->DrawLine(	p1(1).GetIntegerValue()/m_nScale, p1(2).GetIntegerValue()/m_nScale,
					p2(1).GetIntegerValue()/m_nScale, p2(2).GetIntegerValue()/m_nScale,
					unColor );

}

// *************************************************************************

void CVisModelVisualizer::DrawLine( CVisGraphics * g, CVisTransform * transform, 
									const CVisFixpoint x1, const CVisFixpoint y1, const CVisFixpoint z1,
									const CVisFixpoint x2, const CVisFixpoint y2, const CVisFixpoint z2,
									Uint32 unColor )
{
	CVisVector v1, v2;
	v1(1) = x1;
	v1(2) = y1;
	v1(3) = z1;
	v1(4) = 0;

	v2(1) = x2;
	v2(2) = y2;
	v2(3) = z2;
	v2(4) = 0;

	CVisVector p1, p2;
	p1 = transform->TransformToPixel( v1 );
	p2 = transform->TransformToPixel( v2 );

	g->DrawLine(	p1(1).GetIntegerValue()/m_nScale, p1(2).GetIntegerValue()/m_nScale,
					p2(1).GetIntegerValue()/m_nScale, p2(2).GetIntegerValue()/m_nScale,
					unColor );
}