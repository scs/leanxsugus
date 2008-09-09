/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISFASTLABEL_H_
#define _CLASSVISFASTLABEL_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"
#include "classVisFixpoint.h"

#include "classVisTransform.h"

/**
* A single foreground segment of a line.
*/
typedef struct {
	Uint16		unBegin;
	Uint16		unEnd;
	Uint32		unLabel;
} LineSegment;

/**
* A structure that stores the transformed coordinates of each label object, if requested.
*/
typedef struct {
	CVisFixpoint	fpMx;
	CVisFixpoint	fpMy;
	CVisFixpoint	fpMz;
	
	/** The init function that must be used when the structure is not explicitly created
	*   using new(), since, in that case, the constructor will not be called. */
	void			Init() 	{ fpMx.Init(); fpMy.Init(); fpMz.Init(); }
} TransformedCoords;


/**
* This is the label structure that is used to pass object data through the output
* port of the ObjectFinder object.
*/
struct FastLabelObject
{
	union
	{
		/** The area of the object in pixels. */
		Uint32				unArea;

		/** The number of objects found. This is stored here, in the label for object number 0.*/
		Uint32				unNumObjects;
	};

	/** The first moment of the object (used to get the center of masses in x direction). */
	Uint32					unMx;

	/** The first moment of the object (used to get the center of masses in y direction). */
	Uint32					unMy;

	Uint32					unMxx;
	Uint32					unMyy;
	Int32					nMxy;

	/** The bounding box coordinates. */
	Uint32					unBoundingTop;
	Uint32					unBoundingBottom;
	Uint32					unBoundingLeft;
	Uint32					unBoundingRight;

	/** A reference to this object's histogram. */
	Uint16 *				pHistogram;

	/** A reference to this object's transformed coordinates. */
	TransformedCoords *		pTransformedCoords;
	
	/** Users-specific flags that are not used by the fastlabel component. */
	Uint32					unFlags;
	
	bool					bValid;
	bool					bTracked;

	void					InitLabel( Int nHistogramWidth );
	void					Merge( FastLabelObject * wasteObj, Int nHistogramWidth );
	bool					CalcAxes( Int & nMajAxis, Int & nMinAxis );

};

/**
* @brief An RLE-labelling class.
*
* The FastLabel class, unlike normal label components, uses an RLE method to efficiently label an image.
* This method vastly improves performance on DSP systems, since it reduces the amount of conditional statements
* considerably. Furthermore, segments creation (i.e. run-length-coding) is a task well suited to DSP architecture.
* 
* When the normal labelling algorithm's speed mostly depended on the number of foreground pixels in an image,
* the fastlabel's speed solely depends on the number of segments on each line, the RL encoding itself is a constant-time
* operation.
*/
class CVisFastLabel : public CVisComponent
{
public:
	/**
	* Constructor.
	*/
						CVisFastLabel( const Char * strName, Int nMaxSegments, Int nHistogramWidth, Int nMaxObjects, Uint32 unFlags );

	void				SetTransform( CVisTransform * transform );

	/**
	*  Prepares the component for processing. Must be called before the first time DoProcessing is called.
	*/
	void				Prepare();

	/**
	* Processes the input image and generates the label objects.
	*/
	void				DoProcessing();

	enum LabelFlags
	{
		/** If this flag is specified, the component uses a neighborhood of 8 connectivity scheme. Otherwise,
		 *  neighborhood of 4 is used. */
		LABEL_NEIGHBORHOOD_8		= 0x0001,

		/** If this flag is specified, the moments of the label objects	are also acquired. */
		LABEL_ACQUIRE_MOMENTS		= 0x0002,

		/** Specifying this flag will yield a histogram of all label objects found in the image. */
		LABEL_ACQUIRE_HISTOGRAM		= 0x0004,

		/** 
		* This flag enables the transformation of the coordinates of the resulting label objects.
		* Only objects that satisfy the minimum area criteria and do'nt touch the image's borders
		* (according to property MinDistanceToBorder) are transformed.
		*/
		LABEL_TRANSFORM_COORDS		= 0x0008
	};


protected:
	/**
	* Finds segments in a grayscale image's line. Segments are defined by their start and end point and are always
	* separated by pixels of value 0.
	*/
	Int					FindSegments( const Uint8 * restrict pGrayLine, LineSegment * restrict pSegments, const Int nMaxSegments, const Int nCols );

	/**
	* Finds the histograms of segments found with FindSegments(). This function is separated from FindSegments() because it
	* only has to be called when the LABEL_ACQUIRE_HISTOGRAM flag is specified and because it can better be optimized for the
	* DSP this way.
	*/
	void				FindSegmentsHistogram( const Uint8 * restrict pGrayLine, LineSegment * restrict pSegments, const Int nNumSegments, const int nHistogramWidth );

	/**
	* Creates the connectivity matrix for two adjacent image lines that already have been transformed into segments.
	* A connectivity scheme of neighborhood of four is used.
	*
	* See the cpp file for further explanation.
	*/
	void				ConnectSegments4(	const LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
											const LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments,
											Uint8 * restrict pConnectivity );

	/**
	* Creates the connectivity matrix for two adjacent image lines that already have been transformed into segments.
	* A connectivity scheme of neighborhood of eight is used.
	*
	* See the cpp file for further explanation.
	*/
	void				ConnectSegments8(	const LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
											const LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments,
											Uint8 * restrict pConnectivity );

