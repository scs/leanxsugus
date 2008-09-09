
#include "classVisFourierDescriptor.h"

#include "classVisBufferManager.h"

extern "C"
{
#include "DSP_fft16x16t.h"
}

#include <math.h>


#ifndef PI
# define PI (3.14159265358979323846)
#endif



// *************************************************************************

CVisFourierDescriptor::CVisFourierDescriptor( const Char * strName )
:	CVisComponent( strName, "FourierDescr" ),
	m_iportBorderData( "dataBorder", CVisPort::PDT_DATA ),
	m_oportFourierData( "dataFourier", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST )
{
	m_iportBorderData.Init( this );
	m_oportFourierData.Init( this );

	// Generate the twiddle factors for the fft
	GenerateTwiddle( m_aryTwiddleFactors, NUM_BORDERPOINTS );

	// Set the fourier coefficients output size, which isn't affected by any input coefficients.
	m_oportFourierData.SetBufferSize( NUM_BORDERPOINTS * sizeof( Uint16 ) );

	// Acquire memory for the complex samples and the fft coefficients
	CVisBufferManager::Instance()->RequestBuffer( this, 2 * NUM_BORDERPOINTS * sizeof( Uint16 ), (Ptr*)&m_pSamples, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, 2 * NUM_BORDERPOINTS * sizeof( Uint16 ), (Ptr*)&m_pComplexCoefficients, CVisBufferManager::BUF_FAST );

}

// *************************************************************************

void CVisFourierDescriptor::DoProcessing()
{
	BorderData *			border			= (BorderData*)m_iportBorderData.GetBuffer();
	Uint16 *				coefficients	= (Uint16*)m_oportFourierData.GetBuffer();

#ifdef _WRITETOFILE
	static bool bNew = true;
#endif

	// Pick NUM_BORDERPOINTS points out of the border.
	if ( ! PickSamples( border, m_pSamples ) )
		return;

#ifdef _WRITETOFILE
	WriteIntValuesToFile( m_pSamples, NUM_BORDERPOINTS, true, "_cc.m", "cc", bNew ); 
#endif

	// do the FFT
	DSP_fft16x16t( m_aryTwiddleFactors, NUM_BORDERPOINTS, m_pSamples, m_pComplexCoefficients );

#ifdef _WRITETOFILE
	WriteIntValuesToFile( m_pComplexCoefficients, NUM_BORDERPOINTS, true, "_fd.m", "fd", bNew ); 
#endif
		
	// Make translation invariant
	MakeTranslationInvariant( m_pComplexCoefficients );

	// Calculate the absolute values
	MakeAbsolute( m_pComplexCoefficients, coefficients );

	// ... and make scale invariant by scaling all coefficients by c1's size.
	MakeScaleInvariant( coefficients );

#ifdef _WRITETOFILE
	bNew = false;
#endif

}

// *************************************************************************

bool CVisFourierDescriptor::PickSamples( const BorderData * borderPoints, Int16 * complexPoints )
{
	Int32 totalPoints = (signed)(borderPoints->unNumPoints);
	Int32 index = 0;


	if ( totalPoints < NUM_BORDERPOINTS )
		return false;

	for ( Int16 i=0; i<NUM_BORDERPOINTS; i++ )
	{
		Uint32 p = (i * totalPoints ) / NUM_BORDERPOINTS;
		
		// Assemble the complex array. Blow up the values so that the full FFT scale can be
		// used.
		complexPoints[ index*2 ] = borderPoints->unPosX[p] << SAMPLES_SHIFT_FACTOR;
		complexPoints[ index*2 + 1 ] = borderPoints->unPosY[p] << SAMPLES_SHIFT_FACTOR;

		index++;
	}

	return true;
}

// *************************************************************************

void CVisFourierDescriptor::MakeTranslationInvariant( Int16 * complexPoints )
{
	complexPoints[0] = 0;
	complexPoints[1] = 0;
}

// *************************************************************************

void CVisFourierDescriptor::MakeAbsolute( const Int16 * complexPoints, Uint16 * absoluteValues )
{
	Int32 sum;

	for ( Uint32 i=0; i<NUM_BORDERPOINTS; i++)
	{
		sum = (Int32)complexPoints[i*2] * (Int32)complexPoints[i*2] 
				+ (Int32)complexPoints[i*2+1] * (Int32)complexPoints[i*2+1];

		ASSERT( sum >= 0);

		absoluteValues[i] = isqrt((Uint32)sum);

	}
}

