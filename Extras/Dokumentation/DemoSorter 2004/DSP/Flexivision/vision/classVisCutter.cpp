
#include "classVisCutter.h"

#include "classVisTracker.h"

	
// *************************************************************************

CVisCutter::CVisCutter( const Char * strName )
	:	CVisComponent( strName, "Cutter" ),
		m_iportImage( "g8Input", CVisPort::PDT_8BPP_GRAY ),
		m_iportObjectsList( "objPotatoes", CVisPort::PDT_DATA ),
		m_iportLabelObjects( "objLabels", CVisPort::PDT_DATA ),
		m_oportCutImage( "g8Output", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_DOUBLEBUFFER )
		
{
	m_iportImage.Init( this );
	m_iportObjectsList.Init( this );
	m_iportLabelObjects.Init( this );
	m_oportCutImage.Init( this );
	m_oportCutImage.SetImageSize( CUTIMG_WIDTH, CUTIMG_HEIGHT );

	m_bMoreOutputImages = false;
	m_bIsCopying = false;
}
	
// *************************************************************************
	
void CVisCutter::DoProcessing()
{

	m_bMoreOutputImages = true;

	// Pre-load the current and the next object indices. They have to be -2 and -1 since the 
	// NextOutputImage() will not provide a valid image after the first call.
	m_nCurrentObject = -2;
	m_nNextObject = -1;

	// Get image dimensions from the input buffer.
	m_iportImage.GetImageSize( m_unCurrentInputWidth, m_unCurrentInputHeight );

	// Start copy of the first image.
	m_unCurrentCopyIndex = COPY_DONE;
	NextOutputImage();
}	

// *************************************************************************
	
bool CVisCutter::HasMoreOutputImages()
{
	return m_bIsCopying;
}

// *************************************************************************

