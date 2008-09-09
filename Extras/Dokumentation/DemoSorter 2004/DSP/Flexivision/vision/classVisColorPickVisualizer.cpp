
#include "classVisColorPickVisualizer.h"

#include "classVisVision.h"
#include "classVisObjectManager.h"
#include "classVisGraphics.h"

#include "classVisFastLabel.h"
#include "classVisColor.h"
#include "classVisColorPick.h"

CVisColorPickVisualizer::CVisColorPickVisualizer( const Char * strName, Int scale, Uint32 unFlags )
	:CVisComponent( strName, "ColorPickVisualizer" )
	,m_iportImage( "rgbInput", CVisPort::PDT_32BPP_RGB )
	,m_iportLabelData( "objLabels", CVisPort::PDT_DATA )
	,m_iportColorData( "objColors", CVisPort::PDT_DATA )
	,m_oportOutput( "output",	  CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_LARGE
								| ((unFlags & OUTPUT_ON_INPUTPORT) ? CVisOutputPort::OUTPORT_HOLLOW : 0) )
		
{
	m_iportImage.Init( this );
	m_iportLabelData.Init( this );
	m_iportColorData.Init( this );
	m_oportOutput.Init( this );
	
	m_nScale = scale;

	m_unFlags = unFlags;

	this->SetMainInputPort( &m_iportImage );
}
		
// *************************************************************************

void CVisColorPickVisualizer::DoProcessing()
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
	FastLabelObject *	objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	ColorObject *		objColors = (ColorObject*)m_iportColorData.GetBuffer();
	Uint32*				pOutputImg = (Uint32*)m_oportOutput.GetBuffer();		

	// If we have different buffers for input and output, we need to copy the image first.
	// We can do that in the background. The image needs to be ready when before the objects
	// are actually cut.
	int nCopy = COPY_DONE;
	if ( (m_unFlags & OUTPUT_ON_INPUTPORT) == 0 )
		nCopy = StartCopy( pOutputImg, pInputImg, m_iportImage.GetBufferSize() ); 

	// Draw labels
	DrawObjects( objLabels, objColors, pOutputImg );		
}
			
// *************************************************************************

void CVisColorPickVisualizer::DrawObjects( const FastLabelObject * labelObjects, const ColorObject * colorObjects, Uint32 * outputImg )
{
	Uint32 i;
	
	#define CROSS_COLOR 	0x606060
	#define BORDER_COLOR	0xF0F0F0
	
	// Get a graphics object
	CVisGraphics g( outputImg, m_unResultWidth, m_unResultHeight, 32 );

	// Go through all objects and draw them (skip background with index 0)
	for (i=1; i<labelObjects[0].unNumObjects; i++)
	{
		if ( labelObjects[i].bValid )
		{
			Int x1_1, y1_1, x2_1, y2_1;
			Int x1_2, y1_2, x2_2, y2_2;

			Uint32 color;
			Uint8 R, G, B;
			CVisColor::HSVtoRGB(	colorObjects[i].Color.unHue, colorObjects[i].Color.unSat, 255,
									R, G, B );
			color = R | (G<<8) | (B<<16);

			x1_1 = labelObjects[i].unBoundingLeft / m_nScale;
			x1_2 = max ( 0, x1_1 - 1 );
			
			x2_1 = labelObjects[i].unBoundingRight / m_nScale;
			x2_2 = min ( m_unResultWidth-1, x2_1 + 1 );
			
			y1_1 = labelObjects[i].unBoundingTop / m_nScale;
			y1_2 = max ( 0, y1_1 - 1 );
			
			y2_1 = labelObjects[i].unBoundingBottom / m_nScale ;
			y2_2 = min ( m_unResultHeight-1, y2_1 + 1 );

			// Draw bounding box
			g.DrawHorizLine( x1_1, x2_1, y1_1, color );
			g.DrawHorizLine( x1_1, x2_1, y2_1, color );
			g.DrawVertLine( x1_1, y1_1, y2_1, color );
			g.DrawVertLine( x2_1, y1_1, y2_1, color );

			g.DrawHorizLine( x1_2, x2_2, y1_2, color );
			g.DrawHorizLine( x1_2, x2_2, y2_2, color );
			g.DrawVertLine( x1_2, y1_2, y2_2, color );
			g.DrawVertLine( x2_2, y1_2, y2_2, color );

			g.DrawInteger( colorObjects[i].Color.unHue, labelObjects[i].unBoundingLeft, labelObjects[i].unBoundingBottom+2, 0x00FFFFFF, 1 );
		}
	}	
}

// *************************************************************************
