/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISRGBTOGRAY_H_
#define _CLASSVISRGBTOGRAY_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* @brief Converts RGB images to grayscale.
*
* Converts RGB images to grayscale.
*/
class CVisRGBToGray : public CVisComponent
{
public:
						CVisRGBToGray( const Char * strName );

	void				Prepare();
	void				DoProcessing();

protected:
	/**
	* Converts some pixels from RGB (32 bits) to grayscale.
	*/
	void				RGBtoGray( const Uint32 * restrict pRGBImage, Uint32 * restrict pGrayImage, const Uint32 numPixels );

	// Input ports
	CVisInputPort		m_iportRGBInput;

	// output ports
	CVisOutputPort		m_oportGrayOutput;
	
	// Declare intermediate buffers
	Uint32 *			m_pRGBLine_0;
	Uint32 *			m_pRGBLine_1;
	Uint32 *			m_pRGBLine_2;
	
	Uint8 *				m_pGrayLine_0;
	Uint8 *				m_pGrayLine_1;
	Uint8 *				m_pGrayLine_2;
};

#endif


