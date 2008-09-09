/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISTRACKER_H_
#define _CLASSVISTRACKER_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisFastLabel.h"

/** 
* This is the structure that is used to build the list of currently known potato objects.
*/
struct PotatoObject
{
	enum PotatoObjectConsts
	{
		/** 
		* The number of partial bits in this struct's fixed point numbers.
		* Fixed point numbers are used by the classification values.
		*/
		FP_FRACTIONAL_BITS = 16,
		
		/**
		* The maximum number of frames that is used to classify a potato.
		* Since classification results of each frame are stored, this value 
		* defines the size of that list.
		*/
		MAX_FRAMES_PER_POTATO = 20
	};

	Uint32				unPotatoClass;
	Uint32				unPotatoLength;
	Uint32				unPotatoWidth;
	Int					nLastSeenPos_mm_X;
	Int					nLastSeenPos_mm_Y;

	/** The time at which the potato was last seen. In ms. */
	Uint32				unLastSeenTime_ms;

	Uint32				unLastSeenImage;
	Int					nPredictedPos_mm_X;
	Int					nPredictedPos_mm_Y;

	/**
	* The number of the current image seen of this potato. This increments whith each
	* frame that the potatoe could succesfully be tracked. This number also defines the
	* number of classification results available in the classification result arrays. So,
	* 
	*/
	Uint32				unCurrentImageNum;
	
	/**
	* A reference to an entry in the current frame's label list. That label is the contouring
	* object that was identified (i.e. tracked) as this potato.
	*/
	Uint32				unCurrentLabel;

	/**
	* Stores the bounding box in the local coordinate system of the images created by the cutter.
	* This value is only valid after the cutter has processed the image.
	*/
	Uint16				unLocalBoundingLeft;
	Uint16				unLocalBoundingTop;
	Uint16				unLocalBoundingRight;
	Uint16				unLocalBoundingBottom;

	/**
	* These are the classification results for each frame. Each of the classification
	* components write their result to those arrays, frame by frame. The classification
	* task may then retrieve this data and generate an overall classification result
	* for this potato.
	*/
	Uint32				unpClassificationColor[MAX_FRAMES_PER_POTATO];
	Uint32				unpClassificationGreen[MAX_FRAMES_PER_POTATO];
	Uint32				unpClassificationForm[MAX_FRAMES_PER_POTATO];
	Uint32				unpClassificationSplit[MAX_FRAMES_PER_POTATO];

	/**
	* This flag is set when the potato could succesfully be tracked in the current frame.
	* It is only valid during the current frame and shouldn't be accessed by any other
	* task than the tracker.
	*/
	bool				bTracked;
	
	/** 
	* This flag defines whether the potato object is valid (and in use) or not. 
	* The flag may only be set by the VisTracker module and only be cleared by
	* the (asynchronous) classification task.
	*/	
	bool				bValid;
	
	/**
	* This flag is set when the potato could succesfully be tracked in the valid
	* drop zone. This implies that, when the potato reaches the drop zone, it may
	* be classified and the appropriate ejection commands may be generated.
	*/
	bool				bValidEjection;
	
	/**
	* This flag is set when the potato's predicted position reaches the drop zone and
	* thus must be cleared by the classification task, which may, depending on the
	* above flag, generate ejection commands for it, or not.
	*/
	bool				bDropIt;
	
	/**
	* A flag that tells the classification task to discard this object as soon as possible.
	* The discard flag is used to remove objects that couldn't be tracked for a too long time.
	*/
	bool				bDiscard;

	/**
	* Predicts this potato's position and stores the value in unPredictedPos_mm_*. The current
	* time (in global milliseconds) has to be specified as well as the current conveyor's speed
	* in fixed point (Q.16) m/s format.
	*/
	void				PredictPosition(	Uint32 unCurTime, Int fp16ConveyorSpeed);
};

/**
* This is the list of currently known potatoes. 
*/
struct PotatoList
{	
	enum PotatoListConsts {
		MAX_OBJECTS = 256		
	};
	
	/**
	* This variable is used by the cutter to identify the potato currently
	* being processed.
	*/
	Uint32				unCurrentPotatoId;

	/**
	* A list of the potato objects.
	*/
	PotatoObject		pObjects[ MAX_OBJECTS ];
};
	