// *************************************************************************

bool CVisFourierDescriptor::MakeScaleInvariant( Uint16 * absoluteValues )
{
	Uint16 scale;
	Uint32 factor;

#define PRECISION_BITS (16+14)
#define FRACTIONAL_BITS COEFF_FRACTIONAL_BITS

	// Scale by the first coefficient
	scale = absoluteValues[1];

	// Invert the scale so that we'll only have to multiply, not divide.
	// We use a 32 bit number to not loose any precision.

	// scale is a Q.COEFF_FRACTIONAL_BITS fixed point. We devide a Q.PRECISION_BITS fixed point 1.0, 
	// by scale, which leaves us with a Q.(PRECISION_BITS-COEFF_FRACTIONAL_BITS) number.
	factor = (1<<PRECISION_BITS) / scale;

	for ( Uint32 i=1; i<NUM_BORDERPOINTS; i++)
	{
		Uint32 tmp;

		// The Q.(PRECISION_BITS-COEFF_FRACTIONAL_BITS) number is multiplied with a Q.(COEFF_FRACTIONAL_BITS),
		// leaving a Q.PRECISION_BITS number again, in the range of 0..1, since c1 is the biggest coefficient.
		tmp = (Uint32)absoluteValues[i] * factor;

		// Shift the number back the amount of precision bits, but only until
		// we get the previous fixed point representation.
		tmp >>= ( PRECISION_BITS - COEFF_FRACTIONAL_BITS );

		absoluteValues[i] = (Uint16)tmp;
	}		

	return true;
}

// *************************************************************************

// *************************************************************************

// *************************************************************************

Int16 CVisFourierDescriptor::double2int16(double d)
{
    d = floor(0.5 + d);  // Explicit rounding to integer //
	
    if (d >=  32767.0) 
		return  32767;
	
    if (d <= -32768.0) 
		return -32768;
	
    return (Int16)d;
}

// *************************************************************************

void CVisFourierDescriptor::GenerateTwiddle(short *w, int n)
{
	double M = 32767.5;
	int i, j, k;
	
	for (j = 1, k = 0; j < n >> 2; j = j << 2)
	{
		for (i = 0; i < n >> 2; i += j << 1)
		{
			w[k + 11] = double2int16(M * cos(6.0 * PI * (i + j) / n));
			w[k + 10] = double2int16(M * sin(6.0 * PI * (i + j) / n));
			w[k + 9] = double2int16(M * cos(6.0 * PI * (i ) / n));
			w[k + 8] = double2int16(M * sin(6.0 * PI * (i ) / n));
			w[k + 7] = double2int16(M * cos(4.0 * PI * (i + j) / n));
			w[k + 6] = double2int16(M * sin(4.0 * PI * (i + j) / n));
			w[k + 5] = double2int16(M * cos(4.0 * PI * (i ) / n));
			w[k + 4] = double2int16(M * sin(4.0 * PI * (i ) / n));
			w[k + 3] = double2int16(M * cos(2.0 * PI * (i + j) / n));
			w[k + 2] = double2int16(M * sin(2.0 * PI * (i + j) / n));
			w[k + 1] = double2int16(M * cos(2.0 * PI * (i ) / n));
			w[k + 0] = double2int16(M * sin(2.0 * PI * (i ) / n));
			k += 12;
		}
	}
	w[2*n - 1] = w[2*n - 3] = w[2*n - 5] = 32767;
	w[2*n - 2] = w[2*n - 4] = w[2*n - 6] = 0;
}

// *************************************************************************

Uint16 CVisFourierDescriptor::isqrt(Uint32 a)
{
  Uint32 rem = 0;
  Uint32 root = 0;
  Uint32 divisor = 0;
  for(Uint32 i=0; i<16; i++)
  {
    root <<= 1;
    rem = ((rem << 2) + (a >> 30));
    a <<= 2;
    divisor = (root<<1) + 1;

    if(divisor <= rem)
	{
      rem -= divisor;
      root++;
    }
  }
  return (Uint16)(root);
}

// *************************************************************************

// *************************************************************************

// *************************************************************************
