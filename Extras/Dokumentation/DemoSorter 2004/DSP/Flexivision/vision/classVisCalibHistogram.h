/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISCALIBHISTOGRAM_H_
#define _CLASSVISCALIBHISTOGRAM_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisHistogram.h"

/**
* @brief Converts the calibration data to the LUT data required by the classification process.
*
* Converts the calibration data to the LUT data required by the classification process.
*/
class CVisCalibHistogram : public CVisComponent
{
public:

	/**
	* Constructor.
	*/
						CVisCalibHistogram( const Char * strName, int nNumQuantLevels );
					
	/**
	* Prepare the component for processing. This function allocates all additionally needed buffers
	* and must be called by the vision object AFTER the ports have been prepared.
	*/
	void				Prepare();
	
	/**
	* The main processing function
	*/
	void				DoProcessing();	
	

	/**
	* Creates the LUT with the current histogram. The current properties for the background separation
	* are aplied. This function may be called from exterior (i.e. not within the vision network).
	*/
	void				CreateClassificationData();
	
	
	
protected:
	
	/**
	* Creates the LUT data the old way, i.e. using a probability threshold on each RGB color value.
	*/
	void				CreateClassificationData( const HistogramData * restrict histogram, Uint8 * restrict unpLUT  );

	/**
	* Creates the classification data (i.e. the LUT). Uses Hue and Luminance histograms to define thresholds, completely
	* discarding the saturation.
	*
	*/
	void				CreateClassificationData_HueLum(	const HistogramData * restrict histogram, 
															Uint32 * restrict pHueHistogram, Uint32 * restrict pLumHistogram, 
															Uint8 * restrict unpLUT  );

	void				CreateHueLumHistogram(	const HistogramData * restrict histogram, 
												Uint32 * restrict pHueHistogram, Uint32 * restrict pLumHistogram);


	/**
	* Builds a HSL image of the RGB image.
	*/
	void				BuildHSL(		const Uint32 * restrict pRGBImage, 
										Uint32 * restrict pHSLImage );

	Uint32				GetHistogramValue( const Uint32 R, const Uint32 G, const Uint32 B, const Int nBinSize , const HistogramData * restrict  pHistogram);

	void				EnterLUTValue( const Uint32 R, const Uint32 G, const Uint32 B, const Uint8 value, Uint8 * unpLUT );

	/**
	* Converts a single color value from RGB to HSL.
	*/
	void				RGBPixeltoHSL( const Uint8 R, const Uint8 G, const Uint8 B, 
										Uint8 & H, Uint8 & S, Uint8 & L );

	/**
	* Converts a buffer of pixels from RGB to HSL color format.
	*/
	void				RGBtoHSL(		const Uint32 * pRGBImage, Uint32 * pHSIImage, const Uint32 numPixels );

	void				CalculateMeanAndDev( Uint32 * unpVector, Int nNumValues, Uint32 & unArea, Uint32 & unMean, Uint32 & unVariance, Uint32 & unDeviation );
	
	void				CalculateThreshold( Uint32 * unpVector, Int nNumValues, Int fpRatio, Int & nLowerThreshold, Int & nMaxValuePos, Int & nUpperThreshold );
	
	// Define input ports
	CVisInputPort		m_iportHistogram;

	// Define output ports
	CVisOutputPort		m_oportLUTOutput;
	CVisOutputPort		m_oportHueHistogram;
	CVisOutputPort		m_oportLumHistogram;

	// Define properties
	Int32				m_nBGHueCenter;
	CVisProperty		m_propBGHueCenter;

	Int32				m_nBGHueRange;
	CVisProperty		m_propBGHueRange;

	Int32				m_nMinSaturation;
	CVisProperty		m_propMinSaturation;

	Int32				m_nMaxValue;
	CVisProperty		m_propMaxValue;

	Int32				m_nMinValue;
	CVisProperty		m_propMinValue;

	Int32				m_nGreenHueCenter;
	CVisProperty		m_propGreenHueCenter;

	Int32				m_nGreenHueRange;
	CVisProperty		m_propGreenHueRange;
	
	Int32				m_nLuminanceThreshold;
	CVisProperty		m_propLuminanceThreshold;

	Int32				m_fpHueProbabilityThreshold;
	CVisProperty		m_propHueProbabilityThreshold;

	Int32				m_fpLuminanceRange;
	CVisProperty		m_propLuminanceRange;

	Int32				m_fpVeryBadLuminanceThreshold;
	CVisProperty		m_propVeryBadLuminanceThreshold;

	Int32				m_nLUTCreationMethod;
	CVisProperty		m_propLUTCreationMethod;
	
	Int32				m_nBinSize;
	CVisProperty		m_propBinSize;


	Int32				m_fpProbabilityThreshold;
	CVisProperty		m_propProbabilityThreshold;

	// Declare intermediate buffers
	Uint32 *			m_pRGBLine;
	Uint32 *			m_pHSLLine;

	Int					m_nNumQuantLevels;
	
	// Profiler ids
	Uint32				m_unProf1;
	Uint32				m_unProf2;
};


#endif
