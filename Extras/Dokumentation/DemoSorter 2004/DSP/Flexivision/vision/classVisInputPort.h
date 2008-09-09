/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISINPUTPORT_H_
#define _CLASSVISINPUTPORT_H_

class CVisInputPort;

#include "classVisPort.h"
#include "classVisOutputPort.h"

/**
* @brief An input port class for image processing components.
*
* An input port class for image processing components. Input ports may be collected
* to output ports. Input ports do not request or allocate any buffer memory by default,
* since that is done by the output ports. They may however provide some means of data
* conversion and hence must allocate some buffer space.
*/
class CVisInputPort : public CVisPort
{
	friend class CVisOutputPort;	
public:
	/**
	* Constructor.
	*/
	CVisInputPort( const Char * strName, CVisPort::PortDataType dataType );
	
	/**
	* Connects an input port to an output port.
	*/
	void				Connect( CVisOutputPort * pPort );

	void				Disconnect( );
	
	bool				Prepare() { return true; };
	
	bool				GetImageSize( Uint32 & unWidth, Uint32 & unHeight );
	
protected:

	/**
	* Tells the port that it's being connected to by another (output) port.
	*/
	void				Connected( CVisOutputPort * pPort );
	
	/** The output port this input port is connected to. */
	CVisOutputPort *		m_pOutputPort;

};


#endif
