/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISREGIONSELECT_H_
#define _CLASSVISREGIONSELECT_H_

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisTransform.h"

/**
* @brief Region selecting component.
*/
class CVisRegionSelect : public CVisComponent
{
	enum RegionSelectConsts {
		RS_MAXVERTICES	= 10
	};

public:
	enum RegionSelectMode
	{
		RS_MASKINPUTIMAGE,
		RS_CREATELIMITSVECTOR_HORIZ,
		RS_CREATELIMITSVECTOR_VERT
	};

						CVisRegionSelect( const char * strName, CVisPort::PortDataType pType, RegionSelectMode	rsMode, const int numLimits = 0 );

	void				Prepare();

	void				DoProcessing();

	void				MaskInputImage();

	void				SetTransform( CVisTransform * pTransform );

	void				SetLimit( const float fLeft, const float fTop, const float fRight, const float fBottom ); 
	
private:
	void				DrawWorldRect();

	void				CreateLimitsVectorHoriz();

	void				Mask_32(	const Uint8 * restrict pMask, Uint32 * restrict pDest, 
									const int nWidth, const int nHeight );

	void				DrawQuad(	const int x1, const int y1,
									const int x2, const int y2,
									const int x3, const int y3,
									const int x4, const int y4 ); 


	RegionSelectMode	m_rsMode;

	CVisInputPort		m_iportInput;
	CVisOutputPort		m_oportMask;
	CVisOutputPort		m_oportOutput;
	CVisOutputPort		m_oportLimits;
	CVisTransform *		m_pTransform;		
	Ptr					m_pInputLine;

	CVisProperty		m_propLimitLeft;
	float				m_fLimitLeft; 

	CVisProperty		m_propLimitRight;
	float				m_fLimitRight; 

	CVisProperty		m_propLimitTop;
	float				m_fLimitTop; 

	CVisProperty		m_propLimitBottom;
	float				m_fLimitBottom;
};

#endif
