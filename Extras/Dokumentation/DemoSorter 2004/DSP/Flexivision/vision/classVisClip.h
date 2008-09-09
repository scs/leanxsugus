/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISCLIP_H_
#define _CLASSVISCLIP_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* @brief Clips the image down to a certain size.
*
* Clips the image down to a certain size.
*/
class CVisClip : public CVisComponent
{
public:
						CVisClip( const Char * strName, CVisPort::PortDataType pdtType, Uint32 unOffsX, Uint32 unOffsY, Uint32 unWidth, Uint32 unHeight );
						~CVisClip();
	
	void				DoProcessing();
	
protected:
	CVisInputPort		m_iportImage;
	CVisOutputPort		m_oportImage;	
	
	Uint32 				m_unOffsX;
	Uint32 				m_unOffsY;
	Uint32				m_unWidth;
	Uint32				m_unHeight;
	
	

};


#endif
