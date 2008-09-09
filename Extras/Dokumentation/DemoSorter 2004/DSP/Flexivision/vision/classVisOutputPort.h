/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISOUTPUTPORT_H_
#define _CLASSVISOUTPUTPORT_H_


class CVisOutputPort;

#include "classVisPort.h"
#include "classVisInputPort.h"
#include "classVisBufferManager.h"

/**
* @brief An output port class for image processing components.
*
* An output port class for image processing components. Output ports may be connected
* to input ports. Output ports are responsible to allocate the memory needed by the 
* image processing component. There should be one output port for each kind of output
* the component delivers (e.g. contoured image vs. contour info).
*
* Multiple input ports can be connected to a single output port and thus share the 
* data buffers.
*/
class CVisOutputPort : public CVisPort
{
	friend class CVisInputPort;	

public:
	enum OutputPortConsts {
		OUTPORT_FAST = 			CVisBufferManager::BUF_FAST,
		OUTPORT_NON_CACHED = 	CVisBufferManager::BUF_NON_CACHED,
		OUTPORT_LARGE = 		CVisBufferManager::BUF_LARGE,
		
		OUTPORT_DOUBLEBUFFER =	0x10000000, 
		OUTPORT_HOLLOW 		 =  0x20000000,
		
		OUTPORT_MAX_CONNECTEDINPUTS = 8		
	};
	
	/**
	* Constructor.
	*/
						CVisOutputPort( const Char * strName, CVisPort::PortDataType dataType, Uint32 unFlags = 0 );
	
	/**
	* Connects an output port to an input port.
	*/
	void				Connect( CVisInputPort * pInputPort );
	
	bool				Prepare();
	
	bool				GetImageSize( Uint32 & unWidth, Uint32 & unHeight );

	/**
	* Swaps the buffers in case this is a double buffered outport.
	*/
	void				SwapBuffers();

	/**
	* Gets the background buffer if this is a double buffered port.
	*/
	Ptr					GetBackBuffer();
	
	
	/**
	* Informs the connected input ports of a the new buffer pointer.
	* The function is made public so that an ordinary output buffer
	* can be used by the framegrabber.
	*
	* TODO: this is sort of a hack. If real C++ is alowed at any time,
	*		do a nice subclassing of this class (VisFGOutputPort...).
	*/
	void				PropagateBuffer( );
	
protected:
	/**
	* Tells the port that it's being connected to by another (input) port.
	*/
	void				Connected( CVisInputPort * pInputPort );

	/**
	* Tells the output port that an input port has been disconnected.
	*/
	void				Disconnected( CVisInputPort * pInputPort );

	
	/** A list of input ports to which this output port is connected. */
	CVisInputPort *		m_aryInputPorts[OUTPORT_MAX_CONNECTEDINPUTS];
	
	/** The memory flags of this port. */
	Uint32				m_unFlags;

	/** Flags to indicate that this port is double buffered. */
	Bool				m_bDoubleBuffered;
	
	/** Flag to indicate that this port is a hollow port and thus won't need to
	*   allocate a buffer. */
	Bool				m_bHollow;

	/** A pointer to the other buffer, in case this is a double buffered port. */
	Ptr					m_pBackBuffer;
};


#endif
