
#include "classVisLabel.h"

#include <math.h>

					
// *************************************************************************

CVisLabel::CVisLabel( const Char * strName, const Int nMaxObjects, const Bool bGenerateImage )
		:	CVisComponent( strName, "Label" ),
			m_iportInput("input", CVisPort::PDT_8BPP_GRAY),
			m_oportLabels("objLabels", CVisPort::PDT_DATA ),
			m_oportOutput("output", CVisPort::PDT_8BPP_GRAY),
			m_propMinObjectArea("MinObjectArea"),
			m_propNeighborhood("Neighborhood"),
			m_propMaxNumLabelsUsed("MaxContsUsed")
{
	// Initialize ports
	m_iportInput.Init( this );
	m_oportLabels.Init( this );
	m_oportOutput.Init( this );

	// Disable the output port if we don't need to generate an image
	if ( ! bGenerateImage )
		m_oportOutput.SetImageSize( 0, 0 );

	m_bGenerateOutputImage = bGenerateImage;

	// Calculate the object buffer's size.
	m_oportLabels.SetBufferSize( sizeof(LabelObject) * nMaxObjects );

	m_propMinObjectArea.Init( this, CVisProperty::PT_INTEGER, &m_nMinObjectArea );
	m_propNeighborhood.Init( this, CVisProperty::PT_INTEGER, &m_nNeighborhood );
	m_propMaxNumLabelsUsed.Init( this, CVisProperty::PT_INTEGER, &m_nMaxNumLabelsUsed);

	m_nMinObjectArea = 10;
	m_nNeighborhood = 4;
	m_nMaxNumLabelsUsed = 0;

	m_nMaxObjects = nMaxObjects;

}

// *************************************************************************

CVisLabel::~CVisLabel()
{
}

// *************************************************************************

void CVisLabel::Prepare()
{
	// These two lines store the current and the last labeled line (labels are 16 bit indices, thus the
	// line must take 16 bit values, too. The two buffers are swapped throughout the processing of the 
	// image, implementing a double buffering technique.
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * sizeof(Uint16),	(Ptr*)&m_pLabelLastLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * sizeof(Uint16),	(Ptr*)&m_pLabelCurrentLine, CVisBufferManager::BUF_FAST );

	// These are buffers for reading in single lines of the grayscale input image.
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth,					(Ptr*)&m_pGrayCurrentLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth,					(Ptr*)&m_pGrayNextLine, CVisBufferManager::BUF_FAST );

	// This buffer is a list that stores for each label, whether it has been merged with another label. 
	// The list is initialized with each entry containing its own index (which means that this label
	// has been merged with itself) and changes throughout the processing of an image.
	CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxObjects * sizeof(Uint16),		(Ptr*)&m_pMergedLabels, CVisBufferManager::BUF_FAST );
}
					
// *************************************************************************

