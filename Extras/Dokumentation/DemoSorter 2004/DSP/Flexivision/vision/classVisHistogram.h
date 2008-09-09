/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISHISTOGRAM_H_
#define _CLASSVISHISTOGRAM_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* The struct that is used to carry data through the oportHistogram port.
* The memory store is defined as array of size 1, so that it lies directly
* after the other variables in the memory. Of course, its size is bigger and
* depends on the quant levels.
*/
struct HistogramData
{
	Uint32					unNumPixels;
	Uint32					unQuantLevels;
	Uint32					unQuantShift;
	Uint32					unOffsetShift;
	Uint32					unNumChannels;
	Uint16					unpHistogram[1];

	inline int				GetIndex( const int R, const int G, const int B ) const	{ return (	 R  
																								 +(G << unOffsetShift )
																							     +(B << (2*unOffsetShift)) ); }

	inline Uint16			GetValue( const int R, const int G, const int B ) const	{ return unpHistogram[ GetIndex(R,G,B) ]; }
};

/**
* @brief The histogram component.
*
* The histogram component creates the histogram of the input image and accumulates
* it during its lifetime.
*/
class CVisHistogram : public CVisComponent
{
public:
	/**
	* Constructor.
	*/
						CVisHistogram( const char * strName, CVisPort::PortDataType pdtInputType, const int nNumQuantLevels );

	void				Prepare();

	/**
	* Main processing routine.
	*/
	void				DoProcessing();

	/**
	* Clears the histogram. Note: this function accesses the memory directly.
	*/
	void				ClearCalibrationData();

	/**
	* Gets the calibration data from this component.
	*/
	Uint32				GetCalibrationData( Ptr pBuffer );

	/**
	* Sets the calibration data.
	*/
	Uint32				SetCalibrationData( const Ptr pBuffer );

	/**
	* Gets this component's calibration data size.
	*/
	Uint32				GetCalibrationDataSize();

	/**
	* Gets the LUT data port's size, which depends on the number of quantization levels
	* and the number of channels.
	*/
	Uint32				GetHistogramSize( );


protected:
	void				BuildRGBHistogram(		const Uint32 * restrict pRGBImg, 												
												HistogramData * restrict hist, 
												const Uint32 numPixels );

	void				BuildRGBHistogram_msk(	const Uint32 * restrict pRGBImg, 
												const Uint8 * restrict pGrayscaleMask, 
												HistogramData * restrict hist, 
												const Uint32 numPixels );


	CVisInputPort		m_iportInput;
	CVisInputPort		m_iportMaskInput;
	CVisOutputPort		m_oportHistogram;

	/** This is a read-only (kind of) property to make the number of pixels in the histogram visible to the host. */
	Int32				m_nNumPixelsInHist;
	CVisProperty		m_propNumPixelsInHist;	

	bool				m_bRGB;
	Int					m_nNumQuantShift;
	Int					m_nNumQuantLevels;
};


#endif

