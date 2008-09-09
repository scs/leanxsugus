
#include "classVisDemoSorterClassifier.h"

#include "classVisFixpoint.h"


#ifndef _WINDOWS
#include "../classOutputDispatcher.h"
#include "../drvHighResTimer.h"
#endif

#include <math.h>

CVisDemoSorterClassifier::CVisDemoSorterClassifier(const Char * strName)
		:CVisComponent( strName, "ColorClassifier" )
		,m_iportLabelData( "objLabels", CVisPort::PDT_DATA )
		,m_iportColorData( "objColors", CVisPort::PDT_DATA )
		,m_propTempDelay_ms( "TempDelay_ms" )
		,m_propMargin_ms( "Margin_ms" )
		,m_propSideMargin( "SideMargin" )
		,m_propMinSize( "MinSize" )
		,m_propMaxRatio( "MaxRatio" )
		
		,m_propCriticalDistance_2( "CriticalDistance_2" )
		
		,m_propNumObjectsClassified( "NumObjectsClassified" )
		
		,m_propColor0_Hue( "Color0.Hue" )
		,m_propColor0_Eject( "Color0.Eject" )
		,m_propColor0_Count( "Color0.Count" )
		,m_propColor1_Hue( "Color1.Hue" )
		,m_propColor1_Eject( "Color1.Eject" )
		,m_propColor1_Count( "Color1.Count" )
		,m_propColor2_Hue( "Color2.Hue" )
		,m_propColor2_Eject( "Color2.Eject" )
		,m_propColor2_Count( "Color2.Count" )
		,m_propColor3_Hue( "Color3.Hue" )
		,m_propColor3_Eject( "Color3.Eject" )
		,m_propColor3_Count( "Color3.Count" )
		,m_propColor4_Hue( "Color4.Hue" )
		,m_propColor4_Eject( "Color4.Eject" )
		,m_propColor4_Count( "Color4.Count" )
{
	m_iportLabelData.Init( this );
	m_iportColorData.Init( this );
	
	m_propSideMargin.Init( this, &m_fpSideMargin );
	
	m_propTempDelay_ms.Init( this, CVisProperty::PT_INTEGER, &m_nTempDelay_ms );
	m_propMargin_ms.Init( this, CVisProperty::PT_INTEGER, &m_nMargin_ms );
	
	m_propNumObjectsClassified.Init( this, CVisProperty::PT_INTEGER, &m_nNumObjectsClassified );
	
	m_propMinSize.Init( this, CVisProperty::PT_INTEGER, &m_nMinSize );
	m_propMaxRatio.Init( this, CVisProperty::PT_FLOAT, &m_fMaxRatio );
	
	m_propCriticalDistance_2.Init( this, CVisProperty::PT_FLOAT, &m_fCriticalDistance_2 );
	
	m_propColor0_Hue.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[0].nHue));
	m_propColor0_Eject.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[0].nEject));
	m_propColor0_Count.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[0].nCount));
	
	m_propColor1_Hue.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[1].nHue ));
	m_propColor1_Eject.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[1].nEject));
	m_propColor1_Count.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[1].nCount));
	
	m_propColor2_Hue.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[2].nHue ));
	m_propColor2_Eject.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[2].nEject));
	m_propColor2_Count.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[2].nCount));
	
	m_propColor3_Hue.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[3].nHue ));
	m_propColor3_Eject.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[3].nEject));
	m_propColor3_Count.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[3].nCount));
	
	m_propColor4_Hue.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[4].nHue ));
	m_propColor4_Eject.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[4].nEject));
	m_propColor4_Count.Init( this, CVisProperty::PT_INTEGER, &(m_aryColorClasses[4].nCount));
	
	for ( Int i=0; i<NUM_COLORS; i++)
	{
		m_aryColorClasses[i].nHue = i*255/NUM_COLORS;
		m_aryColorClasses[i].nEject = 0;
		m_aryColorClasses[i].nCount = 0;
	}

	m_pModel = NULL;
	
	m_nTempDelay_ms = 0;
	m_nMargin_ms = 100;
	m_fpSideMargin = 0.02;
	
	m_nMinSize = 10;
	m_fMaxRatio = 1.2f;
	m_fCriticalDistance_2 = 0.017f * 0.017f;

	m_nNumObjectsClassified = 0;
	
	m_nNumObjectsClassifier_last = 0;

	m_nNumFramesWithoutObjects = 0;
}
					