void CVisLabel::DoProcessing()
{
#define SWAP8( p1, p2 ) Uint8* temp8 = p1; p1 = p2; p2 = temp8;
#define SWAP16( p1, p2 ) Uint16* temp16 = p1; p1 = p2; p2 = temp16;

	Uint32	i;
	Uint32	copyGray;
	Uint32	numLabels;

	// Acquire buffers
	Uint8 *			pInputImage = (Uint8*)m_iportInput.GetBuffer();
	LabelObject *	labelObject = (LabelObject*)m_oportLabels.GetBuffer();
	
	// Clear buffers
	for ( i=0; i<m_unResultWidth; i++)
		m_pLabelLastLine[i] = 0;

	// Clear merge list
	for ( i=0; i<(unsigned)m_nMaxObjects; i++)
		m_pMergedLabels[i] = i;

	// Clear objects
	ClearObjects();

	// Preload first line
	copyGray = StartCopy( m_pGrayNextLine, (Ptr)(pInputImage), m_unResultWidth );	

	// Contour 0 is the background, so start with 1.
	numLabels = 1;

	for ( Uint32 line=0; line<m_unResultHeight; line++ )
	{
		WaitCopy( copyGray );
		SWAP8( m_pGrayNextLine, m_pGrayCurrentLine );
		if ( line != m_unResultHeight-1 )
			copyGray = StartCopy( m_pGrayNextLine, (Ptr)(pInputImage + (line+1)*m_unResultWidth), m_unResultWidth );		
	
		if ( m_nNeighborhood != 8 )	
			LabelLine4( m_pLabelLastLine, m_pGrayCurrentLine, line, m_pLabelCurrentLine, labelObject, numLabels, m_pMergedLabels, m_unResultWidth );	
		else
			LabelLine8( m_pLabelLastLine, m_pGrayCurrentLine, line, m_pLabelCurrentLine, labelObject, numLabels, m_pMergedLabels, m_unResultWidth );	

		SWAP16( m_pLabelCurrentLine, m_pLabelLastLine );		
	}

	// Store the maximum number of used labels
	if ( numLabels > (unsigned)m_nMaxNumLabelsUsed )
		m_nMaxNumLabelsUsed = numLabels;
	
	// Finalize objects
	FinalizeObjects();


#ifdef _WINDOWS
	// DEBUG: Draw borders
	for ( i=1; i<labelObject[0].unNumObjects; i++)
	{
		LabelObject * obj = &(labelObject[i]);

		Int nMajAxis, nMinAxis;
		Int fpRatio;
		Int x,y, x1,y1,x2,y2;
		Uint8 c;

		obj->CalcAxes( nMajAxis, nMinAxis );		
		fpRatio = (nMajAxis << 16) / nMinAxis;

		if ( ( fpRatio > (Int)(3.5f * (float)(1<<16)))
			&& ( obj->unArea > 30 ))
			c = 255;
		else
			c = 100;
		
			
		x = obj->unMx;
		y = obj->unMy;
		x1 = obj->unBoundingLeft;
		x2 = obj->unBoundingRight;
		y1 = obj->unBoundingTop;
		y2 = obj->unBoundingBottom;

		// Draw cross in center
		DrawHorizLine( x-5, x+5, y, 255 );
		DrawVertLine( x, y-5, y+5, 255 );

		// Draw bounding box
		DrawHorizLine( x1, x2, y1, c );
		DrawHorizLine( x1, x2, y2, c );
		DrawVertLine( x1, y1, y2, c );
		DrawVertLine( x2, y1, y2, c );

		// Draw axes
		DrawVertLine( x1-1, y2 - nMajAxis, y2, 255 );
		DrawVertLine( x2+1, y2 - nMinAxis, y2, 255 );
	}
#endif
}

// *************************************************************************

