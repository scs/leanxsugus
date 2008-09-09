
#include "classVisKeypointsCut.h"


// *************************************************************************

CVisKeypointsCut::CVisKeypointsCut( const char * strName, Int nMaxNumKeys, Uint32 unFlags )
	:CVisComponent( "KeypointCut", strName )
	,m_iportInput( "input", CVisPort::PDT_8BPP_GRAY )
	,m_oportOutput( "output",	  CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_FAST
								| ((unFlags & OUTPUT_ON_INPUTPORT) ? CVisOutputPort::OUTPORT_HOLLOW : 0) )
	,m_propWindowSize( "WindowSize" )
	,m_propRatioThreshold( "RatioThreshold" )
	,m_propMaxMatch( "MaxMatch" )
{
	m_iportInput.Init( this );
	m_oportOutput.Init( this );

	m_propWindowSize.Init( this, CVisProperty::PT_INTEGER, &m_nWindowSize );
	m_propRatioThreshold.Init( this, CVisProperty::PT_FLOAT, &m_fRatioThreshold );
	m_propMaxMatch.Init( this, CVisProperty::PT_INTEGER, &m_nMaxMatch );

	m_nWindowSize = 7;
	m_nMaxMatch = 1000000;
	m_fRatioThreshold = 0.48f;

	m_unFlags = unFlags;

	m_nMaxNumKeys = nMaxNumKeys;
	CVisBufferManager::Instance()->RequestBuffer( this, m_nMaxNumKeys * sizeof(Keypoint), (Ptr*)&m_pKeypoints, CVisBufferManager::BUF_FAST );

	m_nProfile1 = NewProfileTask( "Find Keypoints" );
	m_nProfile2 = NewProfileTask( "Enter Keypoint" );
	m_nProfile3 = NewProfileTask( "Cut" );;
}
  
// *************************************************************************

CVisKeypointsCut::~CVisKeypointsCut()
{
}
  
// *************************************************************************

bool CVisKeypointsCut::DoProcessing()
{	
	Int i;
	
	// If we use the same buffer for output and for input, we must propagate the pointer
	// each time before processing it. 	
	if ( m_unFlags & OUTPUT_ON_INPUTPORT )
	{
		m_oportOutput.SetBuffer( m_iportInput.GetBuffer() );
		m_oportOutput.PropagateBuffer();
	}

	Uint8 * pInputImg = (Uint8*)m_iportInput.GetBuffer();
	Uint8 * pOutputImg = (Uint8*)m_oportOutput.GetBuffer();
	
	// If we have different buffers for input and output, we need to copy the image first.
	// We can do that in the background. The image needs to be ready when before the objects
	// are actually cut.
	int nCopy = COPY_DONE;
	if ( (m_unFlags & OUTPUT_ON_INPUTPORT) == 0 )
		nCopy = StartCopy( pOutputImg, pInputImg, m_iportInput.GetBufferSize() ); 

	// Find keypoints...
	StartProfileTask( m_nProfile1 );
	FindKeypoints( (Uint8*)pInputImg, m_fRatioThreshold, (m_nWindowSize-1)/2 );
	StopProfileTask( m_nProfile1 );

	// Now we need the output image, so make sure the copy operation has finished
	if ( (m_unFlags & OUTPUT_ON_INPUTPORT) == 0 )
		WaitCopy( nCopy );

	// ... and cut the potatoes.
	StartProfileTask( m_nProfile3 );
    CutObjects( (Uint8*)pOutputImg );
	StopProfileTask( m_nProfile3 );

	return true;
}
  
// *************************************************************************

