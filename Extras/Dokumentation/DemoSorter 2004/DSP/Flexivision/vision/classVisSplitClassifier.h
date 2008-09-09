/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISSPLITCLASSIFIER_H_
#define _CLASSVISSPLITCLASSIFIER_H_


#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisLabel.h"
#include "classVisTracker.h"

/**
* This struct carries some information of a split that is later needed to visualize
* all splits found on a frame.
*/
typedef struct {
	Uint16		unBoundingLeft;
	Uint16		unBoundingTop;
	Uint16		unBoundingRight;
	Uint16		unBoundingBottom;
} SplitInfo;

/**
* This list is the data structure used by the m_oportSplitList port to provide
* information about the currently found splits. This information is mainly used
* by the visualizer. 
*/
typedef struct {
	enum SplitListConsts
	{
		MAX_ENTRIES = 10
	};

	Uint32		unNumEntries;

	SplitInfo	arySplitInfos[MAX_ENTRIES];

} SplitList;

/**
* @brief The split classifier.
*/
class CVisSplitClassifier : public CVisComponent
{
public:
						CVisSplitClassifier( const Char * strName );
						~CVisSplitClassifier();

	void				DoProcessing();

protected:
	Int					ClassifyLabelObjects(	LabelObject * restrict pLabelObjects, SplitList * restrict pSplitList,
												const Uint32 unBoundingLeft, const Uint32 unBoundingTop, 
												const Uint32 unBoundingRight, const Uint32 unBoundingBottom);
	
	CVisInputPort		m_iportLabels;
	CVisInputPort		m_iportObjectsList;
	CVisOutputPort		m_oportSplitList;


	Int					m_nMinArea;
	CVisProperty		m_propMinArea;

	Int					m_nMinLength;
	CVisProperty		m_propMinLength;

	Int					m_fpMinRatio;
	CVisProperty		m_propMinRatio;

	Int					m_nBoundingMargin;
	CVisProperty		m_propBoundingMargin;
};

#endif



