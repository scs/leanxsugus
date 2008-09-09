

#include "classVisFastLabel.h"

#include <math.h>


CVisFastLabel::CVisFastLabel( const Char * strName, Int nMaxSegments, Int nHistogramWidth, Int nMaxObjects, Uint32 unFlags )
	:	CVisComponent( strName, "FastLabel" ),
		m_iportInput( "input", CVisPort::PDT_8BPP_GRAY ),
		m_oportLabels("objLabels", CVisPort::PDT_DATA ),
		m_propMinObjectArea( "MinObjectArea" ),
		m_propMinDistanceToBorder("MinDistToBorder"),
		m_propMaxNumLabelsUsed("MaxLabelsUsed"),
		m_propMaxNumSegmentsUsed("MaxSegmentsUsed")
{
	m_iportInput.Init( this );
	m_oportLabels.Init( this );

	m_propMinObjectArea.Init( this, CVisProperty::PT_INTEGER, &m_nMinObjectArea );
	m_nMinObjectArea = 10;

	m_propMinDistanceToBorder.Init( this, CVisProperty::PT_INTEGER, &m_nMinDistanceToBorder );
	m_nMinDistanceToBorder = 1;

	m_propMaxNumLabelsUsed.Init( this, CVisProperty::PT_INTEGER, &m_nMaxNumLabelsUsed);
	m_nMaxNumLabelsUsed = 0;

	m_propMaxNumSegmentsUsed.Init( this, CVisProperty::PT_INTEGER, &m_nMaxNumSegmentsUsed);
	m_nMaxNumSegmentsUsed = 0;

	// Calculate the object buffer's size.
	m_oportLabels.SetBufferSize( sizeof(FastLabelObject) * nMaxObjects );

	// Round max segments down to multiple of four, so we can optimize the algorithm.
	nMaxSegments = nMaxSegments & ~0x03;
	
	m_nMaxSegments = nMaxSegments;
	m_nHistogramWidth = nHistogramWidth;
	m_nMaxObjects = nMaxObjects;
	m_unFlags = unFlags;

	m_pTransform = NULL;

	// Set the histogram width to 0, if the feature is disabled.
	if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) == 0 )
		m_nHistogramWidth = 0;
	
	
	
	/*
	m_nProf1 = this->NewProfileTask("FastLabel: FindSegments per line");
	m_nProf2 = this->NewProfileTask("FastLabel: Connect");
	m_nProf3 = this->NewProfileTask("FastLabel: Resolve");
	m_nProf4 = this->NewProfileTask("FastLabel: Label");
	*/
}

// *************************************************************************

void CVisFastLabel::Prepare()
{
	// Request segments buffer
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth, (Ptr*)&m_pGrayNextLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxSegments * sizeof( LineSegment ), (Ptr*)&m_segCurrentSegments, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxSegments * sizeof( LineSegment ), (Ptr*)&m_segLastSegments, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxSegments * m_nMaxSegments * 4 * sizeof( Uint8 ), (Ptr*)&m_pConnectivity, CVisBufferManager::BUF_FAST );	

	// Allocate and distribute the histogram buffers to the label objects.
	if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) != 0 )
	{
		CVisBufferManager::Instance()->RequestBuffer( this, m_nHistogramWidth * (m_nMaxSegments) * sizeof( Uint16 ), (Ptr*)&m_pCurrentSegmentHistograms, CVisBufferManager::BUF_FAST );
		CVisBufferManager::Instance()->RequestBuffer( this, m_nHistogramWidth * (m_nMaxObjects) * sizeof( Uint16 ), (Ptr*)&m_pHistograms, CVisBufferManager::BUF_FAST );			

		FastLabelObject * pLabelObjects = (FastLabelObject*)m_oportLabels.GetBuffer();
		for ( Int i=0; i<m_nMaxObjects; i++)
			pLabelObjects[i].pHistogram = m_pHistograms + ( m_nHistogramWidth * i );
	}

	// Allocate and distribute the buffer needed for the transformed coordinates, if requested.
	if ( (m_unFlags & LABEL_TRANSFORM_COORDS) != 0 )
	{
		CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxObjects * sizeof( TransformedCoords ), (Ptr*)&m_pTransformedCoords, CVisBufferManager::BUF_FAST );

		FastLabelObject * pLabelObjects = (FastLabelObject*)m_oportLabels.GetBuffer();
		for ( Int i=0; i<m_nMaxObjects; i++)
		{
			pLabelObjects[i].pTransformedCoords = m_pTransformedCoords + i;
			
			pLabelObjects[i].pTransformedCoords->Init();
		}
	}
	
}

