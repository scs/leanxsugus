/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISOBJECTSVISUALIZEREMB_H_
#define _CLASSVISOBJECTSVISUALIZEREMB_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisFastLabel.h"
#include "classVisTracker.h"

#include "classVisVector.h"
#include "classVisCamPlaneTransform.h"

/**
* @brief A quick-and-dirty and absolutely un-optimized visualization component.
*/
class CVisObjectsVisualizerEmb : public CVisComponent
{
public:
							CVisObjectsVisualizerEmb( const Char * strName, Int width, Int height, Int scale );

	void					Prepare();
	
	void					SetTransform( CVisTransform * transform );

	
	/** Processing paints all the color information to screen as well as the label boxes. */
	void					DoProcessing();
	
	/** Postprocessing finally numbers all objects so that the ciphers are well readable. */
	void 					DoPostProcessing();

	void					VisualizeCurrentColorData( Uint32 unPotatoId );
	void					VisualizeCurrentFormData( Uint32 unPotatoId );
	void					VisualizeCurrentSplitData( Uint32 unPotatoId );


protected:
	void					VisualizeLUTData( Uint8 * pLutImg, Uint32 * outputImg );

	/**
	* Visualizes the perspective by drawing the lanes' borders and, if bColorizeLanes is set, their content
	* as well. The latter option take a considerable amount of CPU power, though.
	*
	* VisualizePerspective(false)	~ 1 ms
	* VisualizePerspective(true)	~ 12 ms
	*/
	void					VisualizePerspective( bool bColorizeLanes );

	/**
	* Draws a ruler that moves with the measured speed of the conveyor. The ruler is placed at the
	* left edge of the image.
	*/
	void 					VisualizeConveyorData( Uint32 * outputImg );
	
	/**
	* Draws the labelinginformation onto the output image.
	*/
	void					DrawLabels( const FastLabelObject * objects, Uint32 * outputImg );

	/**
	* Draws the potato object information onto the output image.
	*/
	void					DrawPotatoes( const PotatoObject * object, Uint32 * outputImg );

	/**
	* Draws an integer number to the output image.
	*/
	Uint32					DrawInteger( const Uint32 unNumber, const Int32 x, const Int32 y, const Uint32 color);

	/**
	* Draws a fixed point number to the output image.
	*/
	Uint32					DrawFixedpoint( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color);

	/**
	* Draws a fixed point percentage number.
	*/
	Uint32					DrawFixedpointPercent( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color);


	/**
	* Draws a digit to the given grayscale image.
	*/
	Uint32					DrawDigit( Uint32 * pImage, const Char cDigit, const Int32 x, const Int32 y, const Uint32 color);

	/** Draws a horizontal line in the output image. */
	void					DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint32 color);

	/** Draws a vertical line in the output image. */
	void					DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint32 color);

	/** Draws a free-oriented line in the image. */
	void					DrawLine( const Int x_0, const Int y_0, const Int x_1, const Int y_1, const Uint32 color);


	void					DrawPixel( Uint32 * pImage, const Int32 x, const Int32 y, const Uint32 color);

private:
	/** Helper function for DrawLine() */
	void					DrawLineOctant0(	Uint32 * pImage, const Int nPitch, const Int nWidth, const Int nHeight,
												Int x0, Int y0, Int deltaX, const Int deltaY, const Int Xdirection, const Uint32 color);

	/** Helper function for DrawLine() */
	void					DrawLineOctant1(	Uint32 * pImage, const Int nPitch, const Int nWidth, const Int nHeight,
												Int x0, Int y0, const Int deltaX, Int deltaY, const Int Xdirection, const Uint32 color);

protected:

	CVisTransform *			m_pTransform;

	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportLUTImage;
	CVisInputPort			m_iportLabelData;
	CVisInputPort			m_iportObjectData;
	CVisInputPort			m_iportBorderData;
	CVisInputPort			m_iportSplitsData;
	
	CVisOutputPort			m_oportResult;
	
	Uint8 *					m_pLutDataLine;
	
	Int						m_fpBadColorIndicatorThresh;
	CVisProperty			m_propBadColorIndicatorThresh;
	
	Int						m_fpGreenColorIndicatorThresh;
	CVisProperty			m_propGreenColorIndicatorThresh;
	
	Int						m_fpBadShapeIndicatorThresh;
	CVisProperty			m_propBadShapeIndicatorThresh;
			
	Int						m_nScale;
};
#endif

