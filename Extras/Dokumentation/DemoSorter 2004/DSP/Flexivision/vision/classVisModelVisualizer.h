
#ifndef _CLASSVISMODELVISUALIZER_H_
#define _CLASSVISMODELVISUALIZER_H_

#include "classVisDemoSorterModel.h"

#include "classVisComponent.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

#include "classVisTransform.h"
#include "classVisGraphics.h"

class CVisModelVisualizer : public CVisComponent
{
public:
							CVisModelVisualizer( const Char * strName, Int scale, Uint32 unFlags );

	
	/** Processing paints all the color information to screen as well as the label boxes. */
	void					DoProcessing();

	enum Flags
	{
		OUTPUT_ON_INPUTPORT = 1
	};
	
	void					SetTransform( CVisTransform * transform );
	void					SetModel( CVisDemoSorterModel * model );

protected:
	
	
	/**
	* Draws the model
	*/
	virtual void			DrawModel( Uint32 * outputImg, CVisTransform * transform );

	void					DrawLine( CVisGraphics * g, CVisTransform * transform, 
									const float x1, const float y1, const float z1,
									const float x2, const float y2, const float z2,
									Uint32 unColor );

	void					DrawLine( CVisGraphics * g, CVisTransform * transform, 
									const CVisFixpoint x1, const CVisFixpoint y1, const CVisFixpoint z1,
									const CVisFixpoint x2, const CVisFixpoint y2, const CVisFixpoint z2,
									Uint32 unColor );

protected:

	CVisTransform *			m_pTransform;
	CVisDemoSorterModel *	m_pModel;

	CVisInputPort			m_iportImage;
	CVisOutputPort			m_oportOutput;
	
	Int						m_nScale;

	Uint32					m_unFlags;
};

#endif
