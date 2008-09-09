
#include "classVisObjectsVisualizerEmb.h"
#include "classVisFourierDescriptor.h"
#include "classVisCutter.h"
#include "classVisSplitClassifier.h"

#include "classVisVision.h"
#include "classVisObjectManager.h"

#ifndef _WINDOWS
#include "../drvConveyor.h"
#endif

#define NUM_WIDTH 5
#define NUM_HEIGHT 7
const Uint8 unNumbers[12][NUM_WIDTH*NUM_HEIGHT] =
{
	// 0
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 1
	{	0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0	},
	// 2
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 1, 0, 0,		0, 1, 0, 0, 0,		1, 1, 1, 1, 1	},
	// 3
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 4
	{	0, 0, 0, 1, 0,		0, 0, 1, 1, 0,		0, 1, 0, 1, 0,		1, 0, 0, 1, 0,		1, 1, 1, 1, 1,		0, 0, 0, 1, 0,		0, 0, 0, 1, 0	},
	// 5
	{	0, 1, 1, 1, 1,		0, 1, 0, 0, 0,		0, 1, 0, 0, 0,		0, 1, 1, 1, 0,		0, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 6
	{	0, 1, 1, 1, 1,		1, 0, 0, 0, 0,		1, 0, 0, 0, 0,		1, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 7
	{	1, 1, 1, 1, 1,		0, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0	},
	// 8
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 9
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 1,		0, 0, 0, 0, 1,		0, 0, 0, 0, 1,		1, 1, 1, 1, 0	},
	// .
	{	0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 1, 0, 0	},
	// %
	{	0, 1, 1, 0, 0,		0, 1, 1, 0, 0,		0, 0, 0, 0, 1,		0, 1, 1, 1, 0,		1, 0, 0, 0, 0,		0, 0, 1, 1, 0,		0, 0, 1, 1, 0	}
};


CVisObjectsVisualizerEmb::CVisObjectsVisualizerEmb( const Char * strName, Int width, Int height, Int scale )
		:	CVisComponent( strName, "Visualizer" ) ,
			m_iportImage( "rgbInput", CVisPort::PDT_32BPP_RGB ),
			m_iportLUTImage( "lut8Input", CVisPort::PDT_8BPP_INDEX ), 
			m_iportLabelData( "objLabels", CVisPort::PDT_DATA ),
			m_iportObjectData( "objPotatoes", CVisPort::PDT_DATA ),
			m_iportBorderData( "dataBorder", CVisPort::PDT_DATA ),
			m_iportSplitsData( "objSplits", CVisPort::PDT_DATA ),
			//m_oportResult( "output", CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_FAST ),			
			m_oportResult( "output", CVisPort::PDT_32BPP_RGB, CVisOutputPort::OUTPORT_LARGE | CVisOutputPort::OUTPORT_NON_CACHED ),
			m_propBadColorIndicatorThresh( "BadColorIndThr" ),
			m_propGreenColorIndicatorThresh( "GreenColorIndThr" ),
			m_propBadShapeIndicatorThresh( "BadShapeIndThr" )
{
	m_iportImage.Init( this );
	m_iportLUTImage.Init( this );
	m_iportLabelData.Init( this );
	m_iportObjectData.Init( this );
	m_iportBorderData.Init( this );
	m_iportSplitsData.Init( this );
	m_oportResult.Init( this );
	
	m_oportResult.SetImageSize( width, height );
	
	m_unResultWidth = width;
	m_unResultHeight = height;	
	m_nScale = scale;

	m_pTransform = NULL;
	
	this->SetMainInputPort( &m_iportImage );
	
	// Setup properties	
	m_propBadColorIndicatorThresh.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpBadColorIndicatorThresh, PotatoObject::FP_FRACTIONAL_BITS );
	m_propGreenColorIndicatorThresh.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpGreenColorIndicatorThresh, PotatoObject::FP_FRACTIONAL_BITS );	
	m_propBadShapeIndicatorThresh.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpBadShapeIndicatorThresh, PotatoObject::FP_FRACTIONAL_BITS );
	
	m_fpBadColorIndicatorThresh = float2fp( 0.5, PotatoObject::FP_FRACTIONAL_BITS );
	m_fpGreenColorIndicatorThresh = float2fp( 0.05, PotatoObject::FP_FRACTIONAL_BITS );
	m_fpBadShapeIndicatorThresh = float2fp( 80, PotatoObject::FP_FRACTIONAL_BITS );	
}
					