	/**
	* Resolves the equivalency matrix.
	*
	* See the cpp file for further explanation.
	*/
	void				ResolveEquivalence( Uint8 * restrict pConnectivity, 
											const Uint32 unNumUpperSegments, const Uint32 unNumLowerSegments );
											
	/**
	* Detects the segments of the upper line that are now (newly) connected by the lower segments and don't
	* yet have the same label. Then, these lines' labels are merged.
	*
	* See the cpp file for further explanation.
	*/
	void				MergeUpperSegments( const Uint8 * restrict pConnectivity, 
											LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
											const Uint32 unNumLowerSegments, 
											FastLabelObject * restrict pLabelObjects );

	/**
	* Labels all segments of the lower line. Either the segments receive a label a connected segment
	* of the upper line or a new label is allocated.
	*
	* See the cpp file for further explanation.
	*/
	void				LabelLowerSegments( const Uint16 unCurLine,
											const Uint8 * restrict pConnectivity,
											LineSegment * restrict pUpperSegments, const Uint32 unNumUpperSegments, 
											LineSegment * restrict pLowerSegments, const Uint32 unNumLowerSegments, 
											FastLabelObject * restrict pLabelObjects, Int & nNumLabelsUsed );

	/**
	* Adds a segment to a label object. Sums up all moment accumulators and the histograms and determines
	* the new bounding box.
	*/	
	void 				AddSegmentToLabel(	const Uint16 unCurLine, 
											const LineSegment * restrict pSegment, const Uint16 * restrict pHistogram,
											FastLabelObject * restrict labelObj );
	
	/**
	* Finalizes a label objects after the complete image has been processed. Unused objects
	* are removed from the list (which is an array). Likewise, objects that are too near to
	* the border or too small are discarded. The result of this operation is that all valid
	* objects are nicely arranged in the array, starting with index 1 up to numObjects.
	*/
	void				FinalizeObjects(	const Int nNumLabelsUsed,
											const Uint32 unInputWidth, const Uint32 unInputHeight );

	/**
	* Transforms all label's pixel coordinates according to the CVisTransform object of this component.
	* This function may only be called after FinalizeObjects() has succeeded (in fact, it is called from
	* within FinalizeObjects() now, if the corresponding flag is set).
	*/
	void				TransformCoordinates( );
	

#ifdef _WINDOWS
	void				PrintEquivalenceMatrix( Uint8 * pMatrix, Uint32 unNumUpperSegments, Uint32 unNumLowerSegments ) ;
	void				DrawHorizLine( const Int32 x1, const Int32 x2, const Int32 y, const Uint8 color);
	void				DrawVertLine( const Int32 x, const Int32 y1, const Int32 y2, const Uint8 color);
	void				DrawPixel( Uint8 * pImage, const Int32 x, const Int32 y, const Uint32 color);
#endif

	/** A buffer to hodl the next input line. */
	Uint8 *				m_pGrayNextLine;

	/** A buffer for the current line's segments. */
	LineSegment	*		m_segCurrentSegments;

	/** A buffer for the current line's segments' histograms. */
	Uint16 *			m_pCurrentSegmentHistograms;

	/** A buffer for the last (i.e. upper) line's segments. */
	LineSegment	*		m_segLastSegments;

	/** A buffer to hold the connectivity matrix. */
	Uint8 *				m_pConnectivity;

	/** The input port. Must be an 8 bit image type. */
	CVisInputPort		m_iportInput;

	/** The output port carrying the label objects array. The number of labels is stored in the 
	*   Labelobject of index 0. */
	CVisOutputPort		m_oportLabels;

	/** A buffer to hold the additional histogram information of the output label objects. If enabled,
	*   then in each label object there is a pointer that references to somewhere in this buffer. */
	Uint16 *			m_pHistograms;

	/** A buffer to hold the additional information of the transformed coordinates of each label object.
	*   Like m_pHistograms, each label object has a pointer referencing to a place in this buffer, but
	*	only if the transform feature is enabled. */
	TransformedCoords *	m_pTransformedCoords;

	Int					m_nMaxSegments;
	Int					m_nHistogramWidth;
	Int					m_nMaxObjects;
	Uint32				m_unFlags;

	/** The transformation object that transforms coordinates if that feature is enabled. */
	CVisTransform *		m_pTransform;

	/** Defines the minimum area an objects has to have to be accepted (in pixels). */
	CVisProperty		m_propMinObjectArea;
	Int					m_nMinObjectArea;

	/** This property defines the minimum required distance of an object to the border of the image. */
	CVisProperty		m_propMinDistanceToBorder;
	Int32				m_nMinDistanceToBorder;
	
	/** A property that stores the maximum number of contours used. This is thought as "read-only"
	*   property to see how many labels are required in full operation. */
	CVisProperty		m_propMaxNumLabelsUsed;
	Int32				m_nMaxNumLabelsUsed;	

	/** A property that stores the maximum number of segments used. This is thought as "read-only"
	*   property to see how many segments are required in full operation. */
	CVisProperty		m_propMaxNumSegmentsUsed;
	Int32				m_nMaxNumSegmentsUsed;	
	
	
	
	// DEBUG:::
	Int					m_nProf1;
	Int					m_nProf2;
	Int					m_nProf3;
	Int					m_nProf4;
	

};

#endif