void CVisCutter::NextOutputImage()
{
	// Acquire the input image and the potato objects
	Uint8 *							pImage			= (Uint8*)m_iportImage.GetBuffer();
	PotatoList *					pList			= (PotatoList*)m_iportObjectsList.GetBuffer();
	FastLabelObject *	pLabels			= (FastLabelObject *)m_iportLabelObjects.GetBuffer();

	// Wait for the previous copy to finish. 
	WaitCopy( m_unCurrentCopyIndex );

	m_nCurrentObject = m_nNextObject;
	pList->unCurrentPotatoId = m_nCurrentObject;
	m_bIsCopying = false;

	// Swap the double buffer, which will bring the just copied image to foreground
	m_oportCutImage.SwapBuffers();

	// Acquire the back buffer to which the next image will be copied.
	Uint8 *	pOutputImage	= (Uint8 *)m_oportCutImage.GetBackBuffer();

	// DEBUG: only do this on Windows.
#ifdef _WINDOWS
	MemSet( pOutputImage, 255, CUTIMG_WIDTH * CUTIMG_HEIGHT );
#endif

	// Find the next object in the list
	do
	{
		m_nNextObject++;	
	} while ( ( m_nNextObject < PotatoList::MAX_OBJECTS )
			&& ( (! pList->pObjects[m_nNextObject].bTracked) || (! pList->pObjects[m_nNextObject].bValid) ) );

	if ( m_nNextObject >= PotatoList::MAX_OBJECTS )
	{
		// Tell the scheduler that there are no more images left
		// (that means that there is still one valid image in the output buffer
		// at the moment!)
		m_bMoreOutputImages = false;
		return;
	}

	// That means that we'll start a new copy
	m_bIsCopying = true;

	// Get the current input image's width
	Uint32 inputHeight;
	Uint32 inputWidth;
	m_iportImage.GetImageSize( inputWidth, inputHeight );

	// Calc source and dest
	Uint8 *src, *dst;
	Int32 x,y, w,h;
	Int32 dstx, dsty;

	// First, get the center of the object and round the numbers to multiple of four.
	// This is for compatibility with the copy process that is using DMA transfers. Since
	// the input and output image dimensions are also all multiple of four, the resulting
	// line lengths will also be. This results in much better performance of the copy process.	
	// We have to take the label's pixel-accurate information for this.
	Uint32 unCurLabel = pList->pObjects[m_nNextObject].unCurrentLabel;
	/*
	x = (Int32)pList->pObjects[m_nNextObject].unLastSeenPos_X;
	x = ( x+1 ) & ~(0x03);
	y = (Int32)pList->pObjects[m_nNextObject].unLastSeenPos_Y;
	y = ( y+1 ) & ~(0x03);
	*/	
	x = pLabels[unCurLabel].unMx;
	x = ( x+1 ) & ~(0x03);
	y = pLabels[unCurLabel].unMy;
	y = ( y+1 ) & ~(0x03);

	// Calculate the local bounding box coordinates and store them in the objects list...
	Int nLeft, nTop, nRight, nBottom;
	
	nLeft = (Int)pLabels[unCurLabel].unBoundingLeft - x + CUTIMG_WIDTH/2;
	nTop = (Int)pLabels[unCurLabel].unBoundingTop - y + CUTIMG_HEIGHT/2;
	nRight = (Int)pLabels[unCurLabel].unBoundingRight - x + CUTIMG_WIDTH/2;
	nBottom = (Int)pLabels[unCurLabel].unBoundingBottom - y + CUTIMG_HEIGHT/2;

	// ... which we'll check before writing to the object. Just in case.
	pList->pObjects[m_nNextObject].unLocalBoundingLeft		= (Uint32)max( 0, nLeft );
	pList->pObjects[m_nNextObject].unLocalBoundingTop		= (Uint32)max( 0, nTop );
	pList->pObjects[m_nNextObject].unLocalBoundingRight		= (Uint32)min( CUTIMG_WIDTH, nRight );
	pList->pObjects[m_nNextObject].unLocalBoundingBottom	= (Uint32)min( CUTIMG_HEIGHT, nBottom );

	// Now calculate the region that we'll cut out.
	x = x - CUTIMG_WIDTH/2;
	y = y - CUTIMG_HEIGHT/2;
	w = CUTIMG_WIDTH;
	h = CUTIMG_HEIGHT;
	dstx = dsty = 0;

	
	// Crop region to image size
	if ( x<0 )
	{
		dstx = dstx - x;
		// We cannot easily change the width of an image (because of Copy2D()), so just copy more than we need.
		// But, copy one line less than needed, so that we stay in the buffer
		//w = w + x;		
		h--;
		x = 0;
	}
	if ( (x+w) > (Int32)inputWidth )
	{
		// We cannot easily change the width of an image (because of Copy2D()), so just copy more than we need
		// But, copy one line less than needed, so that we stay in the buffer
		//w = inputWidth - x; 
		h--;		
	}
	if ( y<0 )
	{
		dsty = dsty - y;
		h = h + y;
		y = 0;
	}
	// The last line of the target image may not be used. 
	if ( (y+h) > (Int32)inputHeight-1 )
	{
		h = inputHeight - y - 1;
	}

	// Calculate the source and destionation addresses
	src = pImage + ( x + y*inputWidth );
	dst = pOutputImage + ( dstx + dsty*CUTIMG_WIDTH);

	ASSERT( x + (y+h)*inputWidth <= m_iportImage.GetBufferSize() );
	ASSERT( dstx + (dsty)*CUTIMG_WIDTH + h*w <= CUTIMG_WIDTH * CUTIMG_HEIGHT );
	
	// Start the copy process
	m_unCurrentCopyIndex = StartCopy2D(	COPY_2D1D, dst, src, 
										w, h, 
										m_unCurrentInputWidth );

}

// *************************************************************************

Uint32 CVisCutter::GetCurrentPotatoId()
{
	return m_nCurrentObject;
}

// *************************************************************************

// *************************************************************************

// *************************************************************************

