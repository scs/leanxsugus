/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISPORT_H_
#define _CLASSVISPORT_H_

class CVisPort;

#include "classVisObject.h"
#include "classVisComponent.h"

/**
* @brief A general port class.
*
* A general port class that implements a port's most basic features. So, each
* port has a direction (input / output) and a pointer to a buffer associated
* with it. The buffer isn't allocated within CVisPort, though, this is done in
* a specialized sub-class, like CVisOutputPort.
*/
class CVisPort : public CVisObject
{
public:
	enum PortType {
		PT_INPUT,
		PT_OUTPUT
	};

	enum PortDataType {
		PDT_1BPP,

		PDT_2BPP_GRAY,
		PDT_4BPP_GRAY,
		PDT_8BPP_GRAY,

		PDT_2BPP_INDEX,
		PDT_4BPP_INDEX,
		PDT_8BPP_INDEX,

		PDT_24BPP_RGB,
		PDT_32BPP_RGB,

		PDT_24BPP_HSI,
		PDT_32BPP_HSI,

		PDT_DATA
	};

	
	/**
	* Constructor.
	*/
						CVisPort( const Char * strName, PortType type, PortDataType dataType );
						
	/**
	* Sets the port's data type after construction. This function is only provided for the 
	* CVisFramegrabber class and shouldn't be used from anywhere else. IF it is used, it
	* must be used before any Init() or Prepare() functions are called, since they used
	* the port type to calculate the buffer size.
	*/
	void				SetDataType( PortDataType type );
						
				
	/**
	* Initializes the port and registers it at the component. Memory allocation should 
	* not be done here, but in the Prepare() function.
	*/		
	void 				Init( CVisComponent * pComp );

	CVisComponent *		GetComponent();
	
	/**
	* Sets this port's buffer size, which is relevant when it is an output port. In case it is
	* an input port, it will inherit the buffer size from the output buffer it is connected to.
	*/
	void				SetBufferSize( Uint32 unBufSize );

	/**
	* Sets the port's image size, which, again, is only relevant when it is an output port.
	* The function, like SetBufferSize(), should be called in between Init() and Prepare(), so
	* that the required amount of memory is allocated in Prepare().
	*/
	void				SetImageSize( Uint32 unWidth, Uint32 unHeight );

	void				SetImageBPP( Uint32 unBpp );
	Uint32				GetImageBPP( );

	bool				IsIndexed();
	
	/**
	* Returns the port's buffer size.
	*/
	Uint32				GetBufferSize();
	
	/**
	* The purpose of this function is to prepare the ports for image processing. This means
	* that
	* 	- The buffer and image size and type must be known
	* 	- The buffers must, if this is an output port, be allocated
	*/
	bool				Prepare();
	
	/**
	* The connect function on CVisPort level. This function will determine which type of
	* port this is and call the appropriate CVisInputPort and CVisOutputPort connect() functions.
	*/
	bool				Connect( CVisPort * port );	
	
	/**
	* This will disconnect a previously connected port.
	*/
	void				Disconnect( );

	/**
	* Gets the port's type. 
	*/	
	PortType			GetType();

	/**
	* Returns the data type of this port.
	*/
	PortDataType		GetDataType();

	Uint32				GetBppFromType( PortDataType dataType );
	
	/**
	* Gets the port's buffer.
	*/
	Ptr					GetBuffer();
	
	/**
	* Sets the port's buffer.
	*/
	void				SetBuffer( Ptr pBuffer );

	/**
	* Gets the port to which this port is connected. 
	*/
	bool				GetConnectedPort( CVisPort * & port);
	
	/**
	* Invalidates the cache of this port's buffer. This only makes sense if the buffer is
	* allocated in cached SDRAM on the DSP.
	*
	* Invalidation of the cache must be performed when data was written to the cache using
	* DMA transfers and the component is about to access the buffer directly (i.e. not by
	* DMA but by CPU loads).
	*/
	void				CacheInvalidate();
	
	/*
	* Writes all data from the cache back to the buffer, if any. This only makes sense if the 
	* buffer is allocated in caches SDRAM on the DSP.
	*
	* Write-back of the cache must be performed after the component directly (i.e. not by
	* DMA) accessed the buffer and is now about to read from the buffer by DMA. 
	*/
	void				CacheWriteback();
	
protected:

	void				SetConnectedPort( CVisPort * port );

	/** The port's type. */
	PortType			m_ptType;

	/** The port's data type. */
	PortDataType		m_pdtDataType;
	
	/** A reference to the port's buffer. */
	Ptr					m_pBuffer;
	
	/** The buffer's size. */
	Int32				m_nBufferSize;
	
	/** The port's component. */
	CVisComponent * 		m_pComponent;

	/** The port to which this port is connected. */
	CVisPort *			m_pConnectedPort;
	
	Uint32				m_unWidth;
	Uint32				m_unHeight;
	Uint32				m_unBPP;
	
	
};

#endif
