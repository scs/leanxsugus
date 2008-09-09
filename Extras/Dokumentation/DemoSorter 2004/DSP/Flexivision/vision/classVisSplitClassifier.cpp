
#include "classVisSplitClassifier.h"

CVisSplitClassifier::CVisSplitClassifier( const Char * strName )
	:	CVisComponent( strName, "SplitClassifier"),
		m_iportLabels( "objLabels", CVisPort::PDT_DATA ),
		m_iportObjectsList( "objPotatoes", CVisPort::PDT_DATA ),
		m_oportSplitList( "objSplits", CVisPort::PDT_DATA ),
		m_propMinArea( "MinArea" ),
		m_propMinLength( "MinLength" ),
		m_propMinRatio( "MinRatio" ),
		m_propBoundingMargin( "BoundingMargin" )
{
	m_iportLabels.Init( this );
	m_iportObjectsList.Init( this );
	m_oportSplitList.Init( this );

	m_oportSplitList.SetBufferSize( sizeof( SplitList ) );

	m_nMinArea = 30;
	m_propMinArea.Init( this, CVisProperty::PT_INTEGER, &m_nMinArea );

	m_nMinLength = 15;
	m_propMinLength.Init( this, CVisProperty::PT_INTEGER, &m_nMinLength );

	m_fpMinRatio = F2FP( 2.5, 16 );
	m_propMinRatio.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpMinRatio, 16 );

	m_nBoundingMargin = 3;
	m_propBoundingMargin.Init( this, CVisProperty::PT_INTEGER, &m_nBoundingMargin );
}
	
// *************************************************************************

CVisSplitClassifier::~CVisSplitClassifier()
{
}
	
// *************************************************************************

void CVisSplitClassifier::DoProcessing()
{
	LabelObject *			pLabelObjects = (LabelObject*)m_iportLabels.GetBuffer();
	PotatoList *				pList = (PotatoList *) m_iportObjectsList.GetBuffer();
	SplitList *					pSplitList = (SplitList *) m_oportSplitList.GetBuffer();

	// Get the current potato
	Uint32 unNumSplits;
	Uint32 unCurrentPotato;
	Uint32 unCurrentFrame;
	

	unCurrentPotato = pList->unCurrentPotatoId;
	unCurrentFrame = pList->pObjects[unCurrentPotato].unCurrentImageNum;
	
	// Get the bounding box. This coordinates are still in the total frame's coordinate system
	Uint32 left, top, right, bottom;
	left = pList->pObjects[unCurrentPotato].unLocalBoundingLeft;
	top = pList->pObjects[unCurrentPotato].unLocalBoundingTop;
	right = pList->pObjects[unCurrentPotato].unLocalBoundingRight;
	bottom = pList->pObjects[unCurrentPotato].unLocalBoundingBottom;
	
	// Get the number of splits in this potato
	unNumSplits = ClassifyLabelObjects( pLabelObjects, pSplitList, 
										left, top, right, bottom );
	
	// And store the result in the global object table.
	pList->pObjects[unCurrentPotato].unpClassificationSplit[unCurrentFrame] = unNumSplits;
	
}
	
// *************************************************************************

Int CVisSplitClassifier::ClassifyLabelObjects(	LabelObject * restrict pLabelObjects, SplitList * restrict pSplitList, 
												const Uint32 unBoundingLeft, const Uint32 unBoundingTop, 
												const Uint32 unBoundingRight, const Uint32 unBoundingBottom)
{
	Int unNumSplits = 0;

	// Reset splitlist
	pSplitList->unNumEntries = 0;

	for ( Uint32 i=1; i<pLabelObjects[0].unNumObjects; i++)
	{
		LabelObject * obj = &(pLabelObjects[i]);

		Int nMajAxis, nMinAxis;
		Int fpRatio;
		
		// Splits need a certain area to be taken serious.
		if ( obj->unArea > (unsigned)m_nMinArea )
		{
			// They also need to be within the specified bounding box of the object. We allow a certain amount
			// of margin (should be at least the number of erosions used in the PPU, since the bounding box will
			// be smaller than the real grayscale object by that amount).
			if (	( obj->unBoundingLeft + m_nBoundingMargin >= unBoundingLeft )
				&&	( obj->unBoundingTop + m_nBoundingMargin >= unBoundingTop )
				&&	( obj->unBoundingRight <= unBoundingRight + m_nBoundingMargin )
				&&	( obj->unBoundingBottom <= unBoundingBottom + m_nBoundingMargin ) )
			{

				// Calculate the axes of the ellipsoid approximation and their ratio
				obj->CalcAxes( nMajAxis, nMinAxis );
				fpRatio = (nMajAxis << 16) / nMinAxis;

				// Now see if the length and the ratio mathc the requirements for a split
				if ( (fpRatio > m_fpMinRatio) && (nMajAxis > m_nMinLength) )
				{
					// Increment our counter for that frame
					unNumSplits++;

					// Store the split information for the visualizer if there is still room in
					// the list
					if ( pSplitList->unNumEntries < SplitList::MAX_ENTRIES )
					{
						Uint32 i = pSplitList->unNumEntries;

						pSplitList->arySplitInfos[i].unBoundingLeft = obj->unBoundingLeft;
						pSplitList->arySplitInfos[i].unBoundingRight = obj->unBoundingRight;
						pSplitList->arySplitInfos[i].unBoundingTop = obj->unBoundingTop;
						pSplitList->arySplitInfos[i].unBoundingBottom = obj->unBoundingBottom;
						
						pSplitList->unNumEntries++;
					}

				} // if ratio

			} // if in bounding box

		} // if area satisfying
	}

	return unNumSplits;
}
	
// *************************************************************************