// *************************************************************************

void CVisObjectsVisualizerEmb::Prepare()
{
	(Uint8*)CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth, (Ptr*)&m_pLutDataLine, CVisBufferManager::BUF_FAST );
}
		
// *************************************************************************
	
void CVisObjectsVisualizerEmb::SetTransform( CVisTransform * transform )
{
	m_pTransform = transform;
}
		
// *************************************************************************

void CVisObjectsVisualizerEmb::DoProcessing()
{
	Uint32 y;
	
	// Acquire buffers
	FastLabelObject *				objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList = (PotatoList*)m_iportObjectData.GetBuffer();
	Uint32*							inputImg = (Uint32*)m_iportImage.GetBuffer();
	Uint8 *							lutImg = (Uint8 *)m_iportLUTImage.GetBuffer();
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


	// Draw the conveyor data
	VisualizeConveyorData( outputImg );
		
	if ( (m_eOperationMode == OP_CLASSIFICATION) || (m_eOperationMode == OP_CALIBRATION) )
	{
		// Visualize the perspective
		//VisualizePerspective( false );
		
		// Apply bad lut data first
		VisualizeLUTData( lutImg, outputImg );

		// Draw labels
		DrawLabels( objLabels, outputImg );		
		
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::DoPostProcessing()
{
	Uint32 y;
	
	// Acquire buffers
	FastLabelObject *				objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList = (PotatoList*)m_iportObjectData.GetBuffer();
	Uint32*							outputImg = (Uint32*)m_oportResult.GetBuffer();	

	if ( (m_eOperationMode == OP_CLASSIFICATION) || (m_eOperationMode == OP_CALIBRATION) )
	{
		// Draw the tracking objects
		DrawPotatoes( pList->pObjects, outputImg );	
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizeLUTData( Uint8 * pLutImg, Uint32 * pOutputImg )
{
	Uint32 x,y;
	
	// Determine bounds
	Uint32 w_in, h_in;
	Uint32 w_out, h_out;
	m_iportImage.GetImageSize( w_in, h_in );
	
	w_out = min( w_in/m_nScale, m_unResultWidth );
	h_out = min( h_in/m_nScale, m_unResultHeight );
	
	// Apply bad lut data on output image
	for ( y=0; y< (Uint32)h_out; y++ )
	{
		// First, copy a LUT line out of the SDRAM, downscaling it during
		// copying.			
		WaitCopy( StartCopy2D( 	COPY_2D1D, 
								m_pLutDataLine , 
								pLutImg + y * w_in * m_nScale,
								1, m_unResultWidth, m_nScale ));
		
		
		// Now paint the bad pixels to the visualizer buffer.
		for ( x=0; x< (Uint32)w_out; x++ )
		{
			Uint8 cLut;

			cLut = m_pLutDataLine[ x ];
			
			if  (cLut == 2)
				pOutputImg[ x + y*m_unResultWidth ] = 0x2020FF;
			
			if ( cLut == 3 )
				pOutputImg[ x + y*m_unResultWidth ] = 0x20FF20;
		}
	}

}


// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizePerspective( bool bColorizeLanes )
{
	if ( m_pTransform == NULL )
		return;

	// ----------------------------
	//  Colorize lanes?
	// ----------------------------
	if ( bColorizeLanes )
	{
		CVisVector v, vres;
		CVisFixpoint m(v(1));
		m = 1000;

		for ( int x=0; x<m_unResultWidth; x+=5 )
		{
			for (int y=0; y<m_unResultHeight; y+=5 )
			{
				v(1) = (x*m_nScale);
				v(2) = (y*m_nScale) + 515;
				vres = m_pTransform->TransformToWorld(v);

				int lane;
				Int nMM = (int)(vres(1) * 1000);				
				nMM = nMM + (10 * 126)/2;
				if ( nMM >= 0 )
					lane = nMM / 126;
				else
					lane = -1;
				if ( lane > 9)
					lane = -1;
			
				if ( lane != -1 )
				{
					Uint32 unColor = (lane)*111
									| ((lane*180)<<8)
									| ((lane*230)<<16);
					unColor = ~unColor;
					DrawHorizLine( x-1,x+1,y, unColor );
					DrawVertLine( x,y-1,y+1, unColor );
				}
			}
		}
	}


	// ----------------------------
	// Draw a line for each lane border.
	// ----------------------------
	CVisVector vlane;
	CVisVector vres_top, vres_bottom;
	
	for (int lane=0; lane<10+1; lane++)
	{
	
		// Transform the lane border from world to pixel coordinate. We'll need a point
		// at each end (i.e. at the top and at the bottom).
		vlane(1) = lane*126 - (10*126/2);
		vlane(1) /= 1000;
		vlane(3) = 0;

		// Calculate the top point
		vlane(2) = -1.0;		
		vres_top = m_pTransform->TransformToPixel( vlane );

		// Calculate the bottom point.
		vlane(2) = 1.0;
		vres_bottom = m_pTransform->TransformToPixel( vlane );

		// ... and draw the line.
		DrawLine(	(int)(vres_top(1)) / m_nScale, (int)(vres_top(2)) / m_nScale,
					(int)(vres_bottom(1)) / m_nScale, (int)(vres_bottom(2)) / m_nScale,
					0x00A0A0A0 );
	}
	
	// ----------------------------
	// Draw zones.
	// ----------------------------
	
	// First, we have to get the vision object
	CVisVision * vis;
	Int nIndex = -1;
	if ( CVisObjectManager::Instance()->GetNextObject( (CVisObject**)(&vis), CVisObject::CT_VISION, nIndex ) )
	{
		CVisVector vZone_left, vZone_right;
		CVisVector vRes_left, vRes_right;	
		vZone_left(1) = -0.60;
		vZone_left(3) = 0;
		vZone_right(1) = 0.60;
		vZone_right(3) = 0;
		
		// Get zones
		float fInsertion, fDrop, fEjection;
		vis->GetProperty( "Tracker", "InsertionZone", fInsertion );
		vis->GetProperty( "Tracker", "DropZone", fDrop );
		vis->GetProperty( "Tracker", "EjectionZone", fEjection );
		
		// Store all zones in an array. Note: the zones values are in millimeters,
		// we'll want them in meters.
		float fZones[3] = { 	fInsertion/1000, 	fDrop/1000, 	fEjection/1000 };
		Uint32 unZones[3] = { 	0x0080FF80,			0x00FF8080,		0x008080FF };
		
		// Draw lines.
		for ( int i=0; i<3; i++)
		{
			vZone_left(2) = fZones[i];
			vZone_right(2) = fZones[i];
			vRes_left = m_pTransform->TransformToPixel( vZone_left );
			vRes_right = m_pTransform->TransformToPixel( vZone_right );
			DrawLine(	(int)(vRes_left(1)) / m_nScale, (int)(vRes_left(2)) / m_nScale,
						(int)(vRes_right(1)) / m_nScale, (int)(vRes_right(2)) / m_nScale,
						unZones[i] );
		}
					
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizeCurrentColorData( Uint32 unPotatoId )
{
	Bool bBad = FALSE;
	Bool bGreen = FALSE;
	
	FastLabelObject * objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList = (PotatoList *)m_iportObjectData.GetBuffer();

	Uint32*			inputImg = (Uint32*)m_iportImage.GetBuffer();
	Uint32*			outputImg = (Uint32*)m_oportResult.GetBuffer();
	
	// Get color information
	Uint32 frame = pList->pObjects[unPotatoId].unCurrentImageNum;
	Uint32 unBad = pList->pObjects[unPotatoId].unpClassificationColor[ frame ];
	Uint32 unGreen = pList->pObjects[unPotatoId].unpClassificationGreen[ frame ];
	
	// Classify color
	if ( unBad > m_fpBadColorIndicatorThresh )
		bBad = TRUE;
		
	if ( unGreen > m_fpGreenColorIndicatorThresh )
		bGreen = TRUE;
	
	// Anything bad on this tater?
	if ( bGreen || bBad )
	{
		Uint32 color;
		
		// Get coordinates
		Uint32 x1,x2,y1,y2,index;		
		index = pList->pObjects[pList->unCurrentPotatoId].unCurrentLabel;
		x1 = objLabels[index].unBoundingLeft / m_nScale - 1;
		x2 = objLabels[index].unBoundingRight / m_nScale + 1;
		y1 = objLabels[index].unBoundingTop / m_nScale - 1;
		y2 = objLabels[index].unBoundingBottom / m_nScale + 1;

		// Determine color, green has priority
		if (bBad)
			color = 0x004040FF;
			
		if (bGreen)
			color = 0x0040FF40;
				
		// Draw bounding box
		DrawHorizLine( x1, x2, y1, color );
		DrawHorizLine( x1, x2, y2, color );
		DrawVertLine( x1, y1, y2, color );
		DrawVertLine( x2, y1, y2, color );
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizeCurrentFormData( Uint32 unPotatoId )
{
	Uint32 i;

	FastLabelObject * objLabels	= (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList		= (PotatoList *)m_iportObjectData.GetBuffer();
	BorderData *					border		= (BorderData*)m_iportBorderData.GetBuffer();

	Uint32*			inputImg = (Uint32*)m_iportImage.GetBuffer();
	Uint32*			outputImg = (Uint32*)m_oportResult.GetBuffer();

	Int32 x, y, index;
	index = pList->pObjects[unPotatoId].unCurrentLabel;

	// Draw the form difference value
	x = objLabels[index].unBoundingLeft / m_nScale;
	y = objLabels[index].unBoundingBottom / m_nScale;
	Uint32 frame = pList->pObjects[unPotatoId].unCurrentImageNum;
	Uint32 unDiff = pList->pObjects[unPotatoId].unpClassificationForm[ frame ];
	
//	DrawFixedpoint( unDiff, PotatoObject::FP_FRACTIONAL_BITS, 2, x, y+18, 255 );

	// Calculate "redness" of this shape
	Int nBadDef = m_fpBadShapeIndicatorThresh >> PotatoObject::FP_FRACTIONAL_BITS;
	Int redness = (Uint8)(min( unDiff >> PotatoObject::FP_FRACTIONAL_BITS, nBadDef ) );
	Uint8 r = (Uint8)(redness*255 / nBadDef);
	Uint32 unColor = (50 << 16) + ( (255-r)<<8 ) + r;//PIC_COLORREF( r, 255-r, 50 );

	// Draw the chain code points.
	x = objLabels[index].unMx - CVisCutter::CUTIMG_WIDTH/2;
	x = ( x+1 ) & ~(0x03);
	y = objLabels[index].unMy - CVisCutter::CUTIMG_HEIGHT/2;
	y = ( y+1 ) & ~(0x03);
	for ( i=0; i<border->unNumPoints; i+= 1*m_nScale)
	{
		DrawPixel(	outputImg, 
					(x+border->unPosX[i]) / m_nScale, (y+border->unPosY[i]) / m_nScale,
					/*0x2020FF*/ unColor );
	}	
}

// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizeCurrentSplitData( Uint32 unPotatoId )
{
	Uint32 i;

	FastLabelObject * objLabels	= (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList		= (PotatoList *)m_iportObjectData.GetBuffer();
	SplitList *						pSplitList	= (SplitList*)m_iportSplitsData.GetBuffer();

	Uint32*			outputImg = (Uint32*)m_oportResult.GetBuffer();

	Int32 x, y, index;
	index = pList->pObjects[unPotatoId].unCurrentLabel;
	x = objLabels[index].unMx - CVisCutter::CUTIMG_WIDTH/2;
	x = ( x+1 ) & ~(0x03);
	x = x;
	y = objLabels[index].unMy - CVisCutter::CUTIMG_HEIGHT/2;
	y = ( y+1 ) & ~(0x03);
	y = y;

	for ( i=0; i<pSplitList->unNumEntries; i++)
	{
		Uint32 x1 = (pSplitList->arySplitInfos[i].unBoundingLeft + x) / m_nScale;
		Uint32 x2 = (pSplitList->arySplitInfos[i].unBoundingRight + x) / m_nScale;
		Uint32 y1 = (pSplitList->arySplitInfos[i].unBoundingTop + y) / m_nScale;
		Uint32 y2 = (pSplitList->arySplitInfos[i].unBoundingBottom + y) / m_nScale;

		// Draw bounding box
		DrawHorizLine( x1, x2, y1, 0x008080FF );
		DrawHorizLine( x1, x2, y2, 0x008080FF );
		DrawVertLine( x1, y1, y2, 0x008080FF );
		DrawVertLine( x2, y1, y2, 0x008080FF );
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::VisualizeConveyorData( Uint32 * outputImg )
{
#ifndef _WINDOWS
	Uint32 pos = convGetPosition();
	Uint32 speed = convGetMeasuredSpeed();
#else
	Uint32 pos = 0;
	Uint32 speed = 0;
#endif

	Uint32 w,h;
	
	m_iportImage.GetImageSize( w, h );			
	
	pos = pos % (h/4);
	
	DrawHorizLine( 0, 20, pos / m_nScale, 0x0000FF00 );
	DrawHorizLine( 0, 20, (pos + h/4) / m_nScale, 0x0000FF00 );
	DrawHorizLine( 0, 20, (pos + 2*h/4) / m_nScale, 0x0000FF00 );
	DrawHorizLine( 0, 20, (pos + 3*h/4) / m_nScale, 0x0000FF00 );	
	
	DrawFixedpoint( speed, 16, 3, 1, 1, 0x0000FF00 );
}

		
// *************************************************************************

void CVisObjectsVisualizerEmb::DrawLabels( const FastLabelObject * objects, Uint32 * outputImg )
{
	Uint32 i;
	
	#define CROSS_COLOR 	0x606060
	#define BORDER_COLOR	0xA0A0A0

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
			DrawHorizLine( x-10, x+10, y, CROSS_COLOR );
			DrawVertLine( x, y-10, y+10, CROSS_COLOR );

			// Draw bounding box
			DrawHorizLine( x1, x2, y1, BORDER_COLOR );
			DrawHorizLine( x1, x2, y2, BORDER_COLOR );
			DrawVertLine( x1, y1, y2, BORDER_COLOR );
			DrawVertLine( x2, y1, y2, BORDER_COLOR );
/*
			// Draw color information
			Uint32 unBad = (Uint32)(objects[i].unpHistogram[2]) * (1<<16) / objects[i].unArea;
			Uint32 unGreen = (Uint32)(objects[i].unpHistogram[3]) * (1<<16) / objects[i].unArea;
			DrawFixedpointPercent( unBad, 16, 2, x1, y2+2, 255 );
			DrawFixedpointPercent( unGreen, 16, 2, x1, y2+10, 255 );
*/			
		}
	}	
}
					
// *************************************************************************

void CVisObjectsVisualizerEmb::DrawPotatoes( const PotatoObject * object, Uint32 * outputImg )
{

	CVisVector v_world;
	CVisVector v_pix;

	for ( Int32 i=0; i<PotatoList::MAX_OBJECTS; i++)
	{
		if ( object[i].bValid/* && object[i].bTracked */)
		{
			// Draw the object number where the object was last seen.
			v_world(1) = object[i].nLastSeenPos_mm_X;
			v_world(1) /= 1000;
			v_world(2) = object[i].nLastSeenPos_mm_Y;
			v_world(2) /= 1000;
			v_world(3) = 0;

			v_pix = m_pTransform->TransformToPixel( v_world );

			// Draw object number
			int x = (int)(v_pix(1)) / m_nScale + 2;
			int y = (int)(v_pix(2)) / m_nScale + 2;
			/*
			DrawInteger( i, x-1, y-1, 0x000000 );*/
			DrawInteger( i, x, y+1, 0x000000 );
			DrawInteger( i, x+1, y, 0x000000 );
			DrawInteger( i, x+1, y+1, 0x000000 );
			DrawInteger( i, x, y, 0x6060F0 );
			
			// Now draw a single pixel where the object is predicted.
			v_world(2) = object[i].nPredictedPos_mm_Y;
			v_world(2) /= 1000;

			v_pix = m_pTransform->TransformToPixel( v_world );

			DrawPixel(	outputImg, 
						(int)(v_pix(1)) / m_nScale, 
						(int)(v_pix(2)) / m_nScale, 
						0x80FFFF );
		}
	}
}

// *************************************************************************

Uint32 CVisObjectsVisualizerEmb::DrawInteger( const Uint32 unNumber, const Int32 x, const Int32 y, const Uint32 color)
{
	Uint8 unNumArray[16];
	Uint32 index = 0;
	Uint32 num = unNumber;

	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();

	// First, arrange the number array
	do
	{
		unNumArray[index] = num % 10;
		num = num/10;
		index++;
	} while ( num > 0 );


	// Then print it to the image
	Uint32 xpos = x;
	for ( Uint32 i=0; i<index; i++ )
		xpos += DrawDigit( pImage, unNumArray[index-i-1], xpos, y, color );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisObjectsVisualizerEmb::DrawFixedpointPercent( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color)
{
	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();

	Uint32 xpos = x;

	xpos += DrawFixedpoint( unNumber * 100, unBase, unPrecision, xpos, y, color );
	xpos += DrawDigit( pImage, 11, xpos, y, color );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisObjectsVisualizerEmb::DrawFixedpoint( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color)
{
	Uint8 unNumArray[16];
	Uint32 index = 0;
	Uint32 num;
	Uint32 prec;
	Uint32 xpos = x;

	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();

	// First, draw the digits before the separator
	num = unNumber >> unBase;
	xpos += DrawInteger( num, xpos, y, color );

	// Draw the separator
	xpos += DrawDigit( pImage, 10, xpos, y, color );

	// Then prepare the digits after the separator
	prec = 0;
	num = unNumber - ( (unNumber >> unBase) << unBase );
	do
	{
		// Multiply by 10.
		num = num * 10;

		// Get the digit.
		unNumArray[index] = num >> unBase;

		// Subtract the whole numbers
		num = num - ( (num >> unBase) << unBase );

		prec++;
		index++;
	} while ( (prec < unPrecision) && (index<16) );

	// Then print it to the image
	for ( Uint32 i=0; i<index; i++ )
		xpos += DrawDigit( pImage, unNumArray[i], xpos, y, color );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisObjectsVisualizerEmb::DrawDigit( Uint32 * pImage, const Char cDigit, const Int32 x, const Int32 y, const Uint32 color)
{
	for ( Uint32 k = 0; k<NUM_WIDTH; k++)
	{
		for ( Uint32 l=0; l<NUM_HEIGHT; l++)
		{
			Int32 xpos = x + k;
			Int32 ypos = l + y;
			
			if ( xpos >= 0 
				&& ypos >= 0
				&& (unsigned)xpos < m_unResultWidth
				&& (unsigned)ypos < m_unResultHeight )
			{
				if ( unNumbers[ cDigit ] [k+l*NUM_WIDTH] != 0)
					pImage[ xpos + ypos*m_unResultWidth ] = color;
			}
		}
	}

	return NUM_WIDTH + 1;
}

// *************************************************************************

void CVisObjectsVisualizerEmb::DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint32 color)
{
	Int32 left = min(x1,x2);
	Int32 right = max(x1,x2);

	// Limit the values
	if ( ( left > (signed)m_unResultWidth ) || ( right < 0 ) )
		return;
	if ( left < 0 )
		left = 0;
	if ( right >= (signed)m_unResultWidth )
		right = m_unResultWidth-1;
	if ( (y < 0 ) || ( y>=(signed)m_unResultHeight ))
		return;		

	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();
	Int32 index = y*m_unResultWidth + left;

	for ( Int32 i=left; i<=right; i++, index++)
	{
		pImage[index] = color;
	}
}
					
// *************************************************************************

void CVisObjectsVisualizerEmb::DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint32 color)
{
	Int32 top = min(y1,y2);
	Int32 bottom = max(y1,y2);

	// Limit values
	if ( (top>(signed)m_unResultHeight) || (bottom<0) )
		return;
	if ( top<0 )
		top = 0;
	if ( bottom >= (signed)m_unResultHeight )
		bottom = m_unResultHeight-1;
	if ( (x<0) || (x>=(signed)m_unResultWidth) )
		return;

	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();
	Int32 index = top*m_unResultWidth + x;

	for ( Int32 i=top; i<=bottom; i++, index+=m_unResultWidth)
	{
		pImage[index] = color;
	}
}

// *************************************************************************

void  CVisObjectsVisualizerEmb::DrawLineOctant0( Uint32 * pImage, const Int nPitch, const Int nWidth, const Int nHeight,
												Int x0, Int y0, Int deltaX, const Int deltaY, const Int Xdirection, const Uint32 color)
{
	Int deltaYx2;
	Int deltaYx2minusdeltaXx2;
	Int errorterm;

	deltaYx2 				= deltaY*2;
	deltaYx2minusdeltaXx2	= deltaYx2 - (Int)(deltaX*2);
	errorterm 				= deltaYx2 - (Int)deltaX;

	if ( (x0 >= 0) && (y0 >= 0) && (x0 < nWidth) && (y0 < nHeight) )
		pImage[ x0 + y0*nPitch ] = color;
	
	while ( deltaX-- != 0 )
	{
		if (errorterm >= 0)
		{
			y0++;
			errorterm += deltaYx2minusdeltaXx2;
		}
		else
			errorterm += deltaYx2;
	
		x0 += Xdirection;

		if ( (x0 >= 0) && (y0 >= 0) && (x0 < nWidth) && (y0 < nHeight) )
			pImage[ x0 + y0*nPitch ] = color;
	}
}

// *************************************************************************

void  CVisObjectsVisualizerEmb::DrawLineOctant1(	Uint32 * pImage, const Int nPitch, const Int nWidth, const Int nHeight,
												Int x0, Int y0, const Int deltaX, Int deltaY, const Int Xdirection, const Uint32 color)
	{

	Int deltaXx2;
	Int deltaXx2minusdeltaYx2;
	Int errorterm;

	deltaXx2				= deltaX * 2;
	deltaXx2minusdeltaYx2	= deltaXx2 - (Int)(deltaY*2);
	errorterm				= deltaXx2 - (Int)deltaY;

	if ( (x0 >= 0) && (y0 >= 0) && (x0 < nWidth) && (y0 < nHeight) )
		pImage[ x0 + y0*nPitch ] = color;

	while ( deltaY-- )
	{
		if (errorterm>=0)
		{
			x0			+= Xdirection;
			errorterm	+= deltaXx2minusdeltaYx2;
		}
		else
			errorterm	+= deltaXx2;
	
		y0++;

		if ( (x0 >= 0) && (y0 >= 0) && (x0 < nWidth) && (y0 < nHeight) )
			pImage[ x0 + y0*nPitch ] = color;
	}
}


// *************************************************************************

#define SWAP(a,b, temp)	temp=a; a=b; b=temp;

void CVisObjectsVisualizerEmb::DrawLine( const Int x_0, const Int y_0, const Int x_1, const Int y_1, const Uint32 color)
{
	Int deltaX, deltaY;
	Int temp;

	Int x0 = x_0;
	Int x1 = x_1;
	Int y0 = y_0;
	Int y1 = y_1;

	// Get the buffer
	Uint32 *	pImage = (Uint32*)m_oportResult.GetBuffer();
	
	// y0 has to be smaller or equal y1, swap points if necessary.
	if (y0>y1)
	{
		SWAP(y0,y1, temp);
		SWAP(x0,x1, temp);
	}

	deltaX = x1 - x0;
	deltaY = y1 - y0;

	// 2 cases possible: from left to right or from right to left. Distinguish those.
	if (deltaX > 0)
	{
		// 2 more cases possible: dy is bigger than dx and vice versa. We have to use 
		// a particular function for each of those.
		if ( deltaX > deltaY )
			DrawLineOctant0( pImage, m_unResultWidth, m_unResultWidth, m_unResultHeight,
							x0, y0, deltaX, deltaY, 1, color);
		else
			DrawLineOctant1( pImage, m_unResultWidth, m_unResultWidth, m_unResultHeight,
							x0, y0, deltaX, deltaY, 1, color);
	}
	else
	{
		// invert deltaX
		deltaX= -deltaX;
		
		// 2 more cases possible: dy is bigger than dx and vice versa. We have to use 
		// a particular function for each of those.
		if (deltaX > deltaY)
			DrawLineOctant0( pImage, m_unResultWidth, m_unResultWidth, m_unResultHeight,
							x0, y0, deltaX, deltaY, -1, color);
		else
			DrawLineOctant1( pImage, m_unResultWidth, m_unResultWidth, m_unResultHeight,
							x0, y0, deltaX, deltaY, -1, color);
	}
}

// *************************************************************************

void CVisObjectsVisualizerEmb::DrawPixel( Uint32 * pImage, const Int32 x, const Int32 y, const Uint32 color)
{
	if ( x>= 0 
		&& y >= 0
		&& (unsigned)x < m_unResultWidth
		&& (unsigned)y < m_unResultHeight )
	{
		pImage[ x + y*m_unResultWidth ] = color;
	}					
}
					
// *************************************************************************
					
// *************************************************************************