void CVisLabel::LabelLine4(	const Uint16 * restrict		pLastLabelLine, 
							const Uint8 * restrict		bCurGrayLine, 
							const Uint16 				unCurLineY, 
							Uint16 * restrict			pCurLabelLine,	
							LabelObject * restrict		pLabelObjects, 
							Uint32 &					unNextLabel, 
							Uint16 * restrict			pMergedLabels,
							const Uint32				unCols )
{
	Uint32 i;

	// X = Pixels' Neighbors
	//	 |B|
	// |A|X|

	// The merge list stores pixels that need to be investigated after the label pass.
	Uint32		mergeList[MAX_OBJECTS_PER_LINE];
	Uint32		nextMergeIndex = 0;
	
	// Neighbours of current pixel	               
	Uint16	unA, unB;
	Uint8	unGrayX;

	// Blank out the left and the rightmost pixels in the result
	pCurLabelLine[0] = 0;
	pCurLabelLine[unCols-1] = 0;

		
	// *******************************************
	//  Label all pixels of the line.
	// *******************************************
	// Pixel labels are not yet merged, the pixels at which a merging
	// occurs are just stored in a the MergeList and processed later.

	// Go through all pixels except the outermost ones and label from left
	Uint16 unRes = 0;
	for ( i=1; i<unCols-1; i++ )
	{
		unA = unRes;
		unB = pLastLabelLine[i];
		unGrayX = bCurGrayLine[i];
				
		if ( unGrayX != 0)
		{
			if ( unA || unB )
			{
				// Merge?
				if ( ( unA && unB ) && (unA != unB))
				{
					// Store this pixel
					mergeList[ nextMergeIndex++ ] = i;
					unRes = unB;

					// Safety, don't go too far.
					if ( nextMergeIndex == MAX_OBJECTS_PER_LINE )
						nextMergeIndex--;
				}
				// don't merge, just take the label that is not zero.
				else
				{
					unRes = (unA != 0) ? unA : unB;
				}
			}
			// new label
			else
			{
				// Init label and set the pixel
				pLabelObjects[unNextLabel].InitLabel();
				unRes = unNextLabel;

				// Go on to next label
				unNextLabel++;

				// Safety: don't go too far.
				if ( unNextLabel == (unsigned)m_nMaxObjects )
					unNextLabel--;
			}
		}
		else
			unRes = 0;

		pCurLabelLine[i] = unRes;
	}	

	// *******************************************
	//  Go through the mergelist
	// *******************************************
	// Process all mergings in the line.
	Uint16 unMin, unMax;
	for ( i=0; i< nextMergeIndex; i++)
	{
		Uint16 pix = mergeList[i];
		unA = pCurLabelLine[pix-1];
		unB = pLastLabelLine[pix];	
		unMin = min(unA, unB);
		unMax = max(unA, unB);

		ASSERT( unA != 0);
		ASSERT( unB != 0);
		ASSERT( unA != unB );

		// Merge the labels...
		MergeLabels( unMax, unMin, pLabelObjects, unNextLabel, pMergedLabels );

		// ... this will result in one of the upper labels being freed (since we
		// always merge the upper labels into the lower ones.
		// nextMergeIndex--;

	}

	// *******************************************
	//  Resolve final pixel labels.
	// *******************************************
	Uint16			label;
	LabelObject *	obj;

	for (i=1; i<unCols-1; i++ )
	{
		// Get the resolved label through the mergedLabels list
		label = pMergedLabels[ pCurLabelLine[i] ];

		if ( label != 0 )
		{
			// Store the resolved label
			pCurLabelLine[ i ] = label;

			// Get the gray value
			unGrayX = bCurGrayLine[i];

			obj = pLabelObjects + label;

			// Add information to label object
			obj->unArea++;
			obj->unMx += i;
			obj->unMy += unCurLineY;
			obj->unMxx += i*i;
			obj->unMyy += unCurLineY * unCurLineY;
			obj->nMxy += i*unCurLineY;			

			obj->unBoundingTop		= min( obj->unBoundingTop,		unCurLineY );
			obj->unBoundingBottom	= max (obj->unBoundingBottom,	unCurLineY );
			obj->unBoundingLeft		= min( obj->unBoundingLeft,		i );
			obj->unBoundingRight	= max( obj->unBoundingRight ,	i );
		}
	}
}