void CVisKeypointsCut::FindKeypoints(Uint8 * pImgIn, float fRatioThresh, Int windowSize)
// size:     	(WindowSize - 1) / 2
// thres:    	background/foreground distinction threshold
// ratioThres: 	Threshold for keypoint (if less than this portion is background in the Window
//           	it is a keypoint). In pixels of the filter window.
{
	Int i, j, k, l;
	Int bckg;
	Int mx, my;
	Int	ratioThres;
	
	// Calculate the number of pixels for the ratio
	ratioThres= (Int)(fRatioThresh * ((2*windowSize+1)*(2*windowSize+1)));
	
	// Clear keys from previous pass
	ResetKeys();
		
	// For all pixels
	for(i=windowSize; i<(signed)m_unResultHeight-windowSize-1; i++) 
	{
		for(j=windowSize; j<(signed)m_unResultWidth-windowSize-1; j++) 
		{
			// If this is a background pixel, and at least an opposite pair of 
			// neighbour pixels are foreground, we've got a candidate.
			if ( pImgIn[i*m_unResultWidth + j] == 0 ) 
			{
				if((  pImgIn[i*m_unResultWidth + j - 1 ] != 0 || 
					  pImgIn[i*m_unResultWidth + j + 1 ] != 0 ) &&
					( pImgIn[(i-1)*m_unResultWidth + j ] != 0 ||
					  pImgIn[(i+1)*m_unResultWidth + j ] != 0)) 
				{
					// do the filter
					bckg = 0;
					mx = 0;
					my = 0;
					
					// We now examine a window of pixels around the candidate. We need to find the number
					// of background pixels and their center of mass. The search is aborted if too many
					// background pixels are found. We also need to find the amount of background in the
					// vicinity of the candidate, so we can determine the keypoint's strength. The less 
					// background, the "sharper" the corner and thus the lower the strength (strength is
					// reciprocal).
					
					// For each column in the window...
					for( k = i-windowSize ; k <= i+windowSize ; k++) 
					{
					
						// abort if too much background.
						if (bckg >= ratioThres) 
							break;
					
						
						/*
						// For each row in the window
						for( l = j-windowSize ; l <= j+windowSize ; l++)
						{
							// If it's a background pixel, count it
							int isBack = ( pImgIn[k*m_unResultWidth + l] == 0 );
							
							bckg += isBack;
							mx += (isBack==1) ? l : 0;
							my += (isBack==1) ? k : 0;
						} // for each row
						*/
						
						// For each row in the window
						for( l = j-windowSize ; l <= j+windowSize ; l++)
						{
							// If it's a background pixel, count it
							if ( pImgIn[k*m_unResultWidth + l] == 0 )
							{
								bckg ++;
								mx += l;
								my += k;
							}
						} // for each row
						
					} // for each col
					
					// So, now we're done with the filter window. Check if it's a valid keypoint, which
					// applies if the ratio of background to foreground does not exceed the threshold.
					if (bckg < ratioThres) 
					{
						StartProfileTask( m_nProfile2 );
						EnterKey(bckg, j, i, (float)mx/bckg, (float)my/bckg, 2*windowSize );
						StopProfileTask( m_nProfile2 );
					} 

				} // if two neighbrours are foreground

			} // if background pixel

		} // for m_unResultWidth
	} // for m_unResultHeight
}

  
// *************************************************************************

void CVisKeypointsCut::ResetKeys()
{
	m_nNumKeys = 0;
}

// *************************************************************************

int CVisKeypointsCut::EnterKey(Int strength, Int x, Int y, float mx, float my, Int radius)
{
	#define LOWEST_STRENGTH	((Int)(10000))
	
	Int 	mini = LOWEST_STRENGTH;
	Int 	indi = -1;
	Int 	i;
	  
	// Only insert, if there's space in the array.
	if (m_nNumKeys < m_nMaxNumKeys - 1) 
	{
		// Determine the strongest keypoint in the region of the new keypoint. Strength is
		// reciprocal, so lower numbers mean stronger points (i.e. less background pixels
		// mean sharper spike).
	    for (i=0; i<m_nNumKeys; i++) 
		{
			// The keypoint has to have a lower strength value and must be in the vicinity.
			if (m_pKeypoints[i].strength < mini &&
				abs((Int)m_pKeypoints[i].x - (Int)x) < radius && 
				abs((Int)m_pKeypoints[i].y - (Int)y) < radius) 
		  	{ 
	        	mini = m_pKeypoints[i].strength; 
		        indi = i;
		    }
    	}

		// If there is no stronger keypoint in the vicinity...    	
		if (mini > strength) 
		{ 
			// ... either create a new entry or just re-use and overwrite the 
			// strongest keypoint found.
			if (mini == LOWEST_STRENGTH) 
			{
				indi = m_nNumKeys;
				m_nNumKeys++;
			}
			
			// Copy values.
			m_pKeypoints[indi].strength = strength;
			m_pKeypoints[indi].x = x;
			m_pKeypoints[indi].y = y;
			m_pKeypoints[indi].mx = mx;
			m_pKeypoints[indi].my = my;
		}
	}
	return 0;
}

