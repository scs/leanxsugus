/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISPPUSIM_H_
#define _CLASSVISPPUSIM_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* @brief Simulates the PPU's processing steps.
*
* Simulates the PPU's processing steps, like LUT, erosion and RGB2Gray.
*/
class CVisPPUSim : public CVisComponent
{
public:
						CVisPPUSim( const Char * strName, int nNumQuantLevels );

	void				Prepare();
	void				DoProcessing();

protected:
	/**
	* Converts some pixels from RGB (32 bits) to grayscale.
	*/
	void				RGBtoGray( const Uint32 * restrict pRGBImage, Uint32 * restrict pGrayImage, const Uint32 numPixels );

	/**
	* Looks up pixels in the LUT.
	*/
	void 				LookupPixels( const Uint32 * restrict pRGBImage, Uint8 * restrict pLUTImage, Uint8 * restrict pGrayImage, const Uint8 * restrict unpLUT, const Uint32 numPixels);

	/**
	* Erodes a single line of the LUT image. This is a special version of the erosion; it only erodes background (i.e. =0)
	* pixels and leaves the other pixels unchanged. That means it just grows the background.
	*/
	void				ErodeLine_lut( const Uint8 * restrict grayInput, Uint8 * restrict grayOutput, const Uint32 cols );

	// Input ports
	CVisInputPort		m_iportRGBInput;
	CVisInputPort		m_iportLUT;

	// output ports
	CVisOutputPort		m_oportGrayOutput;
	CVisOutputPort		m_oportLUTOutput;

	// The properties
	Int32				m_nNumErosions;
	CVisProperty		m_propNumErosions;

	
	// Declare intermediate buffers
	Uint32 *			m_pRGBLine;
	Uint8 *				m_pGrayLine;
	Uint8 *				m_pLUTData;
	Uint8 *				m_pErodedLUTData;

	/** The LUT quantisation levels. */
	Int32				m_nNumQuantLevels;
};

#endif