void CVisLabel::LabelLine8(	const Uint16 * restrict		pLastLabelLine, 
							const Uint8 * restrict		bCurGrayLine, 
							const Uint16 				unCurLineY, 
							Uint16 * restrict			pCurLabelLine,	
							LabelObject * restrict		pLabelObjects, 
							Uint32 &					unNextLabel, 
							Uint16 * restrict			pMergedLabels,
							const Uint32				unCols )
{
	Uint32 i;

	//////////////////////////////////////
	// X = Pixels' Neighbors
	// 
	// |B|C|D|
	// |A|X|
	//////////////////////////////////////

	// The merge list stores pixels that need to be investigated after the label pass.
	Uint32		mergeList[MAX_OBJECTS_PER_LINE/**2*/];
	Uint32		nextMergeIndex = 0;
	
	// Neighbours of current pixel	               
	Uint16	unA, unB, unC, unD;
	Uint8	unGrayX;

	// Blank out the left and the rightmost pixels in the result
	pCurLabelLine[0] = 0;
	pCurLabelLine[unCols-1] = 0;

		
	// *******************************************
	//  Label all pixels of the line.
	// *******************************************
	// Pixel labels are not yet merged, the pixels at which a merging
	// occurs are just stored in a the MergeList and processed later.

	// Go through all pixels except the outermost ones and label from left
	Uint16 unRes = 0;
	Uint16 unTmp = 0;
	unB = pLastLabelLine[0];
	unC = pLastLabelLine[1];
	unD = pLastLabelLine[2];

	for ( i=1; i<unCols-1; i++ )
	{
		unGrayX = bCurGrayLine[i];

		if ( unGrayX != 0)
		{
			unA = pCurLabelLine[ i-1 ];
			unB = pLastLabelLine[ i-1 ];
			unC = pLastLabelLine[ i ];
			unD = pLastLabelLine[ i+1 ];
			/*
			// A is the result of the last pixel
			unA = unRes;

			// B,C are C,D of last pixel
			unB = unC;
			unC = unD;

			// read D
			unD = pLastLabelLine[i+1];
*/
			// If there are other labels around, see if we must merge
			if (unA || unB || unC || unD)
			{		
				// find smallest contour index which is not 0
				Uint16 unMin;
				if (unA < unB && unA != 0) 
					unMin = unA;
				else if (unB < unA && unB != 0) 
					unMin = unB;
				else 
					unMin = max(unA, unB);

				if (unC < unD && unC != 0) 
					unTmp = unC;
				else if (unD < unC && unD != 0) 
					unTmp = unD;
				else 
					unTmp = max(unC, unD);

				if (unMin < unTmp && unMin != 0) 
					unMin = unMin;
				else if (unTmp < unMin && unTmp != 0) 
					unMin = unTmp;
				else 
					unMin = max(unMin, unTmp);
				
				//////////////////////////////////////////////
				// Is Merging a possibility?
				//
				// (A && D) or (B && D) 
				// are the only possibilities
				///////////////////////////////////////////
				if (unA && unD) // merge is possible
				{
					// Always merge the larger lable into the smaller.
					if (unA > unMin)
					{
						mergeList[ nextMergeIndex ] = unA;
						mergeList[ nextMergeIndex+1 ] = unMin;
						nextMergeIndex += 2;
					}
					else if (unD > unMin) 
					{
						mergeList[ nextMergeIndex ] = unD;
						mergeList[ nextMergeIndex+1 ] = unMin;
						nextMergeIndex += 2;
					}

						
					// Safety, don't go too far.
					if ( nextMergeIndex == MAX_OBJECTS_PER_LINE )
						nextMergeIndex-=2;
					
				} // merge A and D

				if (unB && unD) // merge is possible
				{				
					// Store the merge. Always merge the larger lable into the smaller.
					if (unB > unMin)
					{
						mergeList[ nextMergeIndex ] = unB;
						mergeList[ nextMergeIndex+1 ] = unMin;
						nextMergeIndex += 2;
					}
					else if (unD > unMin) 
					{
						mergeList[ nextMergeIndex ] = unD;
						mergeList[ nextMergeIndex+1 ] = unMin;
						nextMergeIndex += 2;
					}

						
					// Safety, don't go too far.
					if ( nextMergeIndex == MAX_OBJECTS_PER_LINE )
						nextMergeIndex-=2;

				} // merge B and D


				unRes = unMin;

			} // if labels around			
			else
			// new label
			{
				// Init label and set the pixel
				pLabelObjects[unNextLabel].InitLabel();
				unRes = unNextLabel;

				// Go on to next label
				unNextLabel++;

				// Safety: don't go too far.
				if ( unNextLabel == (unsigned)m_nMaxObjects )
					unNextLabel--;

			} // if no other labels around

		} // if pixel is non-zero
		else
		{
			// Mark the pixel as background.
			unRes = 0;
		}

		pCurLabelLine[i] = unRes;
	}	

	// *******************************************
	//  Go through the mergelist
	// *******************************************
	// Process all mergings in the line.
	Uint16 unMin, unMax;
	for ( i=0; i< nextMergeIndex; i+=2)
	{
		unA = mergeList[i];
		unB = mergeList[i+1];	
		unMin = min(unA, unB);
		unMax = max(unA, unB);

		ASSERT( unA != 0);
		ASSERT( unB != 0);
		ASSERT( unA != unB );

		// Merge the labels...
		MergeLabels( unMax, unMin, pLabelObjects, unNextLabel, pMergedLabels );

		// ... this will result in one of the upper labels being freed (since we
		// always merge the upper labels into the lower ones.
		// nextMergeIndex--;
	}

	// *******************************************
	//  Resolve final pixel labels.
	// *******************************************
	Uint16			label;
	LabelObject *	obj;

	for (i=1; i<unCols-1; i++ )
	{
		// Get the resolved label through the mergedLabels list
		label = pMergedLabels[ pCurLabelLine[i] ];

		if ( label != 0 )
		{
			// Store the resolved label
			pCurLabelLine[ i ] = label;

			// Get the gray value
			unGrayX = bCurGrayLine[i];

			obj = pLabelObjects + label;

			// Add information to label object
			obj->unArea++;
			obj->unMx += i;
			obj->unMy += unCurLineY;
			obj->unMxx += i*i;
			obj->unMyy += unCurLineY * unCurLineY;
			obj->nMxy += i*unCurLineY;			

			obj->unBoundingTop		= min( obj->unBoundingTop,		unCurLineY );
			obj->unBoundingBottom	= max (obj->unBoundingBottom,	unCurLineY );
			obj->unBoundingLeft		= min( obj->unBoundingLeft,		i );
			obj->unBoundingRight	= max( obj->unBoundingRight ,	i );
		}
	}
}

