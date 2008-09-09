
#include "classVisLabelVisualizer.h"

#include "classVisVision.h"
#include "classVisObjectManager.h"
#include "classVisGraphics.h"



CVisLabelVisualizer::CVisLabelVisualizer( const Char * strName, Int width, Int height, Int scale )
		:	CVisComponent( strName, "LabelVisualizer" ) ,
			m_iportImage( "rgbInput", CVisPort::PDT_32BPP_RGB ),
			m_iportLabelData( "objLabels", CVisPort::PDT_DATA ),
			m_oportResult( "output", CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_LARGE | CVisOutputPort::OUTPORT_NON_CACHED )
{
	m_iportImage.Init( this );
	m_iportLabelData.Init( this );
	m_oportResult.Init( this );
	
	m_oportResult.SetImageSize( width, height );
	
	m_unResultWidth = width;
	m_unResultHeight = height;	
	m_nScale = scale;

	this->SetMainInputPort( &m_iportImage );
}
		
// *************************************************************************

void CVisLabelVisualizer::DoProcessing()
{
	Uint32 y;
	
	// Acquire buffers
	Uint32*							inputImg = (Uint32*)m_iportImage.GetBuffer();
	FastLabelObject *				objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	Uint32*							outputImg = (Uint32*)m_oportResult.GetBuffer();	
	
	// Determine bounds
	Uint32 w_in, h_in;
	Uint32 w_out, h_out;
	m_iportImage.GetImageSize( w_in, h_in );
	
	w_out = min( w_in/m_nScale, m_unResultWidth );
	h_out = min( h_in/m_nScale, m_unResultHeight );
	
	// Copy input image downscaled to the output image. We'll use a 2D to 1D DMA transfer
	// for this.
	for ( y=0; y< h_out; y++ )
	{
		WaitCopy( StartCopy2D( 	COPY_2D1D, 
								outputImg + y * m_unResultWidth , 
								inputImg + y * w_in * m_nScale,
								4, m_unResultWidth, 4 * m_nScale ));
	}

	// Draw labels
	DrawLabels( objLabels, outputImg );		
}
			
// *************************************************************************

void CVisLabelVisualizer::DrawLabels( const FastLabelObject * objects, Uint32 * outputImg )
{
	Uint32 i;
	
	#define CROSS_COLOR 	0x606060
	#define BORDER_COLOR	0xF0F0F0
	#define CROSS_SIZE		10
	
	// Get a graphics object
	CVisGraphics g( outputImg, m_unResultWidth, m_unResultHeight, 32 );

	// Go through all objects and draw them (skip background with index 0)
	for (i=1; i<objects[0].unNumObjects; i++)
	{
		if ( objects[i].bValid )
		{
			Int32 x,y;
			Int32 x1,y1,x2,y2;

			x = objects[i].unMx / m_nScale;
			y = objects[i].unMy / m_nScale;

			x1 = objects[i].unBoundingLeft / m_nScale;
			x2 = objects[i].unBoundingRight / m_nScale;
			y1 = objects[i].unBoundingTop / m_nScale;
			y2 = objects[i].unBoundingBottom / m_nScale;

			// Draw cross in center
			int x_min, x_max, y_min, y_max;
			x_min = max( 0, x-CROSS_SIZE );
			x_max = min( m_unResultWidth-1, x+CROSS_SIZE );
			y_min = max( 0, y-CROSS_SIZE );
			y_max = min( m_unResultHeight-1, y+CROSS_SIZE );
			g.DrawHorizLine( x_min, x_max, y, CROSS_COLOR );
			g.DrawVertLine( x, y_min, y_max, CROSS_COLOR );

			// Draw bounding box
			g.DrawHorizLine( x1, x2, y1, BORDER_COLOR );
			g.DrawHorizLine( x1, x2, y2, BORDER_COLOR );
			g.DrawVertLine( x1, y1, y2, BORDER_COLOR );
			g.DrawVertLine( x2, y1, y2, BORDER_COLOR );
		}
	}	
}

// *************************************************************************
