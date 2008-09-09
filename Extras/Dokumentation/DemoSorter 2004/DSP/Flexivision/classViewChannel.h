/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVIEWCHANNEL_H_
#define _CLASSVIEWCHANNEL_H_

#include "FlexiVisioncfg.h"

#include "libBufferQueue.h"
#include "libPicture.h"
#include "classBufferFifo.h"

#include "vision/classVisVision.h"

/**
* @brief A fifo-like entity that transports images from the image processing to the control task.
*
* A fifo-like entity that transports images from the image processing to the control task. The view
* channels are the only mean of providing image access to the GUI.
*/
class CViewChannel
{
public:
	/**
	* Constructor.
	*/
						CViewChannel();
						
	/**
	* Destructor.
	*/
						~CViewChannel();

	/**
	* Initializes this view channel, allocating all the needed memory for the queue.
	* Sets the vision object and thus the source of images for this view channel object.
	* 
	* Note: Don't call this function from a constructor of a statically allocated object,
	*		since DSP/BIOS must be up and running here.
	*/						
	Bool				Init( int nChannelNum, CVisVision * vision, Int nNumBuffers, Int nBufsize, Int nSeg, Int nCollectorSize );

	/**
	* Configures the channels to one of the vision network's ports. The port that 
	* should be observed must be specified using the port's string identifier (comp.port notation).
	* The rate defines how many pictures are dropped until one is put to the queue.
	*/
	Bool 				Configure( const char * strPort, int nDropRate);
	
	/**
	* Takes a snapshot from an already configured viewport. The viewport must not be enabled (if it is,
	* this command is simply ignored). The picture that is finally getting through is the next available
	* picture on the configured port.
	*/
	Bool				TakeSnapshot();
	
	/**
	* Takes a snapshot when an event occurs. 
	*/
	Bool 				TakeEventSnapshot( const Int nEventId );
	
	/**
	* Enables a previously configured channel. This starts the image processing tasks to put images
	* on the queue at the configured rate.
	*/
	Bool				Enable( Bool bEnable );
	
	/**
	* Returns whether the viewport is already enabled.
	*/
	Bool				IsEnabled();
	
	/**
	* Gets the currently topmost buffer from the view channel queue.
	* The function does not block and simply returns FALSE when there were no
	* buffers to get.
	*
	* Each call to GetViewBuffer() must be followed by a call to ReleaseViewBuffer()
	* to release the buffer after use.
	*/
	Bool				GetPicture( Picture * & pPicture );
	
	/**
	* Releases a view channel buffer.
	*
	* This function must be called after GetViewBuffer() to release the acquired buffer.
	*/
	Bool				ReleasePicture( Picture * pPicture );
	
	/**
	* Feeds the allocated view channel buffer to the configured port object.
	*/
	void 				FeedBuffer( );
	
	/**
	* Puts the buffer of the view channel to its queue, with taking the drop
	* rate into account. This is only done, if the channel is enabled.
	*/
	void 				ConcludeBuffer( );
	
protected:	

	/**
	* Allocates a buffer from one of the view channel queue. This function must only be called
	* directly once. For the further times, PutAllocViewChannelBuffer() will call it.
	*
	* The function returns FALSE if a buffer has already been allocated for that channel.
	*/
	Bool 				AllocBuffer( );
	
	/**
	* Puts a buffer to the view channel queues and acquires a new one. If the acquisition
	* fails, the channel is staying with the last buffer.
	*/
	Bool 				PutAllocBuffer( );
	
	
	CVisVision *		m_pVision;
	
	Int					m_nChannelNum;
	
	BufferQueue			m_qQueue;
		
	Int					m_nDropRate;
	Int					m_nCurFrame;
	
	Int					m_nPicType;
	Int					m_nWidth;
	Int					m_nHeight;
	Int					m_nPicSize;
	
	Picture *			m_pCurrentPicture;
			
	Bool				m_bEnabled;
	Bool				m_bTakeSnapshot;
	
	Bool				m_bWaitEvent;
	Bool				m_bCollectEventPictures;
	Int					m_nEventId;
	
	/** This is the event pictures collector. It is meant to collect and temporarily stores images
	 *  when the client is collecting images of events. Since the client cannot get the images
	 *  as fast as they're coming through here, the collector's purpose is to store them until
	 *  the client is able to get them.
	 */
	CBufferFifo			m_EventPicturesCollector;
	Bool				m_bHaveEventPicturesCollector;
};


#endif