// *************************************************************************

void CVisFastLabel::SetTransform( CVisTransform * transform )
{
	m_pTransform = transform;
}

// *************************************************************************

void CVisFastLabel::DoProcessing()
{
	Int nNumCurSegments;
	Int nNumLastSegments;
	Int nNumLabels;

	nNumCurSegments = 0;
	nNumLastSegments = 0;

	// We'll have to start with label 1, since 0 is used for background and general information.
	nNumLabels = 1;

	// Acquire buffers
	Uint8 *				pInputImage = (Uint8*)m_iportInput.GetBuffer();		
	FastLabelObject *	pLabelObjects = (FastLabelObject*)m_oportLabels.GetBuffer();

	// Clear the segments histograms, if histogram acquisition is requested. The label objects' histograms
	// are cleared in InitLabel() and don't need to be handled here.
	if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) != 0 )
	{
		MemSet( m_pCurrentSegmentHistograms, 0, m_nHistogramWidth * m_nMaxSegments * sizeof( Uint16 ) );		
	}

	// Get the dimension of the input image.
	Uint32 unInputWidth;
	Uint32 unInputHeight;
	m_iportInput.GetImageSize( unInputWidth, unInputHeight );
		
	for ( Uint32 line=0; line<unInputHeight; line++ )
	{
		// Load next line
		QuickCopy( m_pGrayNextLine, (Ptr)(pInputImage + (line * unInputWidth)), unInputWidth );	

					
		// Find the RLE segments and their histogram
//		StartProfileTask( m_nProf1 );
		nNumCurSegments = FindSegments( m_pGrayNextLine, m_segCurrentSegments, m_nMaxSegments, unInputWidth );
//		StopProfileTask( m_nProf1 );

		if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) != 0 )
			FindSegmentsHistogram( m_pGrayNextLine, m_segCurrentSegments, nNumCurSegments, m_nHistogramWidth );
		
		
		// Connect the current segments to the previous line
//		StartProfileTask( m_nProf2 );

		if ( (m_unFlags & LABEL_NEIGHBORHOOD_8) != 0 )
			ConnectSegments8( m_segLastSegments, nNumLastSegments, m_segCurrentSegments, nNumCurSegments, m_pConnectivity );
		else
			ConnectSegments4( m_segLastSegments, nNumLastSegments, m_segCurrentSegments, nNumCurSegments, m_pConnectivity );
//		StopProfileTask( m_nProf2 );

/*
		if ( nNumCurSegments > 0 )
			TRACE("Segments found!\n");
		TRACE("Before: \n");
		PrintEquivalenceMatrix( m_pConnectivity, nNumLastSegments, nNumCurSegments );
*/		

		// Resolve the equivalence matrix
//		StartProfileTask( m_nProf3 );		
		
		ResolveEquivalence( m_pConnectivity, nNumLastSegments, nNumCurSegments);		
//		StopProfileTask( m_nProf3 );
/*
		TRACE("After: \n");
		PrintEquivalenceMatrix( m_pConnectivity, nNumLastSegments, nNumCurSegments );
*/

//		StartProfileTask( m_nProf4 );

		// Now resolve all merges in the upper line imposed by the current lower line.
		MergeUpperSegments( m_pConnectivity, 
							m_segLastSegments, nNumLastSegments, 
							nNumCurSegments, 
							pLabelObjects );

		// ... and label the current line.
		LabelLowerSegments( line, m_pConnectivity, 
							m_segLastSegments, nNumLastSegments, 
							m_segCurrentSegments, nNumCurSegments, 
							pLabelObjects, nNumLabels );

		// Store the maximum number of used segments
		if ( nNumCurSegments > m_nMaxNumSegmentsUsed )
			m_nMaxNumSegmentsUsed = nNumCurSegments;
			
//		StopProfileTask( m_nProf4 );
				
		// Swap the lines.
		Ptr p = (Ptr)m_segCurrentSegments;
		m_segCurrentSegments = m_segLastSegments;
		m_segLastSegments = (LineSegment*)p;
		nNumLastSegments = nNumCurSegments;
	}

	FinalizeObjects( nNumLabels, unInputWidth, unInputHeight );

	// Store the maximum number of used labels
	if ( nNumLabels > m_nMaxNumLabelsUsed )
		m_nMaxNumLabelsUsed = nNumLabels;

	//	LogMsg("%d labels used, %d objects found.", nNumLabels, pLabelObjects[0].unNumObjects );