// *************************************************************************

void CVisDemoSorterClassifier::DoProcessing()
{
	Uint32 i;
	
	m_nNumObjectsClassifier_last = 0;
	
	if ( m_pModel == NULL )
		return;

	FastLabelObject *	objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	ColorObject *		objColors = (ColorObject*)m_iportColorData.GetBuffer();

	// Get extents of valid area
	CVisFixpoint fLeft, fRight, fTop, fBottom;
	m_pModel->GetValidArea( fLeft, fRight, fTop, fBottom );
	
				
	for ( i=1; i<objLabels[0].unNumObjects; i++)
	{	
		// Clear the object's flags.
		objLabels[i].unFlags = 0;
		
		// See if the object is valid (i.e. it is in a valid position and has sensibel
		// sizes.
		if ( IsObjectValid( objLabels[i] ) )
		{			
			// The object is valid.
			m_nNumObjectsClassified++;
			m_nNumObjectsClassifier_last++;
			m_nNumFramesWithoutObjects = 0;
			
			// Classify the object.
			Int nClass = ClassifiyColor( objColors[i].Color.unHue, objColors[i].Color.unSat, objColors[i].Color.unLum );
			
			// Increment count for that class.
			if ( nClass != -1 )
				m_aryColorClasses[nClass].nCount++;
				
			// Mark the object as due for ejection if the class is valid and scheduled for ejection.
			if ( (nClass != -1) && ( m_aryColorClasses[nClass].nEject != 0 ) )			
				objLabels[i].unFlags |= FLF_EJECT;
			else
				objLabels[i].unFlags &= ~FLF_EJECT;
				
		} // if object valid

	} // for all objects
	
	// Resolve critical ejections
	//ResolveCriticalEjections( objLabels );
	
	// Generate ejection commands
	for ( i=1; i<objLabels[0].unNumObjects; i++)
	{
		if ( (objLabels[i].unFlags & FLF_EJECT) != 0 )
			EjectObject( objLabels[i] );
	}	
	
#ifndef _WINDOWS
	// See if we've seen no objects for a while and turn of the jets if that's the case
	Uint32 unTime = timeGetHighResTime() + timeFromMs( 10 );
	if ( m_nNumFramesWithoutObjects > 25 )
	{	
		for ( int i=0; i<m_pModel->GetNumJets(); i++ )
			COutputDispatcher::Instance()->Channel( i ).AddCommand( unTime , false );
	}
#endif
}

// *************************************************************************

Int CVisDemoSorterClassifier::GetNumObjectsFound()
{
	return m_nNumObjectsClassifier_last;
}
				
// *************************************************************************

Bool CVisDemoSorterClassifier::IsObjectValid( const FastLabelObject & obj )
{
	// Get extents of valid area
	CVisFixpoint fLeft, fRight, fTop, fBottom;
	m_pModel->GetValidArea( fLeft, fRight, fTop, fBottom );
	
	// Check if object is in that area.
	if (	( (obj.pTransformedCoords->fpMx).GetFloatValue() < fLeft.GetFloatValue()  )
		||	( (obj.pTransformedCoords->fpMx).GetFloatValue() > fRight.GetFloatValue() )
		||	( (obj.pTransformedCoords->fpMy).GetFloatValue() < fBottom.GetFloatValue())
		||	( (obj.pTransformedCoords->fpMy).GetFloatValue() > fTop.GetFloatValue()   )  )
		return FALSE;
	/*
	// Get the object's pixel size	
	int nWidth = (obj.unBoundingRight - obj.unBoundingLeft);
	int nHeight = (obj.unBoundingTop - obj.unBoundingBottom);
	
	// See if the pixel size matches
	if ( ( m_nMinSize < 10 ) || (m_nMinSize < 10) )
		return FALSE;
		
	// See if the ratio matches.
	float fRatio = abs((float)nWidth/(float)nHeight);
	if ( fRatio < 1.0f )
		fRatio = 1 / fRatio;
	if ( fRatio > m_fMaxRatio )
		return FALSE;
			*/
	return TRUE;
}
				
