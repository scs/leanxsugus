
#include "classVisDemoSorterModel.h"

CVisDemoSorterModel::CVisDemoSorterModel( const char * strName )
	:CVisComponent( strName, "DemoSorterModel" )
	,m_propWidth( "Width" )
	,m_propHeight( "Height" )
	,m_propJetsSpacing( "JetsSpacing" )
	,m_propNumJets( "NumJets" )
	,m_propObjectsLength( "ObjectsLength" )
	,m_propObjectsWidth( "ObjectsWidth" )
	,m_propConveyorPosition( "ConveyorPosition" )
	,m_propEjectionPosition_Left( "EjectionPosition_Left" )
	,m_propEjectionPosition_Right( "EjectionPosition_Right" )
{
	m_propWidth.Init( this, &m_fpWidth );
	m_propHeight.Init( this, &m_fpHeight );
	m_propJetsSpacing.Init( this, &m_fpJetsSpacing );
	m_propNumJets.Init( this, CVisProperty::PT_INTEGER, &m_nNumJets );
	m_propObjectsLength.Init( this, & m_fpObjectsLength );
	m_propObjectsWidth.Init( this, &m_fpObjectsWidth );	
	m_propConveyorPosition.Init( this, &m_fpConveyorPosition );
	m_propEjectionPosition_Left.Init( this, &m_fpEjectionPosition_Left );
	m_propEjectionPosition_Right.Init( this, &m_fpEjectionPosition_Right );

	m_fpWidth = 0.24f;
	m_fpHeight = 0.7f;
	m_fpJetsSpacing = 0.014f;
	m_nNumJets = 16;
	m_fpObjectsLength = 0.01f;
	m_fpObjectsWidth = 0.01f;
	m_fpConveyorPosition = 0.15f;
	m_fpEjectionPosition_Left = -0.30f;
	m_fpEjectionPosition_Right = -0.30f;
}

// *************************************************************************

void CVisDemoSorterModel::SetWidth( const CVisFixpoint & fpWidth )
{
	m_fpWidth = fpWidth;
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetWidth() const
{
	return m_fpWidth;
}

// *************************************************************************

void CVisDemoSorterModel::SetHeight( const CVisFixpoint & fpHeight )
{
	m_fpHeight = fpHeight;
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetHeight() const
{
	return m_fpHeight;
}

// *************************************************************************

void CVisDemoSorterModel::SetNumJets( Int nNumJets )
{
	m_nNumJets = nNumJets;
}

// *************************************************************************

Int CVisDemoSorterModel::GetNumJets() const
{
	return m_nNumJets;
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetJetPosition( const int nNumJet ) const
{
	CVisFixpoint f;

	// Calculate position of leftmost jet
	f = -m_fpJetsSpacing * (m_nNumJets);
	f >>= 1;

	// Calculate position of desired jet
	CVisFixpoint m;
	m.Mult( m_fpJetsSpacing, nNumJet );
	f += m_fpJetsSpacing * (nNumJet);
	f += (m_fpJetsSpacing >> 1);

	return f;
}

// *************************************************************************

Int CVisDemoSorterModel::MapPosToLane( const CVisFixpoint & fpPosition ) const
{
	CVisFixpoint f;
	CVisFixpoint fLane;
	Int nLane;

	// Calculate position of leftmost jet
	f = -m_fpJetsSpacing * (m_nNumJets);
	f >>= 1;

	// Offset the given position
	f = fpPosition - f;

	// Get the lane
	fLane.Div( f, m_fpJetsSpacing );

	// Round down and convert to integer
	nLane = fLane.GetIntegerValue();

	// Limit the number of lanes and discard wrong results
	if ( nLane > m_nNumJets-1 )
		return m_nNumJets;	
	if ( nLane < 0 )
		return -1;		

	return nLane;
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetObjectsSize() const
{
	return m_fpObjectsLength;
}

// *************************************************************************

void CVisDemoSorterModel::GetTotalArea( CVisFixpoint & fpLeft, CVisFixpoint & fpRight, CVisFixpoint & fpTop, CVisFixpoint & fpBottom ) const
{
	fpRight = m_fpWidth;
	fpTop = m_fpHeight;
	
	fpRight >>= 1;
	fpTop >>= 1;
	
	fpLeft = - fpRight;
	fpBottom = - fpTop;
}

// *************************************************************************

void CVisDemoSorterModel::GetValidArea( CVisFixpoint & fpLeft, CVisFixpoint & fpRight, CVisFixpoint & fpTop, CVisFixpoint & fpBottom ) const
{
	GetTotalArea( fpLeft, fpRight, fpTop, fpBottom );

	CVisFixpoint l = m_fpObjectsLength;
	l >>= 1;
	CVisFixpoint w = m_fpObjectsWidth;
	w >>= 1;	
	
	fpRight.Sub( w );
	fpTop.Sub( l );
	fpLeft.Add( w );
	fpBottom.Add( l );
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetEjectionPosition( const Int nLane ) const
{
	// The distance is depending on whether the lane is on the left or the right
	// valve bank.
	if ( nLane < m_nNumJets/2 )
		return m_fpEjectionPosition_Left;
	else
		return m_fpEjectionPosition_Right;	
}

// *************************************************************************

const CVisFixpoint CVisDemoSorterModel::GetConveyorPosition( ) const
{
	return m_fpConveyorPosition;
}

// *************************************************************************
