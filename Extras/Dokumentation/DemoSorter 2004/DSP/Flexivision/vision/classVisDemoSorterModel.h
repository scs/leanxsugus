
#ifndef _CLASSVISDEMOSORTERMODEL_H_
#define _CLASSVISDEMOSORTERMODEL_H_

#include "classVisComponent.h"
#include "classVisProperty.h"

#include "classVisFixpoint.h"

class CVisDemoSorterModel : public CVisComponent
{
public:
	CVisDemoSorterModel( const char * strName );


	void					SetWidth( const CVisFixpoint & fpWidth );
	const CVisFixpoint		GetWidth() const;

	void					SetHeight( const CVisFixpoint & fpHeight );
	const CVisFixpoint		GetHeight() const;

	void					SetNumJets( Int nNumJets );
	Int						GetNumJets() const;

	const CVisFixpoint		GetJetPosition( const int nNumJet ) const;
	Int						MapPosToLane( const CVisFixpoint & fpPosition ) const;	

	const CVisFixpoint		GetObjectsSize() const;

	void					GetTotalArea( CVisFixpoint & fpLeft, CVisFixpoint & fpRight, CVisFixpoint & fpTop, CVisFixpoint & fpBottom ) const;
	void					GetValidArea( CVisFixpoint & fpLeft, CVisFixpoint & fpRight, CVisFixpoint & fpTop, CVisFixpoint & fpBottom ) const;

	const CVisFixpoint		GetEjectionPosition( const Int nLane ) const;
	const CVisFixpoint		GetConveyorPosition( ) const;

protected:
	CVisProperty			m_propWidth;
	CVisFixpoint			m_fpWidth;

	CVisProperty			m_propHeight;
	CVisFixpoint			m_fpHeight;

	CVisProperty			m_propJetsSpacing;
	CVisFixpoint			m_fpJetsSpacing;

	CVisProperty			m_propNumJets;
	Int						m_nNumJets;

	CVisProperty			m_propObjectsLength;
	CVisFixpoint			m_fpObjectsLength;
	
	CVisProperty			m_propObjectsWidth;
	CVisFixpoint			m_fpObjectsWidth;

	CVisProperty			m_propConveyorPosition;
	CVisFixpoint			m_fpConveyorPosition;
	
	CVisProperty			m_propEjectionPosition_Left;
	CVisFixpoint			m_fpEjectionPosition_Left;

	CVisProperty			m_propEjectionPosition_Right;
	CVisFixpoint			m_fpEjectionPosition_Right;
};


#endif

