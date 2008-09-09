
#ifndef _KEYPOINTSCUT_H_
#define _KEYPOINTSCUT_H_

#include "classVisComponent.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"
#include "classVisProperty.h"

class CVisKeypointsCut : public CVisComponent
{
	/** 
	* This structure is used to store a keypoint's data.
	*/
	typedef struct 
	{
		Int x; 			// x coordinate of keypoint
		Int y; 			// y coordinate of keypoint
		float mx; 		// x coordinate of center of mass of background
		float my; 		// y coordinate of center of mass of background
		int strength;
	} Keypoint;

public:
							CVisKeypointsCut( const char * strName, Int nMaxNumKeys, Uint32 unFlags );
							~CVisKeypointsCut();
							
	/**
	* Processes the image. The processing step finds key points in the image and tries to connect them.
	* If the connection is possible, it is drawn by a background colored-line to separate adjacent objects.
	* Furthermore, the processing does a thresholding of the image, so that all background pixels equal 0
	* afterwards.
	*/
	bool					DoProcessing();

	/**
	* The flags that may be specified with the constructor.
	*/
	enum KeypointsCutFlags
	{
		OUTPUT_ON_INPUTPORT = 1
	};

protected:
	void 					FindKeypoints(Uint8 * pImgIn, float fRatioThresh, Int windowSize );
	void 					ResetKeys();
	int 					EnterKey(Int strength, Int x, Int y, float mx, float my, Int radius);

	void 					CutObjects( Uint8 * pImgOut );
	float 					KeyDistance( const Keypoint *k1, const Keypoint *k2 );
	void					DrawKeyLine( Uint8 * pImgOut, Int x1, Int y1, Int x2, Int y2);

protected:
	enum eCutConsts{		BACKGROUND = 0 };

	CVisInputPort			m_iportInput;
	CVisOutputPort			m_oportOutput;

	Uint32					m_unFlags;
	
	Keypoint				* m_pKeypoints;
	Int						m_nMaxNumKeys;
	Int						m_nNumKeys;
	
	CVisProperty			m_propWindowSize;
	Int						m_nWindowSize;

	CVisProperty			m_propRatioThreshold;
	float					m_fRatioThreshold;

	CVisProperty			m_propMaxMatch;
	Uint32					m_nMaxMatch;	

	Int 					m_nProfile1;
	Int 					m_nProfile2;
	Int 					m_nProfile3;
};

#endif





















