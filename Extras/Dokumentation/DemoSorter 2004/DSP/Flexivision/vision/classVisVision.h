/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISVISION_H_
#define _CLASSVISVISION_H_

#include "classVisObject.h"
#include "classVisOutputPort.h"
#include "classVisComponent.h"

/**
* @brief The main vision datapath class.
*
* This is the main vision datapath class, which implements a specific image processing 
* algorithm. It's purpose is to allocate and connect the neede vision components and
* enable easy access to them.
*
* This class should be overriden to implement the specific datapath. The modules should
* be created and connected in its constructor. Also, FeedImage() must be overridden to
* guide the new image to the right component(s).
*
* As of yet, the DoProcessing() function also has to be overriden to implement hand-made
* scheduling.
*/
class CVisVision : public CVisObject
{
	enum VisionConsts {
		VIS_MAX_CHANNELS = 4,
		VIS_MAX_EVENTS = 16
	};
	
public:
	/**
	* Constructor. Allocate and connect all needed modules.
	*/
						CVisVision( const Char * strName );
	
	/**
	* Feeds a new image to the image processing path. This function should be overridden
	* to pass the image to the right component.
	*/
	void				FeedImage( Ptr pBuffer );
	
	/**
	* Sets the timestamp of the next image to be processed.
	*/
	void				SetCurrentImageTime( Uint32 unTime );
	
	/**
	* Enables a view on a specific port. Each port can be configured to be put on a specified
	* channel.
	*/
	bool				EnableViewPort( Uint32 unChannel, const Char * strComponentName, const Char * strPortName);
	
	/**
	* Disables a formerly enabled channel.
	*/
	void				DisableViewPort( Uint32 unChannel );
	
	/**
	* Feeds a new buffer to the viewport. The main application may retrieve this buffer
	* after the DoProcessing step of the vision object. It is expected that the main
	* application is changing the buffer from pass to pass.
	*/
	void				FeedViewPortBuffer( Uint32 unChannel, Ptr pBuffer );
	
	/**
	* Returns the image information of an enabled viewport.
	*/
	bool				GetViewPortImageInfo( Uint32 unChannel, Uint32 & width, Uint32 & height, Uint32 & bpp, bool & bData, bool & bIndexed );
	
	bool				SetParameter( const Char * strComponent, const Char * strPort, float value );
	
	/**
	* Prepares all ports and stuff...
	*/
	bool				Prepare();
	
	/**
	* Sets this vision's processing mode.
	*/
//	virtual void		ChangeMode( VisionMode newMode ) ;

	/**
	* Sets all components' operation mode
	*/
	void				SetComponentsOperationMode( CVisComponent::OperationMode mode );
	
	
	/**
	* Processes the last picture fed to the vision object.
	*/
	void				DoProcessing();	

	/**
	* Copies all open viewport channels to the specified buffers. This function must
	* be used by a sub-class.
	*/
	void				CopyViewPorts();
	
	/**
	* Enumerates all output ports of the system. Start with an index of -1.
	*/
	Bool				EnumOutputPorts( Int & nIndex, const Char * & strComponent, const Char * & strPort );
	
	/**
	* Directly sets a property.
	*/
	Bool				SetProperty( const Char * strComponent, const Char * strProperty, float fValue );
	Bool 				GetProperty( const Char * strComponent, const Char * strProperty, float & fValue );

	/**
	* Sets or gets a certain property.
	*/
	Bool 				AccessProperty( const Bool bSet, const Char * strComponent, const Char * strProperty, float & fValue );

	/**
	* Enumerates all properties of the system. Start with an index of -1.
	*/
	Bool 				EnumProperties( Int & nIndex, const Char * & strComponent, const Char * & strProperty, float & fValue );

	/**
	* Returns whether a specific event has occured.
	*/
	Bool				EventOccured( Int nEventId );
	
	/**
	* Sets an event.
	*/
	void				SetEvent( Int nEventId );
	
	/**
	* Clears all events. This may be used by the vision objects, depending on whether an event should
	* be able to trigger images later than when the event actually occured. If only the event-generating
	* images should be allowed to be received by the client application, ClearEvents() has to be called
	* prior any processing for each frame. If it is not called, the event persists until used by the client,
	* which may enable it to receive images much later.
	*/
	void				ClearEvents();
	
protected:	

	Uint32				m_unCurrentImageTime;
	
	/**
	* Stores the buffer information for viewport operation. This essentially does the translation from
	* the output port the viewchannel should transport to the viewchannel's memory buffer. After each
	* frame, the output port's buffer is copied to the viewport's buffer.
	*/
	struct ChannelInfo
	{	
		CVisOutputPort *	pPort;
		Ptr 				pBuffer;
	};
	
	/** The array where the channels are stored. */
	ChannelInfo			m_aryChannels[VIS_MAX_CHANNELS];	
	
	/** The array that stores the events. */
	Bool				m_aryEvents[VIS_MAX_EVENTS];
};

#endif
