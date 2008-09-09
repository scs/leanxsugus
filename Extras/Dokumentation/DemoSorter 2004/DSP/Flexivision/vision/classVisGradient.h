/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISGRADIENT_H_
#define _CLASSVISGRADIENT_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* @brief A gradient generating component.
*
* A gradient generating component.
*/
class CVisGradient : public CVisComponent
{
public:
						CVisGradient( const Char * strName );
						
	void				Prepare();						
						
	void				DoProcessing();						
	
protected:
	Uint32				m_unGradientType;
	
	CVisInputPort		m_iportInput;
	CVisOutputPort		m_oportResult;
};

#endif
