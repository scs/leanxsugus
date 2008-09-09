
#include "classVisCalibHistogram.h"

#include <math.h>


CVisCalibHistogram::CVisCalibHistogram( const Char * strName, int nNumQuantLevels )
	: 	CVisComponent( strName, "CalibHistogram" ),
		m_iportHistogram("inputHistogram", CVisPort::PDT_DATA ),
		m_oportLUTOutput("lutOutput", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST),
		m_oportHueHistogram("hueHistogram", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST),
		m_oportLumHistogram("lumHistogram", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST),
		m_propBGHueCenter("BGHueCenter"),
		m_propBGHueRange("BGHueRange"),
		m_propMinSaturation("MinSaturation"),
		m_propMaxValue("MaxValue"),
		m_propMinValue("MinValue"),
		m_propGreenHueCenter("GreenHueCenter"),
		m_propGreenHueRange("GreenHueRange"),
		m_propBinSize("BinSize"),
		m_propProbabilityThreshold("ProbabilityThreshold"),
		m_propLuminanceThreshold("LuminanceThreshold"),
		m_propHueProbabilityThreshold("HueProbabilityThreshold"),
		m_propLuminanceRange("LuminanceRange"),
		m_propVeryBadLuminanceThreshold("VeryBadLuminanceThreshold"),
		m_propLUTCreationMethod("LUTCreationMethod")
{
	m_iportHistogram.Init( this );
	m_oportLUTOutput.Init( this );
	m_oportHueHistogram.Init( this );
	m_oportLumHistogram.Init( this );		
	
	// Set buffer size of LUT outport, which is quantlevel^3 / 4, since we are able to store
	// 4 lut values per byte.
	m_oportLUTOutput.SetBufferSize( nNumQuantLevels * nNumQuantLevels * nNumQuantLevels / 4 );	

	// Set buffer sizes of histogram outputs
	m_oportHueHistogram.SetBufferSize( 256 * sizeof( Uint32 ) );
	m_oportLumHistogram.SetBufferSize( 256 * sizeof( Uint32 ) );

	// Init properties
	m_propBGHueCenter.Init( this, CVisProperty::PT_INTEGER, &m_nBGHueCenter );
	m_propBGHueRange.Init( this, CVisProperty::PT_INTEGER, &m_nBGHueRange );
	m_propMinSaturation.Init( this, CVisProperty::PT_INTEGER, &m_nMinSaturation );
	m_propMaxValue.Init( this, CVisProperty::PT_INTEGER, &m_nMaxValue );
	m_propMinValue.Init( this, CVisProperty::PT_INTEGER, &m_nMinValue );
	m_propGreenHueCenter.Init( this, CVisProperty::PT_INTEGER, &m_nGreenHueCenter );
	m_propGreenHueRange.Init( this, CVisProperty::PT_INTEGER, &m_nGreenHueRange );
	m_propLuminanceThreshold.Init( this, CVisProperty::PT_INTEGER, &m_nLuminanceThreshold );	
	m_propHueProbabilityThreshold.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpHueProbabilityThreshold, 24 );	
	m_propLuminanceRange.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpLuminanceRange, 24 );
	m_propVeryBadLuminanceThreshold.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpVeryBadLuminanceThreshold, 24 );
	m_propLUTCreationMethod.Init( this, CVisProperty::PT_INTEGER, &m_nLUTCreationMethod);


	// Samro:
	// hue center/range : 195/55
	// MinSat: 5
	// Min/MaxVal: 35/220

	// SCS:
	// hue center/range : 180/60
	// MinSat: 45
	// Min/MaxVal: 0/220

	m_nBGHueCenter = 170;//180;
	// TODO: BGHueRange must be set to around 70, as soon as the BG lighting is working.
	m_nBGHueRange = 90;//60;
	m_nMinSaturation = 0;//45;
	m_nMaxValue = 255;//220;
	m_nMinValue = 00;//20;
	m_nGreenHueCenter = 65;
	m_nGreenHueRange = 35;
	m_nLuminanceThreshold = 0;
	m_fpHueProbabilityThreshold = F2FP( 0.01, 24 );
	m_fpLuminanceRange = F2FP( 2.0, 24 );
	m_fpVeryBadLuminanceThreshold = F2FP( 0.65, 24 );
	m_nLUTCreationMethod = 1;

	m_nBinSize = 1;
	m_propBinSize.Init( this, CVisProperty::PT_INTEGER, &m_nBinSize );

	m_fpProbabilityThreshold = F2FP( 5e-4, 24 );
	m_propProbabilityThreshold.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpProbabilityThreshold, 24 );

	m_nNumQuantLevels = nNumQuantLevels;
	
	m_unProf1 = NewProfileTask( "CalibHistogram: Prof1");
	m_unProf2 = NewProfileTask( "CalibHistogram: Prof1");
}
					
