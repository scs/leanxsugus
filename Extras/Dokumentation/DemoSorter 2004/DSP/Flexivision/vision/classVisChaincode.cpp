
#include "classVisChaincode.h"

#include "classVisTracker.h"
#include "classVisFastLabel.h"


const CVisChaincode::Move CVisChaincode::m_aryMoveLUT[CVisChaincode::NUM_DIRECTIONS] = 
													{ 
														{ 1,  0 },	// E
														{ 0, -1 },	// N
														{ -1, 0 },	// W
														{ 0,  1 }	// S
													};

// *************************************************************************
	
CVisChaincode::CVisChaincode( const Char * strName )
	:	CVisComponent( strName, "Chaincode" ),
		m_iportImage( "g8Input", CVisPort::PDT_8BPP_GRAY ),
		m_iportPotatoObjects( "objPotatoes", CVisPort::PDT_DATA ),
		m_oportBorderData( "dataBorder", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST )
{
	m_iportImage.Init( this );
	m_iportPotatoObjects.Init( this );
	m_oportBorderData.Init( this );

	m_oportBorderData.SetBufferSize( sizeof(BorderData) );

}

// *************************************************************************

void CVisChaincode::DoProcessing()
{
	Int32 nStartX;
	Int32 nStartY;

	// Retreive the image's size.
	m_iportImage.GetImageSize( m_unCurrentInputWidth, m_unCurrentInputHeight );

	// Get the buffers;
	Uint8 *	pImage = (Uint8*)m_iportImage.GetBuffer();
	BorderData * pData = (BorderData*)m_oportBorderData.GetBuffer();

	// clear border
	for ( Int x=0; x<m_unResultWidth; x++)
	{
		pImage[ x ] = 0;
		pImage[ x + m_unResultWidth*(m_unResultHeight-1) ] = 0;
	}
	for ( Int y=0; y<m_unResultHeight; y++)
	{
		pImage[ y * m_unResultWidth ] = 0;
		pImage[ y * m_unResultWidth + m_unResultWidth - 1 ] = 0;
	}

	// Find the start position
	if ( ! FindStartPosition( pImage, nStartX, nStartY ) )
	{
		pData->unNumPoints = 0;
		return;
	}

	// Do the coding
	if ( ! FindChainCode( pImage, nStartX, nStartY, pData ) )
	{
		pData->unNumPoints = 0;
		return;
	}
}
	
// *************************************************************************
	
bool CVisChaincode::FindStartPosition( const Uint8 * pImage, Int32 & nPosX, Int32 & nPosY )
{

	PotatoList * pList = (PotatoList*)m_iportPotatoObjects.GetBuffer();

	// We start at a position in the middle of the upper bounding box edge.
	// Since the bounding box is not of the grayscale object, but of the
	// eroded lut object, we have to go north by a certain amount.
	nPosX = (signed)m_unCurrentInputWidth / 2 ;
	nPosY = pList->pObjects[pList->unCurrentPotatoId].unLocalBoundingTop;

	// If we're on background, go south until we find foreground. This should not 
	// be the case.
	while (pImage[ nPosX + nPosY*m_unCurrentInputWidth ] == 0)
	{
		nPosY++;
		
		if (nPosY >= (signed)m_unResultHeight-1 )
			return FALSE;
	}

	// Now go north until we're on background again.
	while (pImage[ nPosX + nPosY*m_unCurrentInputWidth ] != 0)
	{
		nPosY--;

		if ( nPosY <= 0 )
			return FALSE;
	}

	// go back one pixel
	nPosY++;
	return true;
}

// *************************************************************************

bool CVisChaincode::FindChainCode( const Uint8 * pImage, const Int32 nStartX, const Int32 nStartY, BorderData * pData )
{
	// The index variable that is pointing to the current point in the list.
	Uint32	unCurPoint = 0;

	Int32	nTmpX, nTmpY;
	Int32	nNextX, nNextY;
	Uint8   unNumDirChanges;

	Uint8	unCurDir;

	Uint8	unCurVal;

	// Initialize Variables
	nNextX = nStartX;
	nNextY = nStartY;

	unCurDir = 0;

	// Find all Border points
	do 
	{
		// Insert current Position
		pData->unPosX[ unCurPoint ] = nNextX;
		pData->unPosY[ unCurPoint ] = nNextY;
		pData->unDir[ unCurPoint ]  = unCurDir;
		unCurPoint++;

		// Turn to the left
		unCurDir		= (unCurDir + 1) % NUM_DIRECTIONS;
		unNumDirChanges = 0;

		// Find next foreground point
		do 
		{
			// Determine next pixel to look at.
			nTmpX = nNextX + m_aryMoveLUT[ unCurDir ].nX;
			nTmpY = nNextY + m_aryMoveLUT[ unCurDir ].nY;

			// Determine, if the point is actually within the image
			if (	( nTmpX < 0 ) 
				||	( nTmpX >= (signed)m_unCurrentInputWidth )
				||	( nTmpY < 0 )
				||	( nTmpY >= (signed)m_unCurrentInputHeight) )
			{
				// If the point is not inside the image, it is regarded as being background.
				unCurVal = 0;
			}
			else
			{
				// If it is inside the image, read the pixel value of that position.
				unCurVal = pImage[nTmpX + nTmpY * m_unCurrentInputWidth];
			}
			
			// Turn to the right
			unCurDir = (unCurDir - 1 + NUM_DIRECTIONS ) % NUM_DIRECTIONS; 
			unNumDirChanges ++;

			// Do this as long as we've not found a foreground pixel. Abort if we've been turning
			// for more than 360 ° 			
		} while ( (unCurVal == 0) && (unNumDirChanges <= NUM_DIRECTIONS) );

		// Turn one back to the left
		unCurDir = (unCurDir + 1) % NUM_DIRECTIONS;

		// Make sure that we go in a valid direction
		if (unNumDirChanges > NUM_DIRECTIONS)
		{
			// Break the loop, there is a Problem (perhaps only a single Pixel)
			pData->unNumPoints = 0;
			return false;
		}
		else
		{
			// Advance to the next point.
			nNextX = nTmpX;
			nNextY = nTmpY;
		}

		// Bail out if we've reached the maximum number of points!
		if ( unCurPoint == BorderData::MAX_POINTS )
			return false;
		
		// Do this until we arrive at the starting point again.
	} while ( (nNextX != nStartX) || (nNextY != nStartY) );

	// Store the number of points found in the data structure.
	pData->unNumPoints = unCurPoint;
	return true;
}
	
// *************************************************************************


