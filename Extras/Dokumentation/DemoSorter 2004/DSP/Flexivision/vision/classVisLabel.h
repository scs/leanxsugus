/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISLABEL_H_
#define _CLASSVISLABEL_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* This is the label structure that is used to pass object data through the output
* port of the ObjectFinder object.
*/
struct LabelObject
{
	union
	{
		/** The area of the object in pixels. */
		Uint32			unArea;

		/** The number of objects found. This is stored here, in the label for object number 0.*/
		Uint32			unNumObjects;
	};

	/** The first moment of the object (used to get the center of masses in x direction). */
	Uint32			unMx;

	/** The first moment of the object (used to get the center of masses in y direction). */
	Uint32			unMy;

	Uint32			unMxx;
	Uint32			unMyy;
	Int32			nMxy;

	/** The bounding box points. Note that besides the bounding box's extent, the points
	 *  at which the object touches the bounding box are also stored.*/		 
	Uint32			unBoundingTop;
	Uint32			unBoundingBottom;
	Uint32			unBoundingLeft;
	Uint32			unBoundingRight;
	
	bool			bValid;

	void			InitLabel( );
	void			Merge( LabelObject * wasteObj );
	bool			CalcAxes( Int & nMajAxis, Int & nMinAxis );

};

/**
* @brief Old labeling component.
*/
class CVisLabel : public CVisComponent
{
public:
	enum LabelConsts
	{
		MAX_OBJECTS_PER_LINE = 128
	};

	/**
	* Constructor. 
	*/
						CVisLabel( const Char * strName, const Int nMaxObjects, const Bool bGenerateImage );
						~CVisLabel();
						
	void				Prepare();

	/**
	* The main processing function.
	*/
	void				DoProcessing( );


protected:

	/**
	* Labels a single line.
	*/
	void				LabelLine4(	const Uint16 * restrict		pLastLabelLine, 
									const Uint8 * restrict		bCurGrayLine, 
									const Uint16 				unCurLineY, 
									Uint16 * restrict			pCurLabelLine,	
									LabelObject * restrict		pLabelObjects, 
									Uint32 &					unNextLabel, 
									Uint16 * restrict			pMergedLabels,
									const Uint32				unCols );
									
	void				LabelLine8(	const Uint16 * restrict		pLastLabelLine, 
									const Uint8 * restrict		bCurGrayLine, 
									const Uint16 				unCurLineY, 
									Uint16 * restrict			pCurLabelLine,	
									LabelObject * restrict		pLabelObjects, 
									Uint32 &					unNextLabel, 
									Uint16 * restrict			pMergedLabels,
									const Uint32				unCols );									

	void				InitLabel(	LabelObject *				pLabelObject );

	void				MergeLabels( const Uint16				oldLabel, 
									const Uint16				newLabel, 
									LabelObject * restrict		pLabelObjects, 
									const Uint32				numLabels, 
									Uint16 * restrict			pMergedLabels );

	void				CalcAxes( const LabelObject * obj, Int nMajorAxis, Int nMinorAxis );

	/**
	* Orders the objects in the list so that they are to be found at index 1 incrementing.
	* Also, finalize the center calculation.
	*/
	void				FinalizeObjects();

	/**
	* Clears the objects list.
	*/
	void				ClearObjects();

	// Define input ports
	CVisInputPort		m_iportInput;

	// Define output ports
	CVisOutputPort		m_oportLabels;
	CVisOutputPort		m_oportOutput;

	/** The last line that was labeled, containing labels for each pixel. */
	Uint16 *			m_pLabelLastLine;

	/** The current line being labeled. Stores the resulting labels. */
	Uint16 *			m_pLabelCurrentLine;

	/** The current grayscale or LUT image line that is being processed. */
	Uint8 *				m_pGrayCurrentLine;

	/** The next grayscale or LUT image line that is being loaded in parallel to
	 *  m_pGrayCurrentLine being processed. */
	Uint8 *				m_pGrayNextLine;

	Uint16 *			m_pMergedLabels;

	/** The property that defines the minimum size of an object in pixels. */
	CVisProperty		m_propMinObjectArea;
	Int32				m_nMinObjectArea;
	
	CVisProperty		m_propNeighborhood;
	Int32				m_nNeighborhood;
	
	/** A property that stores the maximum number of contours used. */
	CVisProperty		m_propMaxNumLabelsUsed;
	Int32				m_nMaxNumLabelsUsed;	

	/** Flag that enables the creation of a labeled output image.*/
	Bool				m_bGenerateOutputImage;

	Int					m_nMaxObjects;


	void DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint8 color);
	void DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint8 color);
	void DrawPixel( Uint8 * pImage, const Int32 x, const Int32 y, const Uint32 color);
};

#endif