// *************************************************************************

void CVisCalibHistogram::Prepare()
{
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pRGBLine, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pHSLLine, CVisBufferManager::BUF_FAST );
}
						
// *************************************************************************

void CVisCalibHistogram::DoProcessing()
{
	// If we're in calibration, create the hue and luminance histograms.
	if ( m_eOperationMode == OP_CALIBRATION )
	{
		HistogramData * pHist	= (HistogramData*)m_iportHistogram.GetBuffer();
		Uint32 * pHueHistogram	= (Uint32*)m_oportHueHistogram.GetBuffer();
		Uint32 * pLumHistogram	= (Uint32*)m_oportLumHistogram.GetBuffer();

		CreateHueLumHistogram( pHist, pHueHistogram, pLumHistogram );
	}

	// Check whether any of the properties have changed and re-calculate the
	// LUTs if that's so.
	if ( HavePropertiesChanged() )
	{
		this->CreateClassificationData();
		ResetPropertiesChanged();
	}
		
	// Don't have to do anything else.
}

// *************************************************************************

void CVisCalibHistogram::CreateClassificationData()
{
	Uint8 * unpLUT			= (Uint8*)m_oportLUTOutput.GetBuffer();
	HistogramData * pHist	= (HistogramData*)m_iportHistogram.GetBuffer();

	if ( m_nLUTCreationMethod == 0 )
		CreateClassificationData( pHist, unpLUT );
	else if ( m_nLUTCreationMethod == 1 )
	{
		Uint32 * pHueHistogram = (Uint32*)m_oportHueHistogram.GetBuffer();
		Uint32 * pLumHistogram = (Uint32*)m_oportLumHistogram.GetBuffer();

		CreateClassificationData_HueLum( pHist, pHueHistogram, pLumHistogram, unpLUT );
	}
}

// *************************************************************************

void CVisCalibHistogram::CreateClassificationData( const HistogramData * restrict histogram, Uint8 * restrict unpLUT )
{
	Uint8 H, S, L;
	Uint8 diff;
	Uint32 unCountThreshold;

	// calculate the pixel count per color that defines a color as good
	unCountThreshold = ( (histogram->unNumPixels>>8) * m_fpProbabilityThreshold) >> 16;

	int quantshift = histogram->unQuantShift;
	int quantlevels = histogram->unQuantLevels;

	// Calculate each pixel
	for (Int32 R = 0; R < quantlevels; R++)
	{
		for (Int32 G = 0; G < quantlevels; G++)
		{
			for (Int32 B = 0; B < quantlevels; B++)
			{
				Uint8 res;

				bool bBG = false;
				bool bGreen = false;
				bool bBad = false;

				// Convert pixel to hsl.
				RGBPixeltoHSL( R<<quantshift, G<<quantshift, B<<quantshift,
								H, S, L );

				// calculate the hue difference.
				//diff = (Uint8)( abs( (int)m_nBGHueCenter - (int)H ) % 256 );
				diff = m_nBGHueCenter-H;		
				if ( diff > 128 )
					diff = 256 - diff;
					
				// do threshold based on hue and saturation.
				if ( ( S < m_nMinSaturation ) || ( diff < m_nBGHueRange ) || ( L > m_nMaxValue ) || ( L < m_nMinValue) )
					bBG = true;

				// now see if it's green. This flag only marks the possibility! It only qualifies
				// the color as being green when it is also a bad color
				//diff = (Uint8)( abs( (int)m_nGreenHueCenter - (int)H ) % 256 );
				diff = m_nGreenHueCenter-H;		
				if ( diff > 128 )
					diff = 256 - diff;
					
				if ( diff < m_nGreenHueRange )
					bGreen = true;
				
				// See if it is a good color
				Uint32 unCount;
				unCount = GetHistogramValue( R, G, B, m_nBinSize, histogram );
				if ( unCount < unCountThreshold )				
					bBad = true;

				// DEBUG apply some sort of luminance thresholding
				if ( L < m_nLuminanceThreshold )
					bBad = true;

				// Now calculate the resulting LUT entry
				if ( bBG )
					res = 0;
				else if ( bBad )
				{
					if (bGreen)
						res = 3;
					else
						res = 2;
				}
				else
					res = 1;

				// Now fumble that value into the lut
				EnterLUTValue( R, G, B, res, unpLUT );
			}
		}
	}

}

