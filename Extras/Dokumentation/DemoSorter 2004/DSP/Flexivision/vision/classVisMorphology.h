/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISMORPHOLOGY_H_
#define _CLASSVISMORPHOLOGY_H_

#include "classVisComponent.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"
#include "classVisProperty.h"

/**
* @brief A morphology component.
*
* A morphology component.
*/
class CVisMorphology : public CVisComponent
{
public:
	enum MorphologyOperation
	{
		MORPH_DILATE,
		MORPH_ERODE,
		MORPH_OPEN,
		MORPH_CLOSE,
		MORPH_TOPHAT,
		MORPH_BLACKHAT	
	};
	
							CVisMorphology( const char * strName, CVisPort::PortDataType pdtInputType, MorphologyOperation moOperation);
							~CVisMorphology();
	
	void					Prepare();
	void					DoProcessing();
	
	
protected:

	void					ErodeImg(	Uint8 * restrict dst, const Uint8 * restrict src, 
										const Int iterations,
										const Int bpp,
										const Int cols, const Int rows, 
										const Int srcPitch,  const Int dstPitch );

	void					DilateImg(	Uint8 * restrict dst, const Uint8 * restrict src, 
										const Int iterations,
										const Int bpp,
										const Int cols, const Int rows, 
										const Int srcPitch,  const Int dstPitch );

	void					Difference(	Uint8 * restrict dst, Uint8 * restrict src1, const Uint8 * restrict src2,
										const Int bpp,
										const Int cols, const Int rows, 
										const Int srcPitch,  const Int dstPitch );

	void					ErodeImg_8u(	Uint8 * restrict dst, const Uint8 * restrict src, 											
											const Int cols, const Int rows, 
											const Int srcPitch,  const Int dstPitch );

	void					DilateImg_8u(	Uint8 * restrict dst, const Uint8 * restrict src, 											
											const Int cols, const Int rows, 
											const Int srcPitch,  const Int dstPitch );

	void					Difference_8u(	Uint8 * restrict dst, const Uint8 * restrict src1, const Uint8 * restrict src2, 
											const Int cols, const Int rows, 
											const Int srcPitch,  const Int dstPitch );

	void					ErodeLine_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int srcPitch, const Int numPixels );
	void					DilateLine_8u( Uint8 * restrict dst, const Uint8 * restrict src, const Int srcPitch, const Int numPixels );

	void					DifferenceLine_8u( Uint8 * restrict dst, const Uint8 * restrict src1, const Uint8 * restrict src2,  const Int numPixels );
	

	CVisInputPort			m_iportInput;
	CVisOutputPort			m_oportOutput;

	Uint8*					m_pTempLines;
	
	MorphologyOperation		m_moOperationType;

	Int						m_nNumIterations;
	CVisProperty			m_propNumIterations;
};


#endif