#ifdef _WINDOWS
	// DEBUG: Draw borders
	/*
	for ( Uint32 i=1; i<pLabelObjects[0].unNumObjects; i++)
	{
		FastLabelObject * obj = &(pLabelObjects[i]);

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
	*/
#endif

}

// *************************************************************************

Int CVisFastLabel::FindSegments( const Uint8 * restrict pGrayLine, LineSegment * restrict pSegments, const Int nMaxSegments, const Int nCols )
{
	Int		nCurSegment;
	Int		col;

	Uint8	unLast = 0;
	Uint8	unCur = 0;

	// Clear all segments
	MemSet( pSegments, 0, nMaxSegments * sizeof( LineSegment ) );

	// Start with segment 0.
	nCurSegment = 0;

	// RLE the line
	col = 0;	
	
	
	//#pragma MUST_ITERATE(4,,4)
	//#pragma UNROLL(4)
	while( col<nCols )
	{
		// Read current pixel
		unLast = unCur;		
		unCur = pGrayLine[col];
		
		// If this is the end of a contour, advance the segment index
		if ( (unLast != 0) && (unCur == 0 ) )
			nCurSegment++;
			

		// if this is the start of a new segment, store the position
		else if ( (unLast == 0) && (unCur != 0) )
			pSegments[nCurSegment].unBegin = col;

		// Store the end of the current segment in any case
		pSegments[nCurSegment].unEnd = col;
		
		// Guard the segments buffer
		if ( nCurSegment >= m_nMaxSegments-1 )
			nCurSegment--;
		
		// Advance column
		col++;		
		
	}

	// Finish the current segment, if it reaches to the end of the line
	if ( unCur != 0 )
		nCurSegment++;
		
		
	// Return the number of segments found.
	return nCurSegment;
}

// *************************************************************************

void CVisFastLabel::FindSegmentsHistogram( const Uint8 * restrict pGrayLine, LineSegment * restrict pSegments, const Int nNumSegments, const int nHistogramWidth )
{

	Uint8 	aryHistogram1[256];
	Uint8 	aryHistogram2[256];
	Uint8 	aryHistogram3[256];
	Uint8 	aryHistogram4[256];

	for (Int i=0; i<nNumSegments; i++)
	{
		// Clear histograms
		MemSet( aryHistogram1, 0, sizeof(Uint16) * m_nHistogramWidth );
		MemSet( aryHistogram2, 0, sizeof(Uint16) * m_nHistogramWidth );
		MemSet( aryHistogram3, 0, sizeof(Uint16) * m_nHistogramWidth );
		MemSet( aryHistogram4, 0, sizeof(Uint16) * m_nHistogramWidth );
		
		
		Int begin, end;
		
		begin = pSegments[i].unBegin;
		end = pSegments[i].unEnd;
		
		Int col	= begin;
		
		// Line up to 4
		if ( (col & 0x03) != 0 )
		{
			Int n = min ( (col + 3) & ~0x03, end );
			while( col<n )
			{
				aryHistogram1[ pGrayLine[col] ] ++;
				col++;
			}
		}
		
		// Process 4 pixels at a time
		while( col < end-2 )
		{
		
			Uint32 unNextPixels = *((Uint32*)(&(pGrayLine[col])));
			
			// Don't have to manually call EXTU() intrinsic. It is done by the compiler.
			aryHistogram1[ (unNextPixels & 0x000000FF) 	    ] ++;
			aryHistogram2[ (unNextPixels & 0x0000FF00) >> 8 ] ++;
			aryHistogram3[ (unNextPixels & 0x00FF0000) >> 16] ++;
			aryHistogram4[ (unNextPixels & 0xFF000000) >> 24] ++;
			
			col+=4;
		}
		
		// Process the last (up to three) pixels...
		while( col <= end )
		{
			aryHistogram1[ pGrayLine[col] ] ++;
			col++;
		}			
		
		// Sum up the histograms.
		Uint16 * pHistogram;

		pHistogram = m_pCurrentSegmentHistograms + m_nHistogramWidth*i;
		for ( Int j=0; j<nHistogramWidth; j++ )
		{
			pHistogram[j] = (Uint16)aryHistogram1[j] + (Uint16)aryHistogram2[j] + (Uint16)aryHistogram3[j] + (Uint16)aryHistogram4[j];
		}

		// ASSERT( end-begin+1 == pHistogram[1] + pHistogram[2] + pHistogram[3] );
	}

}

// *************************************************************************