// *************************************************************************

void CVisCalibHistogram::CreateClassificationData_HueLum( const HistogramData * restrict histogram, 
														  Uint32 * restrict pHueHistogram, Uint32 * restrict pLumHistogram, 
														  Uint8 * restrict unpLUT  )
{
	Uint8	H,S,L;
	Int		R,G,B;

	// Calculate each pixel
	int quantshift = histogram->unQuantShift;
	int quantlevels = histogram->unQuantLevels;

	// Create the Hue and luminance histograms.
	CreateHueLumHistogram( histogram, pHueHistogram, pLumHistogram );

	// Now calculate mean and var
	Uint32 unHueArea, unHueMean, unHueVar, unHueDev;
	Uint32 unLumArea, unLumMean, unLumVar, unLumDev;

	CalculateMeanAndDev( pHueHistogram, 256, unHueArea, unHueMean, unHueVar, unHueDev);
	CalculateMeanAndDev( pLumHistogram, 256, unLumArea, unLumMean, unLumVar, unLumDev);

	// Calculate the hue threshold
	Uint32 unHueThresh = ((unHueArea >> 8) * (m_fpHueProbabilityThreshold >> 12)) >> 4;

	// Calculate the luminance thresholds
	Uint32 unUpperLuminanceThreshold = unLumMean + ((unLumDev*(m_fpLuminanceRange>>8)) >> 16)*2;
	Uint32 unLowerLuminanceThreshold = unLumMean - ((unLumDev*(m_fpLuminanceRange>>8)) >> 16);
	Uint32 unVeryBadLuminanceThreshold = (unLowerLuminanceThreshold * m_fpVeryBadLuminanceThreshold) >> 24;

	/*
	Int l,m,u;
	CalculateThreshold( pLumHistogram, 256, F2FP( 0.2, 24 ), l, m, u );
	unLowerLuminanceThreshold = l;
	unUpperLuminanceThreshold = u;
	unVeryBadLuminanceThreshold = (unLowerLuminanceThreshold * m_fpVeryBadLuminanceThreshold) >> 24;
*/

	// Calculate each pixel
	for (R = 0; R<quantlevels; R++)
	{
		for (G = 0; G<quantlevels; G++)
		{
			for (B = 0; B<quantlevels; B++)
			{
				Uint8 res;
				Uint8 diff;

				bool bBG = false;
				bool bBadHue = false;
				bool bGreen = false;
				bool bBad = false;
				bool bVeryBad = false;

				// Convert pixel to hsl.
				RGBPixeltoHSL( R<<quantshift, G<<quantshift, B<<quantshift,
								H, S, L );

				// calculate the hue difference.
				//diff = (Uint8)( abs( (int)m_nBGHueCenter - (int)H ) % 256 );
				diff = m_nBGHueCenter-H;		
				if ( diff > 128 )
					diff = 256 - diff;
					
				// do threshold based on hue and saturation.
				if ( ( S < m_nMinSaturation ) || ( diff < m_nBGHueRange ) || ( L > m_nMaxValue ) || ( L < m_nMinValue) )
					bBG = true;
				
				// Determine whether this is an allowed potato hue.
				bBadHue = (pHueHistogram[H] < unHueThresh);

				// now see if it's green. This flag only marks the possibility! It only qualifies
				// the color as being green when it also has a bad hue
				//diff = (Uint8)( abs( (int)m_nGreenHueCenter - (int)H ) % 256 );
				diff = m_nGreenHueCenter-H;		
				if ( diff > 128 )
					diff = 256 - diff;
					
				if ( diff < m_nGreenHueRange )
					bGreen = true;
				
				// See if it is a good color
				if ( (L<unLowerLuminanceThreshold) || (L>unUpperLuminanceThreshold) || bBadHue )
					bBad = TRUE;

				// See if the color is very bad.
				if ( L < unVeryBadLuminanceThreshold )
					bVeryBad = TRUE;
			
				// Now calculate the resulting LUT entry
				if ( bBG )
					res = 0;
				else if ( bBad )
				{
					// DEBUG: mark very bad pixels as green.
					if ( (bGreen && bBadHue) || bVeryBad )
						res = 3;
					else
						res = 2;
				}
				else
					res = 1;

				// Now fumble that value into the lut
				EnterLUTValue( R, G, B, res, unpLUT );
			}
		}
	}
		
	
}
						
