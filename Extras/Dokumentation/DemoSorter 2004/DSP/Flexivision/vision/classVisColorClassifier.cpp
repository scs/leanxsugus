
#include "classVisColorClassifier.h"


CVisColorClassifier::CVisColorClassifier(const Char * strName)
		:	CVisComponent( strName, "ColorClassifier" ),
			m_iportLabelData( "objLabels", CVisPort::PDT_DATA ),
			m_iportObjectData( "objPotatoes", CVisPort::PDT_DATA )
{
	m_iportLabelData.Init( this );
	m_iportObjectData.Init( this );

}
					
// *************************************************************************

void CVisColorClassifier::DoProcessing()
{
	FastLabelObject *				objLabels = (FastLabelObject*)m_iportLabelData.GetBuffer();
	PotatoList *					pList = (PotatoList*)m_iportObjectData.GetBuffer();

	Uint32 obj_index;
	Uint32 label_index;
	Uint32 cur_frame;

	obj_index = pList->unCurrentPotatoId;
	label_index = pList->pObjects[obj_index].unCurrentLabel;
	cur_frame = pList->pObjects[obj_index].unCurrentImageNum;

	// calculate pixel class ratios
	Uint32 unBad = (Uint32)(objLabels[label_index].pHistogram[2]) 
			* (1<<PotatoObject::FP_FRACTIONAL_BITS) 
			/ objLabels[label_index].unArea;

	Uint32 unGreen = (Uint32)(objLabels[label_index].pHistogram[3])
			* (1<<PotatoObject::FP_FRACTIONAL_BITS)  
			/ objLabels[label_index].unArea;

	pList->pObjects[obj_index].unpClassificationColor[cur_frame] = unBad;
	pList->pObjects[obj_index].unpClassificationGreen[cur_frame] = unGreen;
}
					
// *************************************************************************
					
// *************************************************************************
					
// *************************************************************************
					
// *************************************************************************
					
// *************************************************************************
