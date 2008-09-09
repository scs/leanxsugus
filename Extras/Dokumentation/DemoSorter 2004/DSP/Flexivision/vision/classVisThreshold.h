/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISTHRESHOLD_H_
#define _CLASSVISTHRESHOLD_H_

#include "classVisComponent.h"
#include "classVisPort.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

extern "C"
{
#include "img_thr_gt2max.h"
#include "img_thr_gt2thr.h"
#include "img_thr_le2min.h"
#include "img_thr_le2thr.h"
}

/**
* @brief A threshold component.
*/
class CVisThreshold : public CVisComponent
{
	
public:

	enum ThresholdType
	{
		TT_CLAMP_TO_MAX,
		TT_CLIP_ABOVE,
		TT_CLAMP_TO_ZERO,
		TT_CLIP_BELOW
	};
	
								CVisThreshold( const Char * strName, ThresholdType ttType );
								~CVisThreshold();


	void						DoProcessing();

protected:
	void						Threshold_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int cols, const Int rows, const Uint8 unThreshold, const ThresholdType ttType );
	CVisPort::PortDataType		m_pdtDataType;

	CVisInputPort				m_iportInput;
	CVisOutputPort				m_oportOutput;

	CVisProperty				m_propThreshold;
	Int							m_nThreshold;

	ThresholdType				m_ttType;

};


#endif



