/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISBORDERPAINT_H_
#define _CLASSVISBORDERPAINT_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisChainCode.h"

/**
* @brief Paints the chaincode to a target image.
*
* Paints the chaincode to a target image, in order to clear or enhance an object's border.
*/
class CVisBorderPaint : public CVisComponent
{
public:
							CVisBorderPaint( const Char * strName );


	void					DoProcessing();


protected:
	void					Paint1( const BorderData * restrict pBorder, Uint8 * restrict pImage, const Uint8 unColor );
	void					Paint3( const BorderData * restrict pBorder, Uint8 * restrict pImage, const Uint8 unColor );
	void					Paint( const BorderData * restrict pBorder, Uint8 * restrict pImage, Int nWidth, const Uint8 unColor );

	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportBorderData;
	CVisOutputPort			m_oportImage;

	Int32					m_nBrushSize;
	CVisProperty			m_propBrushSize;

	Int32					m_nColor;
	CVisProperty			m_propColor;

};


#endif
