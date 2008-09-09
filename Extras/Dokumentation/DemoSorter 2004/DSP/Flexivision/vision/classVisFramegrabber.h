/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISFRAMEGRABBER_H_
#define _CLASSVISFRAMEGRABBER_H_

#include "classVisComponent.h"
#include "classVisOutputPort.h"

/**
* @brief The framegrabber component.
*
* The framegrabber component, which is at the top of each processing chain, allows the feeding
* of images to the network.
*/
class CVisFramegrabber : public CVisComponent
{
public:
						CVisFramegrabber( const Char * strName, Uint32 unWidth, Uint32 unHeight, CVisPort::PortDataType pdtOutputType );
						
	void				Prepare();						
						
	void				SetInputBuffer( Ptr	pBuffer );
	void				DoProcessing();						
	
protected:
	
	CVisOutputPort			m_oportImage;
	Ptr						m_pImageBuffer;	
	
};

#endif
