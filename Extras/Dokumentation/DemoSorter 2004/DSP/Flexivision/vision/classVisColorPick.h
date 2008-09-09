
/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISCOLORPICK_H_
#define _CLASSVISCOLORPICK_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisFastLabel.h"

/**
* The structure that is used to pass an object's color information through the
* output port to the next component. The port will contain an array of this structure,
* whereby the first element of the array contains the number of objects in the array
* (including the first object).
*/
struct ColorObject
{
	union
	{
		struct
		{
			Uint8		unHue;
			Uint8		unLum;
			Uint8		unSat;
			Uint8		unHueDivSat;
		} Color;

		Int				nNumObjects;
	};
};


/**
* @brief 
*
*
*/
class CVisColorPick : public CVisComponent
{
public:
							CVisColorPick( const Char * strName, const Int nMaxNumObjects );

	
	/**  */
	void					DoProcessing();
	

protected:

	void					PickColors( const Uint32 * pInputImg, const FastLabelObject * labels, ColorObject * pColorObjects );
	
	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportLabelData;
	CVisOutputPort			m_oportColorData;

	CVisProperty			m_propWindowSize;
	Int						m_nWindowSize;

	CVisProperty			m_propMinLuminance;
	Int						m_nMinLuminance;

	CVisProperty			m_propMaxLuminance;
	Int						m_nMaxLuminance;

	ColorObject				* m_Colors;

	Int						m_nMaxNumObjects;
};
#endif

