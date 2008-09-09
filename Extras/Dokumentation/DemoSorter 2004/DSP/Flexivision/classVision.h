/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISION_H_
#define _CLASSVISION_H_

#include "vision/classVisVision.h"
#include "vision/classVisFrameGrabber.h"
#include "vision/classVisClip.h"
#include "vision/classVisRGBToGray.h"
#include "vision/classVisThreshold.h"
#include "vision/classVisMorphology.h"
#include "vision/classVisKeypointsCut.h"
#include "vision/classVisFastLabel.h"
#include "vision/classVisColorPick.h"
#include "vision/classVisDemoSorterClassifier.h"
#include "vision/classVisColorPickVisualizer.h"
#include "vision/classVisLabelVisualizer.h"
#include "vision/classVisModelVisualizer.h"

#include "vision/classVisDemoSorterModel.h"
#include "vision/classVisCamPlaneTransform.h"

/**
* @brief The main vision network class.
*
* The main vision class, which assembles the vision network and performs maintenance operations
* like property setting and mode changes. It also provides an interface to set and get the calibration
* data. The vision object is used by the image processing task, which feeds images to it and also
* calls the DoProcessing() functions.
*/
class CVision : public CVisVision
{
public:
	enum VisionMode {
		VM_IDLE					= 0,
		VM_SERVICE				= 1,
		VM_CLASSIFICATION		= 2,
		VM_CALIBRATION			= 3
	};
	
	enum VisionEvents {
		VE_OBJECTS_FOUND		= 0
	};

						CVision();	
	
	/**
	* Feeds a new image to the vision framework.
	*/
	void				FeedImage( Ptr pBuffer );	
	
	/**
	* The main processing function.
	*/
	void				DoProcessing();	

	/**
	* Changes the vision algorithm's operation mode.
	*/
	void				ChangeMode( VisionMode newMode );
	VisionMode			GetMode( );
		
protected:
	VisionMode					m_vmMode;

	CVisFramegrabber			m_compFrameGrabber;
	CVisClip					m_compClip;
	CVisRGBToGray				m_compRGBToGray;
	CVisThreshold				m_compThreshold;
	CVisMorphology				m_compMorphology;
	CVisKeypointsCut			m_compKeypointsCut;
	CVisFastLabel				m_compLabel;
	CVisColorPick				m_compColorPick;
	CVisDemoSorterClassifier	m_compDemoSorterClassifier;
	CVisLabelVisualizer			m_compLabelVisualizer;
	CVisColorPickVisualizer		m_compColorVisualizer;
	CVisModelVisualizer			m_compModelVisualizer;
	

	CVisDemoSorterModel			m_compModel;
	CVisCamPlaneTransform		m_compCoordTransform;

};

#endif