// *************************************************************************

void CVisCalibHistogram::CreateHueLumHistogram( const HistogramData * restrict histogram, 
												Uint32 * restrict pHueHistogram, Uint32 * restrict pLumHistogram)
												
{
	Uint8	H,S,L;
	Int		R,G,B;

	// Clear histograms
	for ( Int i=0; i<256; i++)
	{
		pHueHistogram[i] = 0;
		pLumHistogram[i] = 0;
	}	

	// Calculate each pixel
	int quantshift = histogram->unQuantShift;
	int quantlevels = histogram->unQuantLevels;

	// For each RGB pixel, calculate its HSV counterpart and sum up the values in
	// 1-dimensional HUE and LUM histograms. We won't need the SAT values, since
	// they don't contain much information anyway.
	for (R = 0; R < quantlevels; R++)
	{
		for (G = 0; G < quantlevels; G++)
		{
			for (B = 0; B < quantlevels; B++)
			{				
				// Convert pixel to hsl.
				RGBPixeltoHSL( R<<quantshift, G<<quantshift, B<<quantshift,
								H, S, L );

				// Get the value from the RGB histogram
				Uint32 unValue = (Uint32)(histogram->GetValue( R, G, B ));

				// ... and sum it up to the H and L histograms.
				if ( pHueHistogram[H] < 0xFFFF0000 )
					pHueHistogram[H] += unValue;

				if ( pLumHistogram[L] < 0xFFFF0000 )
					pLumHistogram[L] += unValue;
			}
		}
	}

	/*
	TRACE("Hue\tLum\n");
	for ( i=0; i<256;i++ )
	{
		TRACE("%d\t%d\n", pHueHistogram[i], pHueHistogram[i] );
	}
	*/
}


// *************************************************************************


void CVisCalibHistogram::BuildHSL( const Uint32 * restrict pRGBImage, Uint32 * restrict pHSLImage )
{
	Uint32 copyRGB;
	Uint32 copyHSL;

	// Prologue; copy the RGB lines from the image
	copyRGB = StartCopy( (Ptr)m_pRGBLine, (Ptr)pRGBImage, m_unResultWidth * 4 );
	copyHSL = COPY_DONE;
	
	// Kernel
	for ( int lines = 0; lines < m_unResultHeight; lines++)
	{
		Uint32 offs = lines * m_unResultWidth;

		// Convert RGB to HSL
		// -> need RGB input
		// -> done with HSL output afterwards
		WaitCopy( copyRGB );
		WaitCopy( copyHSL );
		RGBtoHSL( m_pRGBLine, m_pHSLLine, m_unResultWidth );	
		copyHSL = StartCopy( (Ptr)(pHSLImage + offs), (Ptr)m_pHSLLine, m_unResultWidth * 4 );
		if ( lines < m_unResultHeight-1)
			copyRGB = StartCopy( (Ptr)m_pRGBLine, (Ptr)(pRGBImage + offs + m_unResultWidth), m_unResultWidth * 4 );
	}

	// Epilogue
	WaitCopy( copyHSL );
}