// *************************************************************************

void CVisLabel::MergeLabels( const Uint16 oldLabel, const Uint16 newLabel, LabelObject * restrict pLabelObjects, const Uint32 numLabels, Uint16 * restrict pMergedLabels )
{
	LabelObject * oldObj;
	LabelObject * newObj;

	oldObj = pLabelObjects + oldLabel;
	newObj = pLabelObjects + newLabel;

	// Take all indices that point to label1 and bend them to label2.
	// TODO::: see if this is really necessary. Doesn't bending the oldLabel suffices?
	for (Uint32 i=1; i<numLabels; i++)
	{
		// This one we must bend
		if (pMergedLabels[i] == oldLabel)
		{
			oldObj = pLabelObjects + oldLabel;

			// Set the conversion index in the array
			pMergedLabels[i] = newLabel;
		}
	}

	// Now merge the label data
	newObj->Merge( oldObj );
}

// *************************************************************************

void CVisLabel::FinalizeObjects()
{
	Uint32 count;
	Uint32 i;

	// Acquire buffer
	LabelObject *	objects = (LabelObject*)m_oportLabels.GetBuffer();

	// Go through objects list with two indices. Move all used objects down as far
	// as possible, so that all can be found at indices following and including 1.

	count = 1; // entry 0 is background.
	i = 1;

	while( (count < (unsigned)m_nMaxObjects-1) && (i < ((unsigned)m_nMaxObjects)) ) 
	{
		// Only add objects that have a minimal size
	  	if ( (objects[i].bValid) && (objects[i].unArea >= (unsigned)m_nMinObjectArea) )
	   	{ 
	   		// Mask out objects that touch the image borders.
	   		if ( (objects[i].unBoundingTop > 0) && (objects[i].unBoundingBottom < m_unResultHeight-1 )
	   			 && (objects[i].unBoundingLeft > 0) && (objects[i].unBoundingRight < m_unResultWidth-1 ) )
	   		{
			
				objects[count].bValid = true;
				objects[count].unArea	= objects[i].unArea;
/*
				// Calculate the center of gravity of the first momentum
				objects[count].unMx		= objects[i].unMx / objects[i].unArea;
				objects[count].unMy		= objects[i].unMy / objects[i].unArea;

				// Normalize the higher order momentums to the center of gravity
				objects[count].unMxx	= objects[i].unMxx / objects[i].unArea;
				objects[count].unMyy	= objects[i].unMyy / objects[i].unArea;
				objects[count].unMxy	= objects[i].unMxy / objects[i].unArea;
				objects[count].unMxx	= ( objects[count].unMxx - objects[count].unMx * objects[count].unMx );
				objects[count].unMyy	= ( objects[count].unMyy - objects[count].unMy * objects[count].unMy );
				objects[count].unMxy	= ( objects[count].unMxy - objects[count].unMx * objects[count].unMy );
				*/

				
				// Normalize the higher order momentums to the center of gravity
				// Note: we change to signed calculation since the mxy term may get negative
				Int nArea = (signed)objects[i].unArea;
				Int mxx, myy, mxy;
				// We need floating point here because of simplicity. The mx^2 and my^2 terms tend range from very
				// small values to rather large ones, so that 32 bit arithmetics won't suffice for larger objects
				// and doing the /nArea division first will mess up small objects.
				mxx = (Uint32)( (float)objects[i].unMxx - ((float)objects[i].unMx * (float)objects[i].unMx) / (float)nArea) / nArea;
				myy = (Uint32)( (float)objects[i].unMyy - ((float)objects[i].unMy * (float)objects[i].unMy) / (float)nArea) / nArea;
				mxy = (Int)( (float)objects[i].nMxy - (float)(objects[i].unMx * (float)objects[i].unMy) / (float)nArea) / nArea;
				objects[count].unMxx	= (mxx > 0) ? mxx : 0;
				objects[count].unMyy	= (myy > 0) ? myy : 0;
				//objects[count].nMxy		= (mxy > 0) ? mxy : 0;
				objects[count].nMxy		= mxy;

				// Calculate the center of gravity of the first momentum
				objects[count].unMx		= objects[i].unMx / objects[i].unArea;
				objects[count].unMy		= objects[i].unMy / objects[i].unArea;

				// Copy bounding box.
				objects[count].unBoundingLeft	= objects[i].unBoundingLeft;
				objects[count].unBoundingRight	= objects[i].unBoundingRight;
				objects[count].unBoundingTop	= objects[i].unBoundingTop;
				objects[count].unBoundingBottom = objects[i].unBoundingBottom;

				// onto the next...		
				count++;
		
			}
		}

	   i++;
	}

	// Store the number of labels in the contour no. 0.
	objects[0].unNumObjects = count;

}


