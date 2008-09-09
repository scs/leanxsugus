/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISFOURIERDESCRIPTOR_H_
#define _CLASSVISFOURIERDESCRIPTOR_H_		

#include "classVisComponent.h"
#include "classVisProperty.h"
#include "classVisInputPort.h"
#include "classVisOutputPort.h"
#include "classVisChaincode.h"


/**
* @brief The fourier descriptor creating component.
*
* This component creates fourier descriptors out of an object's chain code information.
*/
class CVisFourierDescriptor : public CVisComponent
{
public:
	

						CVisFourierDescriptor( const Char * strName );

	void				DoProcessing();

	enum FourierDescriptorConsts {
		/**
		* Number of points used for the FFT. Due to the nature of the FFT algorithm applied (Ti DSP lib),
		* this must be a power of 4!.
		*/
		/*NUM_POINTS = 64,				*/
		NUM_BORDERPOINTS = 64,			

		/**
		* The number of bits the values are shifted to the left before feeding them to the 16 bit FFT, so that 
		* the full 16 bit scale is used. Numbers are expected to have up to 9 valid bits (Which we conclude
		* from the fact that those values are coordinates in an image of the approximate size of 120x80; plus
		* the sign bit) and thus may be shifted up to 6 bits to the left.
		*/
		SAMPLES_SHIFT_FACTOR = 2,		

		/**
		* The number of partial bits of the absolute FFT values (fixed point). This value is relatively hard
		* to compute since many factors have to be taken into consideration:
		*	-	We'd like as much precision as possible -> as high a value as possible
		*	-	We won't take all the fft coeffs into classification consideration; we're more interested in
		*		the bigger ones. -> Maybe we won't need that much precision for small numbers (below 1.0).
		*	-	All values are scaled to c1, which is, in most cases, the biggest coefficient. Thus, we won't
		*		need much range above 1.0. Though, it might well be that certain values exceed c1 by 10 times...
		*	-	The accumulators used in the fourier classifier are only 32 bits wide and should at least be
		*		capable of acquiring 30 minutes of full calibration.
		*		30 mins * 15 fps * ~30 Potatoes = ~ 2^19
		*		So, we need at least 19 bits for the accumulation, which leaves the  coefficients with 13 bits.
		*
		* Given the fact that most values are smaller than c1 most of the time (at least their mean value),
		* and that c1 is always 1 and therefore the biggest value to handle (in the mean), we can choose the
		* number so that 1.0 may be added 2^19 times by the accumulator. That leads us to 1.0 being representated
		* as (1<<13) and thus to a fixed point format with 12 partial bits. Also, we don't have to bother for
		* single coefficients that surpass c1 by as much as 16 times.
		*
		* Note: the variance accumulator is expected to be even less the problem since merely the squares of the
		* coeffs are summed up, which, in most cases, are even smaller than the coefficients theirselves ( c<1 ).
		*/
		COEFF_FRACTIONAL_BITS = 12
	};

protected:
	/**
	* Takes NUM_BORDERPOINTS evenly distributed points out of the chaincode array, which can then be fed
	* into the fft.
	*/
	bool				PickSamples( const BorderData * borderPoints, Int16 * complexPoints );

	void				MakeTranslationInvariant( Int16 * complexPoints );
	void				MakeAbsolute( const Int16 * complexPoints, Uint16 * absoluteValues );

	/**
	* Makes the fourier coefficients scale invarient, which means that they all get normalized to 
	* fourier coefficient 1. The function takes the absolute values of the fourier coefficients.
	* The resulting numbers are fixed point, 14 bits base. Their values are expected to be from
	* 0..1 (c1 is the biggest coefficient most of the time), but may go up to 4.
	*/
	bool				MakeScaleInvariant( Uint16 * absoluteValues );


	/** GenerateTwiddle helper function. */
	Int16				double2int16(double d);

	/**
	* Generates the twiddle coefficients for the FFT.
	* TODO: this could be done off-line, so that no math.h is required.
	*/
	void				GenerateTwiddle(short *w, int n);

	/**
	* An integer square root function.
	*/
	Uint16				isqrt(Uint32 a);

	/** The twiddle factor array. */
	Int16				m_aryTwiddleFactors[ 2 * NUM_BORDERPOINTS ];	

	Int16 *				m_pComplexCoefficients;
	Int16 *				m_pSamples;

	CVisInputPort		m_iportBorderData;
	CVisOutputPort		m_oportFourierData;
};


#endif