void CVisFastLabel::ConnectSegments4( const LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
									const LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments,
									Uint8 * restrict pConnectivity )
{
	// See describtion in ConnectSegments8() for more information.

	//const Int nOffs = m_nMaxSegments * 2;
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;

	// Clear the connectivity matrix.
	MemSet( pConnectivity, 0, m_nMaxSegments * m_nMaxSegments * 4 );

	// Match segments
	for ( Uint32 i = 0; i<unNumUpperSegments; i++)
	{
		for ( Uint32 j = 0; j<unNumLowerSegments; j++ )
		{
			{
				Int nIndex;
				Int nConnected;
	
				// See if those two segments are connected			
				Bool bConnected1 = !( pUpperSegments[i].unBegin > pLowerSegments[j].unEnd );
				Bool bConnected2 = !( pUpperSegments[i].unEnd < pLowerSegments[j].unBegin );
				nConnected = ( bConnected1 && bConnected2 ) ? 1 : 0;
				
				// Calculate the index ...
				//nIndex = (nOffs * i) + (j + m_nMaxSegments);
				nIndex = (nOffs * i) + (j + unNumUpperSegments );
						
				// .. and make the entry
				pConnectivity[ nIndex ] |= nConnected;
			}
		}
	}
}

// *************************************************************************

void CVisFastLabel::ConnectSegments8( const LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
									const LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments,
									Uint8 * restrict pConnectivity )
{
	// This routine will generate the connectivity matrix between the upper and the lower lines' segments.
	//
	// The connectivity matrix is organized in a way that also connections between segments of the same 
	// line may be stored (which is important in MergeUpperSegments()). The matrix is square, with a width
	// of 2*m_nMaxSegments. The connections of the segments of the upper line are stored at indices 
	// 0..(m_nMaxSegments-1), whereas those of the lower line's segments are stored at indices m_nMaxSegments..
	// (2*m_nMaxSegments-1). 
	// 
	// This routine will only examine connection between segments of the upper line to
	// segments of the lower line and thus will only operate on the upper-right quadrant of the matrix (or the
	// lower-left, the matrix is made symmetrical later on).
	//
	// . . . . X X X X
	// . . . . X X X X
	// . . . . X X X X
	// . . . . X X X X
	// . . . . . . . .
	// . . . . . . . .
	// . . . . . . . .
	// . . . . . . . .
	//
	// Connectivity between two adjacent lines' segments is easily determined. There are only two cases in
	// which the two segments are NOT connected:
	//
	// 1)
	//     |a-------b|
	//                 |c----d|      -> c > b
	//
	// 2)
	//                |a-------b|
	//      |c----d|                 -> a > d
	//
	// For 8-connectivity, just add one to b and d before comparing.
	
	//const Int nOffs = m_nMaxSegments * 2;
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;

	// Clear the connectivity matrix.
	MemSet( pConnectivity, 0, m_nMaxSegments * m_nMaxSegments * 4 );

	// Match segments
	for ( Uint32 i = 0; i<unNumUpperSegments; i++)
	{
		for ( Uint32 j = 0; j<unNumLowerSegments; j++ )
		{
			{
				Int nIndex;
				Int nConnected;
	
				// See if those two segments are connected			
				Bool bConnected1 = !( pUpperSegments[i].unBegin > pLowerSegments[j].unEnd + 1 );
				Bool bConnected2 = !( pUpperSegments[i].unEnd + 1 < pLowerSegments[j].unBegin );
				nConnected = ( bConnected1 && bConnected2 ) ? 1 : 0;
				
				// Calculate the index...
				//nIndex = (nOffs * i) + (j + m_nMaxSegments);
				nIndex = (nOffs * i) + (j + unNumUpperSegments );

				// ... and make the entry
				pConnectivity[ nIndex ] |= nConnected;
			}
		}
	}
}
// *************************************************************************