// *************************************************************************

void CVisDemoSorterClassifier::ResolveCriticalEjections( FastLabelObject * objLabels )
{
	Uint32 unNumObjects = objLabels[0].unNumObjects;
	
	// For all objects
	for ( Uint32 i=1; i<unNumObjects; i++)
	{
		// Only look at those that need to be ejected
		if ( (objLabels[i].unFlags & FLF_EJECT) != 0 )
		{
			// Compare with all other objects
			for ( Uint32 j=1; j<unNumObjects; j++)	
			{
				// Only look at this object, if it must not be ejected (which also excludes object i,
				// so we don't have to check that).
				if ( (objLabels[j].unFlags & FLF_EJECT) == 0 )
				{
					float fDist_x = (objLabels[i].pTransformedCoords->fpMx.GetFloatValue() - objLabels[j].pTransformedCoords->fpMx.GetFloatValue() );
					float fDist_y = (objLabels[i].pTransformedCoords->fpMy.GetFloatValue() - objLabels[j].pTransformedCoords->fpMy.GetFloatValue() );
				
					float fDist = fDist_x*fDist_x + fDist_y*fDist_y;
				
					// unmark the object for ejection.
					if ( fDist < m_fCriticalDistance_2 )
					{
						objLabels[i].unFlags |= FLF_CRITICAL_DONTEJECT;	
						//LogMsg( "Object removed from ejection due to critical constellation!" );
					}
					
				} // Other objects must not be ejected
				
			} // for all other objects
			
		} // object needs to be ejected.
			
	} // for all objects
	
	
	// Transform the FLF_CRITICAL_DONTEJECT to the real ejection flag
	for ( Uint32 k=1; k<unNumObjects; k++)
	{
		// If the object cannot be ejected, remove the ejection flag.
		if ( (objLabels[k].unFlags & FLF_CRITICAL_DONTEJECT) != 0 )
			objLabels[k].unFlags &= ~FLF_EJECT;
	}
}
				
// *************************************************************************

void CVisDemoSorterClassifier::EjectObject( const FastLabelObject & obj )
{
	// Get the leftmost and rightmost lanes.
	Int nLeftLane = GetOuterLane( obj, -1 );
	Int nRightLane = GetOuterLane( obj, 1 );
	
	for ( Int nLane=nLeftLane; nLane <= nRightLane; nLane++ )
	{
		// Calculate ejection time.
		Uint32 unEjectionTime = CalculateEjectionTime( obj.pTransformedCoords->fpMy, nLane );
	
#ifndef _WINDOWS	
		// Add a command that turns the main jet on and then off.
		COutputDispatcher::Instance()->Channel( nLane ).AddCommand( m_unCurrentTime + unEjectionTime - timeFromMs( m_nMargin_ms ), true );
		COutputDispatcher::Instance()->Channel( nLane ).AddCommand( m_unCurrentTime + unEjectionTime + timeFromMs( m_nMargin_ms ), false );
#endif
	}
	
#ifdef _WINDOWS
	LogMsg("Ejected from lane %d to lane %d", nLeftLane, nRightLane );
#else
	//LogMsg("Ejected from lane %d to lane %d", nLeftLane, nRightLane );
#endif

}
					
// *************************************************************************

Int CVisDemoSorterClassifier::GetOuterLane( const FastLabelObject & obj, Int nSide )
{
	CVisVector vWorld;
	CVisVector vPixel;
	
	// Put pixel coordinates into a vector. We already know y and z.	
	vPixel(2) = obj.unMy;
	vPixel(3) = 0;
	vPixel(4) = 1;
	
	// choose border pixel position in x direction
	if (nSide == -1)
		vPixel(1) = obj.unBoundingLeft;
	else if (nSide == 0)
		vPixel(1) = obj.unMx;
	else if (nSide == 1)
		vPixel(1) = obj.unBoundingRight;
		
	// Transform that point to world coordinates.
	vWorld = m_pTransform->TransformToWorld( vPixel );
	
	// Convert to lane and return
	return m_pModel->MapPosToLane( vWorld(1) );
	
}
					