/**
* @brief The potato tracker class.
*
* The potato tracker class, which tracks all known potatoes.
*/
class CVisTracker : public CVisComponent
{
public:
	
	/**
	* Constructor.
	*/
							CVisTracker( const Char * strName );
	
	/**
	* The main processing function.
	*/
	void					DoProcessing( );

	void					SetCurrentImageTime( Uint32 unMilliseconds );
	
	void					SetConveyorSpeed( Uint32 fp16Speed );
	
	/**
	* Tells the tracker that the processing of a specific potato has completed.
	* This will increment the counter of the number of classified frames per
	* potato. Due to the asynchronous nature of the classification task, this
	* may only be done AFTER all classification results are written to the result
	* arrays.
	*
	* DoneWithPotato() must be called by the vision object.
	*/
	void 					DoneWithPotato( const Uint32 unPotatoId );


protected:
	/**
	* Clears the objects list. This usually has to be done before the first
	* processing step or when there was a long pause (e.g. when switching from
	* calibration to classification).
	*/
	void					ClearList();
	
	/**
	* Tracks all objects given in the potObj list and tries to match them with the 
	* labels in labelObj list.
	*/
	void					TrackObjects( FastLabelObject * labelObjects, PotatoObject * potObjects );
	
	/**
	* Adds all not yet tracked objects to the list of known objects. This must be called
	* after the known objects are have been tracked by TrackObjects().
	*/
	void 					AddNewObjects( const FastLabelObject * labelObjects, PotatoObject * potObjects );

	/** A flag that triggers the clearing of the potato list in the next processing step. */
	bool					m_bClearListNext;

	PotatoObject *			m_pPotatoList;

	CVisInputPort			m_iportLabelObjects;
	CVisOutputPort			m_oportPotatoObjects;

	/**
	* Defines the millimeters per pixel, representation is in fixed point
	* format with 16 bits after the point.
	*/
	Uint32					m_fpMillimeterPerPixel;
	CVisProperty			m_propMillimeterPerPixel;

	/**
	* Defines the conveyor's speed pixels per millisecond (if a pixel equals a mm, the speed is in m/s), 
	* representation is in fixed point format with 16 fractional bits (Q.16).
	*/
	Uint32					m_fpConveyorSpeed;

	/**
	* Defines the current frame's timestamp.
	*/
	Uint32					m_unCurrentFrameTime;

	/**
	* Defines the radius from the predicted position in which to search for an object.
	* The radius is stored ^2 and its unit is pixels.
	*/
	Int32					m_nSearchRadius2;
	CVisProperty			m_propSearchRadius2;

	/**
	* The zone in which new label objects may be inserted to the potato objects list.
	* The insertion zone spans from the top of the image (where the potatoes are
	* supposed to enter the image...) as many pixels down as given by this parameter.
	*/
	Int32					m_nInsertionZone;
	CVisProperty			m_propInsertionZone;

	/**
	* This zone defines the area of the image where potatoes are dropped off the tracker's
	* list and, probably, ejected by the classification task. This area must not be larger
	* than the average y distance between two potatoes, so that only one potato per column
	* may be dropped at a time.
	*
	* The drop zone spans from the bottom of the image upwards, as many pixels as specified.
	*
	* Note: The drop zone and the insertion zone mustn't overlap! Dropped potatoes would otherwise
	*		be immediately added to the list again.
	*/
	Int32					m_nDropZone;
	CVisProperty			m_propDropZone;
	
	/**
	* This zones defines the area in which a succesfull tracking of a potato leads to the 
	* generation of ejection commands for that potato. That means, if a potato could not be
	* tracked at least once in this zone, no ejection commands will be generated it it reaches
	* the drop zone (or, in this case, if it's predicted position reaches the drop zone).
	*
	* Like the drop zone, the valid ejection zone also spans from the bottom of the image upwards.
	*/
	Int32					m_nValidEjectionZone;
	CVisProperty			m_propValidEjectionZone;
	
	/**
	* This property defines the TIME after which an object should be deleted when it could not
	* be tracked. The drop time is specified in milliseconds.
	*/
	Int32					m_nDropTime;
	CVisProperty			m_propDropTime;
	

};


#endif