void CVisFastLabel::ResolveEquivalence( Uint8 * restrict pConnectivity, 
										const Uint32 unNumUpperSegments, const Uint32 unNumLowerSegments )
{
	// This function resolves equivalences in the connectivity matrix. As a result, the matrix shows
	// which segments are connected with each other, even if they are connected via several other 
	// segments.
	//
	// First, the matrix is made symmetrical and the diagonal ones are added (all segments are connected
	// to themselves).
	//
	// Example:
	//  upper line: |--0--|   |--1--|     |---2---|  |-3-|
	//  lower line:      |--4--|   |---5---|     |---6---|
	//
	//
	// . . . . 1 . . .     1 . . . 1 . . .
	// . . . . 1 1 . .     . 1 . . 1 1 . .
	// . . . . . 1 1 .     . . 1 . . 1 1 .
	// . . . . . . 1 .     . . . 1 . . 1 .
	// . . . . . . . . ->  1 1 . . 1 . . .
	// . . . . . . . .     . 1 1 . . 1 . .
	// . . . . . . . .     . . 1 1 . . 1 .
	// . . . . . . . .     . . . . . . . 1
	//
	// Then, a modified version of Floyd Warshall is applied to the matrix, yielding the
	// final connectivity (in the case of the example, each segment is connenected with
	// each other, except for segment 7).
	//
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// 1 1 1 1 1 1 1 .
	// . . . . . . . 1
	//
	// This matrix is then used by MergeUpperSegments() and LabelLowerSegments() to create the final label
	// information

	//const Int nOffs = m_nMaxSegments * 2;
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;

	// const Int nWidth = m_nMaxSegments*2;
	const Int nWidth = nOffs;
	
	// First, make the connectivity matrix symmetrical by adding the entries to the lower
	// left quadrant
	for ( Int i=0; i<nWidth; i++ )
	{
		for ( Int j=0; j<nWidth; j++ )
		{
			 pConnectivity[ i * nOffs + j ] |= pConnectivity[ j * nOffs + i ];
		}
	}

	// Then add the diagonal elements
	for ( Int d=0; d<nWidth; d++ )
		pConnectivity[ d*nOffs + d ] = 1;
	
	// Use floyd warshall to resolve the connectivity of the upper segments
	Int j;
	for ( j=0; j<nWidth; j++ )
	{
		for ( Int i=0; i<nWidth; i++ )
		{
			if ( pConnectivity[ j * nOffs + i ] != 0 )
			{
			/*
				for ( Int k=0; k<nWidth; k++ )
					pConnectivity[ i * nOffs + k ]	|= pConnectivity[ j * nOffs + k ];
					*/
				// OR 4 bools at a time.
				for ( Int k=0; k<nWidth; k+=4 )
					*(Uint32*)(&pConnectivity[ i * nOffs + k ])	|= 
							*(Uint32*)(&pConnectivity[ j * nOffs + k ]);
			}
		}
	}
}

// *************************************************************************

void CVisFastLabel::MergeUpperSegments( const Uint8 * restrict pConnectivity, 
										LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
										const Uint32 unNumLowerSegments, 
										FastLabelObject * restrict pLabelObjects )
{
	// We have to examine whether any of the specified segments of the upper line is connected
	// to another segment of that line. We then have to examine whether those two segemnts 
	// have different labels and, if this is true, merge the two labels. A connection between
	// two segments on the upper line is only possible through a segment on the lower line that
	// connects to two of the upper segments.
	//
	// 1 X X X . . . .
	// . 1 X X . . . .
	// . . 1 X . . . .
	// . . . 1 . . . .
	// . . . . 1 . . .
	// . . . . . 1 . .
	// . . . . . . 1 .
	// . . . . . . . 1
	//
	// Since the upper line's segments are stored at the lower positions of the connectivity matrix,
	// we only have to examine the upper-right triangle of the upper-left quadrant. For each one (equals
	// connectivity between the two segments), we examine the labels of those segments and merge if
	// required.

	//const Int nOffs = m_nMaxSegments * 2;
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;

	// Go through the upper-right triangle of the upper-left quadrant. Don't
	// include the diagonal series of 1s.
	for ( Int i=0; i<(signed)unNumUpperSegments-1; i++ )
	{
		for ( Int j=i+1; j<(signed)unNumUpperSegments; j++ )
		{
			// If there is a '1', we'll have to examine the labels.
			if ( pConnectivity[ i * nOffs + j ] != 0 )
			{
				Uint32 oldLabel, newLabel;

				newLabel = pUpperSegments[i].unLabel;
				oldLabel = pUpperSegments[j].unLabel;

				if ( newLabel != oldLabel )
				{
					FastLabelObject * oldObj;
					FastLabelObject * newObj;

					newObj = pLabelObjects + newLabel;
					oldObj = pLabelObjects + oldLabel;

					// Now merge the label data
					newObj->Merge( oldObj, m_nHistogramWidth );

					// And re-label all segments in the current line with the old label
					for ( Uint32 k=0; k<unNumUpperSegments; k++ )
						if ( pUpperSegments[k].unLabel == oldLabel )
							pUpperSegments[k].unLabel = newLabel;
				}
					
			} // if '1'

		} 
	}
}
// *************************************************************************