// *************************************************************************

Uint32 CVisCalibHistogram::GetHistogramValue( const Uint32 R, const Uint32 G, const Uint32 B, const Int nBinSize , const HistogramData * restrict pHistogram)
{
	Int r, r_l, r_h;
	Int g, g_l, g_h;
	Int b, b_l, b_h;

	Uint32 unNumPixels;
	int quantlevels = pHistogram->unQuantLevels;

	if ( nBinSize == 1 )
		return (pHistogram->GetValue( R, G, B ));

	r_l = max( 0, (signed)R-(nBinSize/2) );
	r_h = min( quantlevels, R+nBinSize );

	g_l = max( 0, (signed)G-(nBinSize/2) );
	g_h = min( quantlevels, G+nBinSize );

	b_l = max( 0, (signed)B-(nBinSize/2) );
	b_h = min( quantlevels, B+nBinSize );

	unNumPixels = 0;
	for ( r = r_l; r<r_h; r++ )
		for ( g = g_l; g<g_h; g++ )
			for ( b = b_l; b<b_h; b++ )
				unNumPixels += (pHistogram->GetValue(r, g, b));

	return unNumPixels / ((nBinSize/2+1)*(nBinSize/2+1)*(nBinSize/2+1));
}

// *************************************************************************

void CVisCalibHistogram::EnterLUTValue( const Uint32 R, const Uint32 G, const Uint32 B, const Uint8 value, Uint8 * unpLUT )
{

	Uint32	LUTIndex;
	Uint32	ByteIndex;
	Uint8	LUTEntry;

	LUTIndex = ( R + G*m_nNumQuantLevels + B*m_nNumQuantLevels*m_nNumQuantLevels ) >> 2;
	ByteIndex = R & 0x03;

	// Read entry
	LUTEntry = unpLUT[ LUTIndex ];

	// Mask out the corresponding bits
	LUTEntry = LUTEntry & ~(0x03 << (ByteIndex*2));

	// Enter the new bits
	LUTEntry = LUTEntry | (value << (ByteIndex*2));		
	unpLUT[LUTIndex] = LUTEntry;
}
						
						
// *************************************************************************


#define  HSLMAX   252	/* H,L, and S vary over 0-HSLMAX */
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */
                        /* HSLMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HSLMAX must each fit in a Uint8. */
/* 
* Hue is undefined if Saturation is 0 (grey-scale) 
*/
#define UNDEFINED (HSLMAX*2/3)

void CVisCalibHistogram::RGBPixeltoHSL( const Uint8 R, const Uint8 G, const Uint8 B, 
										Uint8 & H, Uint8 & S, Uint8 & L )
{
	Uint32 	cMax, cMin;
	Uint16 	Rdelta, Gdelta, Bdelta;

	cMax = (Uint32)max( max(R,G), B);	/* calculate lightness */
	cMin = (Uint32)min( min(R,G), B);
	L = (Uint8)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	// r=g=b --> achromatic case, i.e. S and H are undefined
	if (cMax==cMin)
	{			
		S = 0;					
		H = UNDEFINED;
	} 
	// Chromatic cases:
	else 		
	{		
		// Determine Saturation			
		if (L <= (HSLMAX/2))	
			S = (Uint8)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (Uint8)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
			
		// Now, determine Hue
		Rdelta = (Uint16)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (Uint16)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (Uint16)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (Uint8)(Bdelta - Gdelta);
			
		else if (G == cMax)
			H = (Uint8)((HSLMAX/3) + Rdelta - Bdelta);
			
		else // B == cMax 
			H = (Uint8)(((2*HSLMAX)/3) + Gdelta - Rdelta);

//		if (H < 0) H += HSLMAX;     //always false
		if (H > HSLMAX) H -= HSLMAX;
	}
}

					
// *************************************************************************

