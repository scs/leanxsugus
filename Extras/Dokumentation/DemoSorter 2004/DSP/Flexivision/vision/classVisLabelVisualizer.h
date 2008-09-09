/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISLABELVISUALIZER_H_
#define _CLASSVISLABELVISUALIZER_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisFastLabel.h"

/**
* @brief A label visualizing component.
*
* A label visualizing component, which visualizes the fastlabel component's data output by
* drawing bounding boxes on the image. The image input must be in RGB and is downscaled a
* certain amount.
*/
class CVisLabelVisualizer : public CVisComponent
{
public:
							CVisLabelVisualizer( const Char * strName, Int width, Int height, Int scale );

	
	/** Processing paints all the color information to screen as well as the label boxes. */
	void					DoProcessing();
	

protected:
	/**
	* Draws the labelinginformation onto the output image.
	*/
	void					DrawLabels( const FastLabelObject * objects, Uint32 * outputImg );

protected:

	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportLabelData;
	CVisOutputPort			m_oportResult;
	
	Int						m_nScale;
};
#endif

