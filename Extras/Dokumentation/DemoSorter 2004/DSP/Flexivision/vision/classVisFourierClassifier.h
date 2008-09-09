/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISFOURIERCLASSIFIER_H_
#define _CLASSVISFOURIERCLASSIFIER_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"
#include "classVisFourierDescriptor.h"

/**
* TODO:
* Switching between calibration and classification mode should be done through a virtual function
* so that the framework doesn't have to handle it.
*
* TODO:
* The *CalibrationData functions need to be virtual functions of the CVisComponent, too.
*/

/**
* @brief The fourier classifier class.
*
* The fourier classifier class.
*/
class CVisFourierClassifier : public CVisComponent
{
public:
	enum FourierClassifierConsts
	{
		NUM_COEFFS = CVisFourierDescriptor::NUM_BORDERPOINTS,
		VAR_FRACT_BITS = 31
	};
						CVisFourierClassifier( const Char * strName );
	void				DoProcessing();

	/**
	* Clears all calibration data.
	*/
	void				ClearCalibrationData();

	Uint32				GetCalibrationData( Ptr pBuffer );
	Uint32				SetCalibrationData( const Ptr pBuffer );
	Uint32				GetCalibrationDataSize();

	/**
	* Creates the data needed for the classification out of the calibration data.
	* This function has to be called after a calibration phase before starting
	* classification.
	*/
	void				CreateClassificationData();

protected:
	/**
	* Calculates the difference of a given set of fourier coefficients to the calibrated
	* set of coefficient (mean and variance data).
	*
	* The return value is of fixed point type.
	*/
	Int32				CalculateDifference(	const Uint16 * restrict pCoefficients, 
												const Uint16 * restrict pMean, 
												const Uint32 * restrict pVar );

	/**
	* Adds a set of fourier coefficient to the accumulation buffers. This is done during
	* calibration. To be able to use the calibration set, CalculateMeanAndVar() has to be
	* called to extract the mean and variance data out of the accumulators.
	*/
	void				AddToAccumulators(		const Uint16 * restrict pCoefficients, 
												Uint32 * restrict pMeanAcc, 
												float * restrict pVarAcc );

	/**
	* Calculates the mean and variance data sets from the accumulators. Usually,
	* this function is called when switching from calibration to classificitaion
	* mode.
	*/
	void				CalculateMeanAndVar(	const Uint32 unNumSamples, 
												const Uint32 * restrict pMeanAcc, 
												const float * restrict pVarAcc, 
												Uint16 * restrict pMean, 
												Uint32 * restrict pVar );
	
	/** 
	* This is the fourier descriptor mean value accumulator buffer. Each of the (typically 64)
	* values for each view of each potato are summed up here during calibration and can then
	* be used to calculate their mean values.
	*/
	Uint32 * 			m_pFourierDescrMeanAcc;

	/**
	* This is the fourier descriptor variance accumulator buffer. The squares of each of the
	* fourier coefficients are summed up here during calibration. When finishing the calibration,
	* by the mean accumulators are subtracted from the the variance accumulators, resulting 
	* in a variance value for each of the fft coefficients.
	*/
	float * 			m_pFourierDescrVarAcc;

	Uint16 *			m_pFourierDescrMean;
	Uint32 *			m_pFourierDescrVar;

	/**
	* The number of samples already stored in the accumulators.
	*/
	Uint32				m_unNumSamples;

	CVisInputPort		m_iportFourierData;
	CVisInputPort		m_iportObjectsList;

	/** A Debug port: */
#ifdef _DEBUG
	CVisOutputPort		m_oportDifference;
#endif

	Uint32				m_unNumRelevantPairs;

};

#endif