void CVisFastLabel::LabelLowerSegments( const Uint16 unCurLine,
										const Uint8 * restrict pConnectivity,
										LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
										LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments, 
										FastLabelObject * restrict pLabelObjects, Int & nNumLabelsUsed )
{
	// Lower segments may either be labelled by a label already used in the upper line, by one that is used in the
	// lower line (shouldn't be possible wihtout also being used by the upper line) or by a new label. So, we first
	// have to search the lower-left (or upper-right) quadrant to see if a segment is connected to one of the upper
	// segments. If that's not the case for a segment, we have to create a new label for it.
	//
	// 1 . . . . . . .
	// . 1 . . . . . .
	// . . 1 . . . . .
	// . . . 1 . . . .
	// X X X X 1 . . .
	// X X X X . 1 . .
	// X X X X . . 1 .
	// X X X X . . . 1
	//
	// Note: MergeUpperSegments() must already be applied to the upper segments before calling this function.

	//const Int nOffs = m_nMaxSegments * 2;
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;

	// Go through the lower left quadrant line-wise (i.e. search the connected segments
	// for each of the lower line's segments). We only have to search the lines that are
	// actually used.
	for ( Uint32 i=0; i<unNumLowerSegments; i++ )
	{
		Bool bDone = FALSE;

		// Get the current segment's reference
		LineSegment *	pCurSegment = &(pLowerSegments[i]);
		Uint16 *		pCurHistogram = m_pCurrentSegmentHistograms + i*m_nHistogramWidth;

		// Now, for a given lower segment, search segments on the upper line that are
		// connected to it.
		for ( Uint32 j=0; j<unNumUpperSegments; j++ )
		{
			// If we find a '1' we're happy.
			// Note: the segments of the lower line are stored at the higher indices in the connectivity
			//		 matrix. Thus, we have to add m_nMaxSegments to the index.
			if ( pConnectivity[ (i+unNumUpperSegments) * nOffs + j ] != 0 )
			{
				Uint32 unLabel;
				FastLabelObject * labelObj;				

				// Get the upper segment's label and the label object
				unLabel = pUpperSegments[j].unLabel;
				labelObj = pLabelObjects + unLabel;

				// Now add this segment's info to the object
				AddSegmentToLabel( unCurLine, pCurSegment, pCurHistogram, labelObj );

				// We found a label for this segment and are thus done with it.
				pCurSegment->unLabel = unLabel;
				bDone = TRUE;
				break;
			}
		}

		// If we didn't find a segment of the upper line that is connected to the current segment 
		// (and already labelled), we have to create a new label.
		if ( ! bDone )
		{
			FastLabelObject * newLabelObj = &(pLabelObjects[nNumLabelsUsed]);
			newLabelObj->InitLabel( m_nHistogramWidth );

			pCurSegment->unLabel = nNumLabelsUsed;
			AddSegmentToLabel( unCurLine, pCurSegment, pCurHistogram, newLabelObj );

			// Advance to the next label.
			nNumLabelsUsed++;

			// Emergency:	don't go too far with this index! This should not happen if
			//				nNumLabelsUsed is chosen carefully.
			if ( nNumLabelsUsed >= m_nMaxObjects )
				nNumLabelsUsed--;
		}
	}
}

// *************************************************************************

void CVisFastLabel::AddSegmentToLabel(	const Uint16 unCurLine, 
										const LineSegment * restrict pSegment, const Uint16 * restrict pHistogram,
										FastLabelObject * restrict labelObj )
{
	Int a,b;

	a = pSegment->unBegin-1;		// for all moments to be calculated correctly, we have to decrement the beginning by one.
	b =	pSegment->unEnd;

	// Calculate accumulators
	Int area = b - a;
	Int mx = ( (b*(b+1)) - (a*(a+1)) ) / 2;
	Int my = area * unCurLine;
	
	// Add the segment's area
	labelObj->unArea += area;

	// Add the CG accumulators
	labelObj->unMx += mx;
	labelObj->unMy += my;

	// Add the higher order accumulators, only if requested
	if ( (m_unFlags & LABEL_ACQUIRE_MOMENTS) != 0 )
	{
		Int mxx = ( (b*(b+1)*(2*b+1)) - ( a*(a+1)*(2*a+1) ) ) / 6;
		Int myy = area * unCurLine * unCurLine;
		Int mxy = unCurLine * mx;
		labelObj->unMxx += mxx;
		labelObj->unMyy += myy;
		labelObj->nMxy += mxy;
	}
		
	// Add the histogram, only if requested
	if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) != 0 )
	{
		for ( Int i=0; i<m_nHistogramWidth; i++)
			labelObj->pHistogram[i] += pHistogram[i];
	}

	// ASSERT( labelObj->unArea == (labelObj->pHistogram[1] + labelObj->pHistogram[2] + labelObj->pHistogram[3] ));
	
	// update the bounding box
	labelObj->unBoundingTop		= min( labelObj->unBoundingTop,		unCurLine );
	labelObj->unBoundingBottom	= max (labelObj->unBoundingBottom,	unCurLine );
	labelObj->unBoundingLeft	= min( labelObj->unBoundingLeft,	pSegment->unBegin );
	labelObj->unBoundingRight	= max( labelObj->unBoundingRight ,	pSegment->unEnd );

}

