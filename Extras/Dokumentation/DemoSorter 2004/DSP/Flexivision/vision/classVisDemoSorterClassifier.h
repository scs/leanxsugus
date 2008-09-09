/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISDEMOSORTERCLASSIFIER_H_
#define _CLASSVISDEMOSORTERCLASSIFIER_H_

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
class CVisDemoSorterClassifier : public CVisComponent
{
public:
							CVisDemoSorterClassifier( const Char * strName );
							
	void 					SetCurrentImageTime( Uint32 unTime );
	void					DoProcessing();

	void					SetModel( CVisDemoSorterModel * model );
	void					SetTransform( CVisTransform * transform );
	
	Int						GetNumObjectsFound();
	

protected:
	Bool 					IsObjectValid( const FastLabelObject & obj );
	void 					ResolveCriticalEjections( FastLabelObject * objLabels );
	void 					EjectObject( const FastLabelObject & obj );
	Int 					GetLane( const FastLabelObject & obj, bool bLeft );
	Int						GetOuterLane( const FastLabelObject & obj, Int nSide );
	Int 					GetOtherLane( const CVisFixpoint & pos_x, const Int nMainLane );
	Int						CalculateEjectionTime( const CVisFixpoint & pos_y, const Int nLane );
	Int 					ClassifiyColor( Uint8 unHue, Uint8 unSat, Uint8 unLum );
	
	
	enum ClassifierConsts
	{
		NUM_COLORS = 5,
		
		FLF_EJECT = 0x0001,					// A flag for use with the fastlabel object. It says the object is being ejected.
		FLF_CRITICAL_DONTEJECT = 0x0002		// A flag for use with the fastlabel object. It says the object cannot be ejected due to a critical constellation.
	};
	
	struct ColorClass
	{
		Int		nHue;
		Int		nEject;
		Int		nCount;
	};
	
	CVisInputPort			m_iportLabelData;
	CVisInputPort			m_iportColorData;
	
	CVisProperty			m_propTempDelay_ms;
	Int						m_nTempDelay_ms;
	
	CVisProperty			m_propMargin_ms;
	Int						m_nMargin_ms;
	
	CVisProperty			m_propMinSize;
	Int						m_nMinSize;
	
	CVisProperty			m_propMaxRatio;
	float					m_fMaxRatio;
	
	CVisProperty			m_propSideMargin;
	CVisFixpoint			m_fpSideMargin;
	
	CVisProperty			m_propCriticalDistance_2;
	float					m_fCriticalDistance_2;
		
	CVisProperty			m_propNumObjectsClassified;
	Int						m_nNumObjectsClassified;

	CVisProperty			m_propColor0_Hue;
	CVisProperty			m_propColor0_Eject;
	CVisProperty			m_propColor0_Count;
	
	CVisProperty			m_propColor1_Hue;
	CVisProperty			m_propColor1_Eject;
	CVisProperty			m_propColor1_Count;
	
	CVisProperty			m_propColor2_Hue;
	CVisProperty			m_propColor2_Eject;
	CVisProperty			m_propColor2_Count;
	
	CVisProperty			m_propColor3_Hue;
	CVisProperty			m_propColor3_Eject;
	CVisProperty			m_propColor3_Count;
	
	CVisProperty			m_propColor4_Hue;
	CVisProperty			m_propColor4_Eject;
	CVisProperty			m_propColor4_Count;

	ColorClass				m_aryColorClasses[NUM_COLORS];

	CVisDemoSorterModel *	m_pModel;
	CVisTransform *			m_pTransform;
	
	Uint32					m_unCurrentTime;	
	         
	Int						m_nNumObjectsClassifier_last;
	Int						m_nNumFramesWithoutObjects;
	
	
	
};


#endif

