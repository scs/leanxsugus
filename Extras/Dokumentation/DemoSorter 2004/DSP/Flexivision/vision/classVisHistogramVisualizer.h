/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISHISTOGRAMVISUALZIER_H_
#define _CLASSVISHISTOGRAMVISUALZIER_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* @brief The histogram visualizer is used to visualize histograms.
* The histogram visualizer is used to visualize histograms.
*/
class CVisHistogramVisualizer : public CVisComponent
{
public:
	CVisHistogramVisualizer( const char * strName, const Int nHistogramWidth, const Int nHeight );

	void				DoProcessing();
	
protected:

	void				VisualizeDecimal( const Uint32 * restrict pHistogram, Uint32 * restrict pImage );

	CVisInputPort		m_iportHistogram;
	CVisOutputPort		m_oportOutput;

	CVisProperty		m_propColor;
	Int					m_nColor;

	CVisProperty		m_propLog;
	Int					m_nLog;

	Int					m_nBackgroundColor;

};


#endif