// *************************************************************************

void CVisFastLabel::FinalizeObjects( const Int nNumLabelsUsed, const Uint32 unInputWidth, const Uint32 unInputHeight )
{
	Uint32 count;
	Uint32 i;

	// Acquire buffer
	FastLabelObject *	objects = (FastLabelObject*)m_oportLabels.GetBuffer();

	// Go through objects list with two indices. Move all used objects down as far
	// as possible, so that all can be found at indices following and including 1.

	count = 1; // entry 0 is background.
	i = 1;

	while( (count < (unsigned)m_nMaxObjects-1) && (i < ((unsigned)nNumLabelsUsed)) ) 
	{
		// Only add objects that have a minimal size
	  	if ( (objects[i].bValid) && (objects[i].unArea >= (unsigned)m_nMinObjectArea) )
	   	{ 
	   		// Mask out objects that are too near to the image border.
	   		if (	((Int)objects[i].unBoundingTop			> m_nMinDistanceToBorder) 
					&& ((Int)objects[i].unBoundingBottom 	< (Int)unInputHeight-m_nMinDistanceToBorder-1 )
	   				&& ((Int)objects[i].unBoundingLeft		> m_nMinDistanceToBorder) 
					&& ((Int)objects[i].unBoundingRight		< (Int)unInputWidth-m_nMinDistanceToBorder-1 ) )
	   		{
			
				objects[count].bValid = true;
				objects[count].unArea	= objects[i].unArea;
				
				Int nArea = (signed)objects[i].unArea;				

				// Calculate the higher moments, if this is requested.
				// Normalize the higher order momentums to the center of gravity
				if ( (m_unFlags & LABEL_ACQUIRE_MOMENTS) != 0 )
				{
					Int mxx, myy, mxy;

					// We need floating point here because of simplicity. The mx^2 and my^2 terms tend to range from very
					// small values to rather large ones, so that 32 bit arithmetics won't suffice for larger objects
					// and doing the /nArea division first will mess up small objects.
					mxx = (Uint32)( (float)objects[i].unMxx - ((float)objects[i].unMx * (float)objects[i].unMx) / (float)nArea) / nArea;
					myy = (Uint32)( (float)objects[i].unMyy - ((float)objects[i].unMy * (float)objects[i].unMy) / (float)nArea) / nArea;
					mxy = (Int)( (float)objects[i].nMxy - (float)(objects[i].unMx * (float)objects[i].unMy) / (float)nArea) / nArea;

					/*
					mxx = ( (signed)objects[i].unMxx - (signed)(objects[i].unMx * objects[i].unMx) / nArea) / nArea;
					myy = ( (signed)objects[i].unMyy - (signed)(objects[i].unMy * objects[i].unMy) / nArea) / nArea;
					mxy = ( objects[i].nMxy - (signed)(objects[i].unMx * objects[i].unMy) / nArea) / nArea;
					*/

					/*
					objects[count].unMxx	= (mxx > 0) ? mxx : 0;
					objects[count].unMyy	= (myy > 0) ? myy : 0;
					//objects[count].nMxy		= (mxy > 0) ? mxy : 0;
					objects[count].nMxy		= mxy;
					*/				

					objects[count].unMxx	= mxx;
					objects[count].unMyy	= myy;
					objects[count].nMxy		= mxy;
				}

				// Calculate the center of gravity of the first momentum
				objects[count].unMx		= objects[i].unMx / objects[i].unArea;
				objects[count].unMy		= objects[i].unMy / objects[i].unArea;

				// Copy bounding box.
				objects[count].unBoundingLeft	= objects[i].unBoundingLeft;
				objects[count].unBoundingRight	= objects[i].unBoundingRight;
				objects[count].unBoundingTop	= objects[i].unBoundingTop;
				objects[count].unBoundingBottom = objects[i].unBoundingBottom;

				// Copy histogram, if requested
				if ( (m_unFlags & LABEL_ACQUIRE_HISTOGRAM) != 0 )
				{
					for ( Int k=0; k<m_nHistogramWidth; k++)
						objects[count].pHistogram[k] = objects[i].pHistogram[k];
				}

				// ASSERT( objects[count].unArea == objects[count].pHistogram[1] + objects[count].pHistogram[2] + objects[count].pHistogram[3] );

				// Clear the tracked flag.
				objects[count].bTracked = false;

				// onto the next...		
				count++;
		
			}
		}

	   i++;
	}

	// Store the number of labels in the contour no. 0.
	objects[0].unNumObjects = count;

	// Now, if requested, transform the coordinates.
	if ( (m_unFlags & LABEL_TRANSFORM_COORDS) != 0 )
		TransformCoordinates();
}