void CVisCalibHistogram::RGBtoHSL( const Uint32 * restrict pRGBImage, Uint32 * restrict pHSIImage, const Uint32 numPixels )
{
	Uint8	R, G, B;
	Uint8 	H, L, S;
	Uint32 	cMax, cMin;
	Uint16 	Rdelta, Gdelta, Bdelta;

	for( unsigned int i=0; i<numPixels ; i++)
	{

		R = (Uint8)(pRGBImage[i] & 0x0000FF);
		G = (Uint8)((pRGBImage[i] & 0x00FF00) >> 8);
		B = (Uint8)((pRGBImage[i] & 0xFF0000) >> 16);

		cMax = (Uint32)max( max(R,G), B);	/* calculate lightness */
		cMin = (Uint32)min( min(R,G), B);
		L = (Uint8)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

		// r=g=b --> achromatic case, i.e. S and H are undefined
		if (cMax==cMin)
		{			
			S = 0;					
			H = UNDEFINED;
		} 
		// Chromatic cases:
		else 		
		{		
			// Determine Saturation			
			if (L <= (HSLMAX/2))	
				S = (Uint8)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
			else
				S = (Uint8)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
				
			// Now, determine Hue
			Rdelta = (Uint16)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
			Gdelta = (Uint16)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
			Bdelta = (Uint16)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

			if (R == cMax)
				H = (Uint8)(Bdelta - Gdelta);
				
			else if (G == cMax)
				H = (Uint8)((HSLMAX/3) + Rdelta - Bdelta);
				
			else // B == cMax 
				H = (Uint8)(((2*HSLMAX)/3) + Gdelta - Rdelta);

	//		if (H < 0) H += HSLMAX;     //always false
			if (H > HSLMAX) H -= HSLMAX;
		}
		
		pHSIImage[i] = (((Uint32)H) << 16) | (((Uint32)S) << 8) | (((Uint32)L));
	}
}
						
// *************************************************************************

void CVisCalibHistogram::CalculateMeanAndDev( Uint32 * unpVector, Int nNumValues, Uint32 & unArea, Uint32 & unMean, Uint32 & unVariance, Uint32 & unDeviation )
{					
	float	fMean;
	float	fVar;
	float	fAreaAcc = 0.0;
	float	fMeanAcc = 0.0;
	float	fVarAcc = 0.0;

	for ( Int i=0; i< nNumValues; i++)
	{
		fAreaAcc += (float)(unpVector[i]);
		fMeanAcc += (float)(unpVector[i]) * i;
		fVarAcc += (float)(unpVector[i]) * i * i;
	}

	fMean = fMeanAcc / fAreaAcc;
	fVar = fVarAcc / fAreaAcc - fMean*fMean;

	unArea = (Int) fAreaAcc;
	unMean = (Int) fMean;
	unVariance = (Int) fVar;
	unDeviation = (Int)(sqrt(fVar));	
}

// *************************************************************************
	
void CVisCalibHistogram::CalculateThreshold( Uint32 * unpVector, Int nNumValues, Int fpRatio, Int & nLowerThreshold, Int & nMaxValuePos, Int & nUpperThreshold )	
{
	Uint32	unCurMax = 0;
	Uint32	nThresh;

	nMaxValuePos = 0;

	// Find the maximum value in the vector
	for ( Int i=0; i< nNumValues; i++)
	{
		if ( unpVector[i] > unCurMax )
		{
			unCurMax = unpVector[i];
			nMaxValuePos = i;
		}
	}

	// Abort if there is none (i.e. the vector is empty)
	if ( nMaxValuePos	== 0 )
	{	
		nLowerThreshold = 0;
		nMaxValuePos = nNumValues/2;
		nUpperThreshold = nNumValues;
	}

	// Find the threshold value
	nThresh = (unCurMax * (fpRatio>>16)) >> 8;

	// Find the lower threshold
	nLowerThreshold = nMaxValuePos-1;
	while ( ((unpVector[ nLowerThreshold ] > nThresh) || (unpVector[ nLowerThreshold+1 ] > nThresh))
				&& (nLowerThreshold>0) )
		nLowerThreshold--;

	// find the upper threshold
	nUpperThreshold = nMaxValuePos+1;
	while ( ((unpVector[ nUpperThreshold ] > nThresh) || (unpVector[ nUpperThreshold-1 ] > nThresh))
				&& (nUpperThreshold<nNumValues-1) )
		nUpperThreshold++;
}

// *************************************************************************
	

