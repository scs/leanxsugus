/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISCHAINCODE_H_
#define _CLASSVISCHAINCODE_H_



#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"

/**
* The data structure that is used to pass to the next component. Both, a border
* point's coordinates and the direction that was used to reach it are stored in
* here.
*/
struct BorderData
{
	enum BorderDataConsts {
		MAX_POINTS = 2048
	};

	Uint32			unNumPoints;
	Uint16			unPosX[ MAX_POINTS ];
	Uint16			unPosY[ MAX_POINTS ];
	Uint8			unDir[ MAX_POINTS ];
};


/**
* @brief Extracts an object's border boints.
*
* Extracts an object's border boints.
*/
class CVisChaincode : public CVisComponent
{
public:

	enum ChainCodeConsts {
		NUM_DIRECTIONS = 4
	};

							CVisChaincode( const Char * strName );

	void					DoProcessing();


protected:

	/**
	* Finds the start position in the image. At the moment, this function starts at the object's 
	* center of masses (which is in the center of the image, due to the cutter's behaviour) and
	* searches to the north for background pixels. Once a background pixel is found, the last of
	* the foreground pixels is taken as start point.
	*/
	bool					FindStartPosition( const Uint8 * pImage, Int32 & nPosX, Int32 & nPosY );

	/**
	* Performs the actual chain coding on the image. 
	*/
	bool					FindChainCode( const Uint8 * pImage, const Int32 nStartX, const Int32 nStartY, BorderData * pData );
	
	/**
	* A universal move structure that can be used for both, 4 and 8-neighborhood.
	*/
	struct Move
	{
		Int32		nX;
		Int32		nY;
	};

	/**
	* A constant array that maps directions to moves.
	*/
	static const Move		m_aryMoveLUT[NUM_DIRECTIONS];

	Uint32					m_unCurrentInputWidth;
	Uint32					m_unCurrentInputHeight;

	CVisInputPort			m_iportImage;
	CVisInputPort			m_iportPotatoObjects;
	CVisOutputPort			m_oportBorderData;

};


#endif