// *************************************************************************

void LabelObject::InitLabel()
{
	// Initialize the object structure.
	unArea = 0;
	unMx = 0;
	unMy = 0;
	unMxx = 0;
	unMyy = 0;
	nMxy = 0;
	unBoundingTop = 0x0FFFFFFF;
	unBoundingBottom = 0;
	unBoundingLeft = 0x0FFFFFFF;
	unBoundingRight = 0;

	bValid = true;
}


// *************************************************************************

void LabelObject::Merge( LabelObject * wasteObj )
{
	// Now sum up the object info
	unArea	+= wasteObj->unArea;
	unMx	+= wasteObj->unMx;
	unMy	+= wasteObj->unMy;
	unMxx	+= wasteObj->unMxx;
	unMyy	+= wasteObj->unMyy;
	nMxy	+= wasteObj->nMxy;
	
	unBoundingTop = min( unBoundingTop,			wasteObj->unBoundingTop );
	unBoundingBottom = max (unBoundingBottom,	wasteObj->unBoundingBottom );
	unBoundingLeft = min( unBoundingLeft,		wasteObj->unBoundingLeft );
	unBoundingRight = max( unBoundingRight ,	wasteObj->unBoundingRight );

	// invalidate old label
	wasteObj->bValid = false;
}
					
// *************************************************************************

