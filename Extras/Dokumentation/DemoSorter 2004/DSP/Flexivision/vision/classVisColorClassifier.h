/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISCOLORCLASSIFIER_H_
#define _CLASSVISCOLORCLASSIFIER_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisFastLabel.h"
#include "classVisColorPick.h"

#include "classVisDemoSorterModel.h"

/**
* @brief The color classifier.
* The color classifier.
*/
class CVisColorClassifier : public CVisComponent
{
public:
							CVisColorClassifier( const Char * strName );

	void					DoProcessing();


protected:
	
	CVisInputPort			m_iportLabelData;
	CVisInputPort			m_iportColorData;
	


};


#endif

