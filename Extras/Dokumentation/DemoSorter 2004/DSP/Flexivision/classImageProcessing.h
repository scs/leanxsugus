/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSIMAGEPROCESSING_H_
#define _CLASSIMAGEPROCESSING_H_

class CImageProcessing;

#define VIEW_CHANNELS_NUM			4
#define VIEW_CHANNELS_DEPTH			2
#define VIEW_CHANNEL0_MAXIMGSIZE	(1380 * 1030 * 4)
#define VIEW_CHANNELS_MAXIMGSIZE	(1380 * 1030)

#include "FlexiVisioncfg.h"

#include "drvPPU.h"
#include "drvConveyor.h"

#include "classControl.h"
#include "classProfiler.h"
#include "classViewChannel.h"

// include vision stuff
#include "vision/classVisInputPort.h"
#include "vision/classVisOutputPort.h"
#include "vision/classVisComponent.h"
#include "classVision.h"

/**
* @brief The image processing task object.
*
* The vision task object, that is responsible for everything related to image
* processing. The vision task is a more important task in the system than the control
* task and thus runs with higher priority.
*
* The image processing task has a number of jobs:
* - Setup and Initiate the image reception
* - Allocate and initiate the vision network
* - Provide an interface to access the vision network and its components.
* - Provide image channels that transport images to the control task and then to the GUI.
* - Provide an interface to read and write the calibration data.
*/
class CImageProcessing
{
public:
	enum ImageProcessingMode
	{
		IPM_IDLE,
		IPM_SERVICE,
		IPM_PROCESSING,
		IPM_CALIBRATION
	};
	
		
						CImageProcessing();
						~CImageProcessing();
						
	/** Sets a reference to the control object */
	void				SetControl( CControl * control );

	/** Enters the main vision task.*/	
	void				StartTask();
		
	CViewChannel *		GetViewChannel( int nChannel );	
	
	/**
	* Enumerates all the output ports available in the system.
	* Start with an index of -1.
	*/
	Bool 				EnumOutputPorts( Int & nIndex, const Char * & strComponent, const Char * & strPort );
	
	/**
	* Sets or gets a certain property.
	*/
	Bool 				AccessProperty( const Bool bSet, const Char * strComponent, const Char * strProperty, float & fValue );
	
	Bool 				SetProperty( const Char * strComponent, const Char * strProperty, const float fValue );
	
	/**
	* Enumerates all the properties available in the system.
	* Start with an index of -1.
	*/
	Bool 				EnumProperties( Int & nIndex, const Char * & strComponent, const Char * & strProperty, float & fValue );
	
	Bool				SetMode( ImageProcessingMode ipmMode );
	ImageProcessingMode GetMode( );
	
	/**
	* Adjusts the color levels at the FPGA. The given values are in percent, where a value
	* of 0 means that this color isn't amplified at all.
	*/
	void				AdjustColorValues( Int nRed, Int nGreen, Int nBlue );
	
	void				SetMaxFramerate( Int nMaxFPS );
		
	void 				SetFrameTime( Int nFrameTime_us );

	
protected:	
	enum ImageProcessingConsts
	{
		IPC_MINSLEEP_MS				= 5,
		IPC_MAXSLEEP_MS				= 1000,
		IPC_FRAMES_PER_TRIGGER		= 4
	};

	/**
	* Feeds all allocated view channel buffers to the vision object.
	*/
	void 				FeedViewChannelBuffers( );
	
	/**
	* Puts the buffers of all enabled view channels to their queues, with takeing the drop
	* rate into account.
	*/
	void 				ConcludeViewChannelBuffers( );

	
	Int 				WriteFixedPoint(float f, Uint8 * buffer );
	Int 				WriteInteger(Uint32 i, Uint8 * buffer );
	
	/** 
	* Initializes the view channels. This must not be done in the constructor,
	* but only when the DSP/BIOS is up and running.
	*/
	Bool				InitViewChannels();

	/** Allocate some view channels. */
	CViewChannel		m_aryViewChannels[VIEW_CHANNELS_NUM];	

	/** 
	* A reference to the control object for message posting and profiler 
	* operation.
	*/	
	CControl *			m_Control;
	
	/** The main vision object. */
	CVision *			m_pVision;
	
	/** The device driver handle. */
	PPU_DevHandle 		m_hPPU;	
	
	/** The watchdog ID of this task. */
	Uint32				m_unWatchId;
	
	/** The profiler ID of the main processing task. */
	Uint32				m_unOverheadProfilerId;
	Uint32				m_unMainProfileId;
	
	/** A flag to halt the image processing. */
	Bool				m_bEnableProcessing;
	
	Int					m_nMaxFPS;
	Int					m_nFrameTime;
																
};

#endif