bool LabelObject::CalcAxes( Int & nMajAxis, Int & nMinAxis )
{
	// M = [mxx, mxy; mxy, myy]
	// Eigenvalues lambda of M are related to the length of the Axes:
	//   lambda = ((mxx + myy) +- sqrt((mxx+myy)^2 - 4*(mxx*myy-mxy^2))) / 2
	//   Length of Axis = 4 * sqrt(lambda)
	// Eigenvectors of M are the direction of the Axes
	//   (mxx - labda) * x + mxy * y = 0
	// Condition: The FinalizeObjects() function must already be called on the contour object

	float dSq, dLambda[2];
//	float dXb,dYb, dXe,dYe;
	float dLeng;

	// Calcualte the Eigenvalues of Matrix M
	dSq = (float)sqrt((float)((unMxx + unMyy) * (unMxx + unMyy) - 4*(unMxx * unMyy - nMxy * nMxy)));
    dLambda[0] = (float)(((unMxx + unMyy) + dSq) / 2.0);
    dLambda[1] = (float)(dLambda[0] - dSq);

	// Calculate Eigenvectors of the Matrix M
	Uint32 i;
	for (i = 0; i < 2; i++)
	{
		/*
		// Calculate the Eigenvector for Lambda[i]
		if (unMxy == 0.0)       // Object is symmetric and horizontal or vertical
		{
			if (unMxx > unMyy)   // Longer Axis (Major Axis) is horizontal
			{
				if (i == 0)
				{ dYb = 0; dXb = 1; }
				else
				{ dYb = 1; dXb = 0; }
			}
			else               // Longer Axis (Major Axis) is vertical
			{
				if (i == 0)
				{ dYb = 1; dXb = 0; }
				else
				{ dYb = 0; dXb = 1; }
			}
		}
		else if((dLambda[i] - unMxx) != 0)
		{
			dYb = 1;                            // choose y
			dXb = unMxy / (dLambda[i] - unMxx);   // calculate x
		} 
		else
		{
			dXb = 1;
			dYb = 0;
		}
*/
		// Set the length of the Eigenvector to the length of the Axis.
		dLeng = 2.0f * (float)sqrt(fabs(dLambda[i]));      // Length of the Axis
		
		/*
		dSq = sqrt(dXb * dXb + dYb * dYb);   // Current Length of the Eigenvector
		dXb = dXb * dLeng / dSq;             // Normalize Eigenvector
		dYb = dYb * dLeng / dSq;
		dXe = - dXb;
		dYe = - dYb;

		// move to centre of gravity
		dXb += unMx;
		dYb += unMy;
		dXe += unMx;
		dYe += unMy;
*/
		if (i == 0)
		{
			/*
			dMajAxisBx = dXb;
			dMajAxisBy = dYb;
			dMajAxisEx = dXe;
			dMajAxisEy = dYe;
			*/

			nMajAxis = max( 1, (Int)(2*dLeng));
		}
		else
		{
			/*
			dMinAxisBx = dXb;
			dMinAxisBy = dYb;
			dMinAxisEx = dXe;
			dMinAxisEy = dYe;
			*/

			nMinAxis = max( 1, (Int)(2*dLeng));
		}
	}

	return true;
}
					
// *************************************************************************

void CVisLabel::ClearObjects()
{
	// Acquire output buffer
	LabelObject * obj = (LabelObject*)m_oportLabels.GetBuffer();

	for (Uint32 i=0; i<(unsigned)m_nMaxObjects; i++)
	{
		obj[i].bValid = false;
	}
}
					
// *************************************************************************

// DEBUG stuff:


void CVisLabel::DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint8 color)
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
	if ( (y < 0 ) || ( y>(signed)m_unResultHeight ))
		return;		

	// Get the buffer
	Uint8 *	pImage = (Uint8*)m_iportInput.GetBuffer();
	Int32 index = y*m_unResultWidth + left;

	for ( Int32 i=left; i<=right; i++, index++)
	{
		pImage[index] = color;
	}
}
					
// *************************************************************************

void CVisLabel::DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint8 color)
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
	if ( (x<0) || (x>(signed)m_unResultWidth) )
		return;

	// Get the buffer
	Uint8 *	pImage = (Uint8*)m_iportInput.GetBuffer();
	Int32 index = top*m_unResultWidth + x;

	for ( Int32 i=top; i<=bottom; i++, index+=m_unResultWidth)
	{
		pImage[index] = color;
	}
}
					
// *************************************************************************

void CVisLabel::DrawPixel( Uint8 * pImage, const Int32 x, const Int32 y, const Uint32 color)
{
	if ( x>= 0 
		&& y >= 0
		&& (unsigned)x < m_unResultWidth
		&& (unsigned)y < m_unResultHeight )
	{
		pImage[ x + y*m_unResultWidth ] = color;
	}					
}

