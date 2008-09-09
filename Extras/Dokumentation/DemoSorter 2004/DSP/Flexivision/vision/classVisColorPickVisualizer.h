/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISCOLORPICKVISUALIZER_H_
#define _CLASSVISCOLORPICKVISUALIZER_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisColorPick.h"

/**
* @brief 
*
*
*/
class CVisColorPickVisualizer : public CVisComponent
{
public:
							CVisColorPickVisualizer( const Char * strName, Int scale, Uint32 unFlags );

	
	/** Processing paints all the color information to screen as well as the label boxes. */
	void					DoProcessing();

	enum Flags
	{
		OUTPUT_ON_INPUTPORT = 1
	};
	

protected:
	/**
	* Draws the labelinginformation onto the output image.
	*/
	void					DrawObjects( const FastLabelObject * labelObjects, const ColorObject * colorObjects, Uint32 * outputImg );

protected:

	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportLabelData;
	CVisInputPort			m_iportColorData;
	CVisOutputPort			m_oportOutput;
	
	Int						m_nScale;

	Uint32					m_unFlags;
};
#endif