// *************************************************************************

void CVisFastLabel::TransformCoordinates( )
{
	// Acquire buffer
	FastLabelObject *	objects = (FastLabelObject*)m_oportLabels.GetBuffer();

	CVisVector vecPix;
	CVisVector vecTrans;

	// Abort if no transform is set.
	if ( m_pTransform == NULL )
		return;

	// Assume z=1. That's certainly not true, but we just don't know better. A value
	// of 1 will favor all transformations (->mult, div!), though.
	vecPix(3) = 1.0;

	// Go through all objects.
	for ( int i=1; i<objects[0].unNumObjects; i++)
	{
		// Assemble input vector in pixel coordinates.
		vecPix(1) = objects[i].unMx;
		vecPix(2) = objects[i].unMy;

		// Transform to world coordinates.
		vecTrans = m_pTransform->TransformToWorld( vecPix );

		// And store in the label object.
		objects[i].pTransformedCoords->fpMx = vecTrans(1);
		objects[i].pTransformedCoords->fpMy = vecTrans(2);
		objects[i].pTransformedCoords->fpMz = vecTrans(3);
		
		/*
		char str[1024];
		vecTrans.FormatMatrix( str );
		LogMsg("Transformed vector: %s", str );
		LogMsg("fp1: %d @ %d, v(1): %d @ %d", objects[i].pTransformedCoords->fpMx.m_nValue, objects[i].pTransformedCoords->fpMx.m_nFractBits, vecTrans(1).m_nValue, vecTrans(1).m_nFractBits );
		LogMsg("Transformed object: %f x %f", objects[i].pTransformedCoords->fpMx.GetFloatValue(), objects[i].pTransformedCoords->fpMy.GetFloatValue() );
		*/
	}
}

// *************************************************************************

void FastLabelObject::InitLabel( Int nHistogramWidth )
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

	// Clear the histogram
	if ( (nHistogramWidth != 0) )
	{
		for ( Int i=0; i<nHistogramWidth; i++)
			pHistogram[i] = 0;
	}

	bValid = true;
}


// *************************************************************************

void FastLabelObject::Merge( FastLabelObject * wasteObj, Int nHistogramWidth )
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

	// Merge the histogram
	if ( (nHistogramWidth != 0) )
	{
		for ( Int i=0; i<nHistogramWidth; i++)
			pHistogram[i] += wasteObj->pHistogram[i];
	}

	// invalidate old label
	wasteObj->bValid = false;
}
					
// *************************************************************************

bool FastLabelObject::CalcAxes( Int & nMajAxis, Int & nMinAxis )
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


#ifdef _WINDOWS

				
// *************************************************************************

void CVisFastLabel::PrintEquivalenceMatrix( Uint8 * pMatrix, Uint32 unNumUpperSegments, Uint32 unNumLowerSegments ) 
{
	const Int nOffs = (unNumUpperSegments + unNumLowerSegments + 3) & ~0x03;
	const Int nWidth = nOffs;

	for ( Int i=0; i< nWidth; i++)
	{
		for ( Int j=0; j<nWidth; j++ )
		{
			TRACE(" %01d", pMatrix[ i*nOffs + j ] );
		}

		TRACE("\n");
	}
}
				
// *************************************************************************

void CVisFastLabel::DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint8 color)
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

void CVisFastLabel::DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint8 color)
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

void CVisFastLabel::DrawPixel( Uint8 * pImage, const Int32 x, const Int32 y, const Uint32 color)
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

#endif

// *************************************************************************