// *************************************************************************

void CVisKeypointsCut::CutObjects( Uint8 * pImgOut )
{
	int i, j, ind1=0, ind2=0;
	float diff, mindist=0.0;
	
	// While we still find pairs of keypoints...
	while (mindist < m_nMaxMatch) 
	{
		// Set mindist insanely high.
		mindist = 1e9;
		
		// For all keypoints found...
		for (i=0; i<m_nNumKeys; i++) 
		{
			// Try to pair it with another keypoint...
			for (j=i+1; j<m_nNumKeys; j++) 
			{
				// Only inspect those points if they weren't used earlier.
				if (m_pKeypoints[i].strength >= 0 && m_pKeypoints[j].strength >= 0) 
				{
					// Find the distance between those two keypoints. The "distance" is not only the
					// physical distance but also a grade of how good those two points match together
					// in directions.
					diff = KeyDistance(&m_pKeypoints[i], &m_pKeypoints[j]);
					//TRACE("Distance between key %d and %d: %f\n", i, j, diff);
					
					// If this is the lowest distance found so far, store the indices.
					if (diff < mindist) 
					{
						//TRACE("current mindist.\n");
						mindist = diff;
						ind1 = i;
						ind2 = j;
					}
				}
			}
		}
		
		// If this is a valid pair...
		if (mindist < m_nMaxMatch) 
		{
			//TRACE("Connected key %d with %d. dist:%f\n", ind1, ind2, mindist);
			// Connect the pair and draw the line
			DrawKeyLine(pImgOut, m_pKeypoints[ind1].x, m_pKeypoints[ind1].y, m_pKeypoints[ind2].x, m_pKeypoints[ind2].y);
			
			// invalidate both keypoints so they won't be used for further pairings.
			m_pKeypoints[ind1].strength = -1;
			m_pKeypoints[ind2].strength = -1;
		}
		
		//TRACE("***Next Try***\n");
	}
}

// *************************************************************************

float CVisKeypointsCut::KeyDistance(const Keypoint *k1, const Keypoint *k2) 
{
	float a[2], b[2], d[2];
	float dnorm, anorm, bnorm;
	float cosa, cosb;
	
	a[0] = k1->x - k1->mx;
	a[1] = k1->y - k1->my;
	b[0] = k2->x - k2->mx;
	b[1] = k2->y - k2->my;
	d[0] = (float)(k2->x - k1->x);
	d[1] = (float)(k2->y - k1->y);
	dnorm = d[0]*d[0] + d[1]*d[1];
	anorm = a[0]*a[0] + a[1]*a[1];
	bnorm = b[0]*b[0] + b[1]*b[1];
	cosb = -(d[0]*b[0] + d[1]*b[1]) / dnorm*bnorm;
	cosa = (d[0]*a[0] + d[1]*a[1]) / dnorm*anorm;
	if (cosa > 0 && cosb > 0)
		return dnorm/cosb/cosa;
	else 
		return 1e20f;
}

// *************************************************************************

void CVisKeypointsCut::DrawKeyLine( Uint8 * pImgOut, Int x1, Int y1, Int x2, Int y2)
{
	float 	diff;
	int 		j;
	float 	dx, dy;
	Uint8 * 	p;
	
	// Calculate distance. This time it's the real distance. Only the greater
	// of the two distances (x, y) is needed.
	diff = (float)(max(abs(x1 - x2), abs(y1 - y2)));
	
	// Calculate increments
	dx = (x2-x1) / (float)diff;
	dy = (y2-y1) / (float)diff;
	
	if(diff == 0) 
	{
		dx =0; 
		dy =0;
	}
	
	// draw line 
	for(j = 0 ; j <= diff; j++) 
	{  
		p = pImgOut + (int)(x1 + j*dx) + m_unResultWidth * (int)(y1 + j*dy);
		
		if (p > pImgOut + m_unResultWidth && 
		p < pImgOut + (m_unResultHeight-1) * m_unResultWidth ) 
		{
			// Clear the current pixel and all of its direct neighbours.
			// This must be done to get a clear, non-interrupted line.
			*p = BACKGROUND;
			*(p-1) = BACKGROUND;
			*(p-m_unResultWidth) = BACKGROUND;
			*(p-m_unResultWidth-1)= BACKGROUND;
		}
	}
}

// *************************************************************************