// *************************************************************************

Int CVisDemoSorterClassifier::GetOtherLane( const CVisFixpoint & pos_x, const Int nMainLane )
{
	Int nOtherLane;
	CVisFixpoint otherPos;
	
	// Seek to the left.
	otherPos.Sub( pos_x, m_fpSideMargin ); 
	nOtherLane = m_pModel->MapPosToLane( otherPos );
	if ( nOtherLane != nMainLane )
		return nOtherLane;
		
	// Seek to the right.
	otherPos.Add( pos_x, m_fpSideMargin ); 
	nOtherLane = m_pModel->MapPosToLane( otherPos );
	if ( nOtherLane != nMainLane )
		return nOtherLane;
		
	// No success
	return -1;
}
				
// *************************************************************************

Int CVisDemoSorterClassifier::CalculateEjectionTime( const CVisFixpoint & pos_y, const Int nLane )
{
	#define g 9.81f

	float fConveyorPosition = m_pModel->GetConveyorPosition().GetFloatValue();
	float fEjectionPosition = m_pModel->GetEjectionPosition( nLane ).GetFloatValue();

	// Calculate the total time an object need to fall from the conveyor to the ejectors
	float fFallT;
	fFallT = sqrtf( (2.0f / g) * ( fConveyorPosition - fEjectionPosition ) );
			
	// Calculate the time the object is already on its way from the conveyor.
	float fDeltaT;
	fDeltaT = sqrtf( (2.0f / g) * fabs( fConveyorPosition - pos_y.GetFloatValue() ) );

	// The ejection time is the total fall time minus the time the object is already airborn.
	float fEjectionTime = fFallT - fDeltaT;

	// Convert the seconds to the high res timing units
	Uint32 unDeltaEjectionTime;
#ifndef _WINDOWS
	unDeltaEjectionTime = timeFromUs((int)(1000000.0f * fEjectionTime) );
#else
	unDeltaEjectionTime = (Uint32)(1000000.0f * fEjectionTime);
#endif

	// Return it.
	return unDeltaEjectionTime;
					
}	
					
// *************************************************************************

Int CVisDemoSorterClassifier::ClassifiyColor( Uint8 unHue, Uint8 unSat, Uint8 unLum )
{
	Int nMinDist = 0xFFFF;
	Int nMinIndex = -1;
	
	for ( Int i=0; i<NUM_COLORS; i++)
	{
		Uint8 dist;
		
		dist = unHue - ((Uint8)m_aryColorClasses[i].nHue);
		if ( dist > 128 )
			dist = 256 - dist;

		if ( dist<nMinDist )
		{
			nMinDist = dist;
			nMinIndex = i;
		}
	}
	
	// Do special classification for orange and read objects!
	// This is sort of a hack and would belong into another kind
	// of classifier. Because it's a hack, we also don't need
	// to parameterize the stuff here.
	if ( (nMinIndex == 1) || (nMinIndex==2) )
	{
		float a=0.16f;
		float b=36.0f;
		
		float t=(float)unHue + a*(float)unSat;
		
		// Red?
		if ( t<b )
			nMinIndex = 2;
		else
			nMinIndex = 1;
	}
		
	
	//LogMsg( "h\t%d\tsat\t%d\tlum\t%d\tclass\t%d", unHue, unSat, unLum );
	LogMsg( "Object classified as class %d", nMinIndex );
	
	return nMinIndex;
}

// *************************************************************************

void CVisDemoSorterClassifier::SetModel( CVisDemoSorterModel * model )
{
	m_pModel = model;
}

// *************************************************************************
		
void CVisDemoSorterClassifier::SetTransform( CVisTransform * transform )
{
	m_pTransform = transform;
}		

// *************************************************************************

void CVisDemoSorterClassifier::SetCurrentImageTime( Uint32 unTime )
{
	m_unCurrentTime = unTime;
}

					
// *************************************************************************
					
// *************************************************************************
