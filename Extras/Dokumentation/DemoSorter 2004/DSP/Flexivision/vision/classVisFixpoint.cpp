
#include "classVisFixpoInt.h"


// *************************************************************************

void CVisFixpoint::AdjustFractionBits( const int nFraction )
{
	if ( ( nFraction - m_nFractBits) > 0 )
	{
		m_nValue <<= ( nFraction - m_nFractBits);
	}
	else
	{
		m_nValue >>= ( m_nFractBits - nFraction );
	}

	m_nFractBits = nFraction;
}

// *************************************************************************

void CVisFixpoint::Div( const CVisFixpoint & fpA, const CVisFixpoint & fpDiv )
{
	// This function does a binary division, which may be compared to a normal
	// written division.
	Uint32 remainder;
    Int32  result;
    Uint8  neg_sign;
    Int8   bit;
	Int32  a, div;

	// Get the values	
	a = fpA.m_nValue;
	div = fpDiv.m_nValue;

    result    = 0x00000000l;
    remainder = 0x0000000l;
    neg_sign  = (   ( 0 != (a & 0x80000000))
				  ^ ( 0 != (div & 0x80000000))      );   // calc the resulting sing
    
    if (div < 0) div = -div;
    if (a   < 0) a = -a;   

	// Start with the MSB bit, but leave the sign.
    bit     = 8*sizeof( m_nValue ) - 2;
    
    // First step: ordinary div
    while (bit >= 0)
    {
        remainder |= (a >> bit) & 0x00000001u;
        
        if ( remainder >= (Uint32)div )
        {
            result |= 0x00000001u;
            
            remainder -= div;
        }
        
        result <<= 1;
        remainder <<= 1;
        
        bit--;
    }

	// Now, the result has (a.numFracts - div.numFracts) fractional bits. We now have to determine
	// the missing number of bits up to min(a.numFracts, div.numFracts ), which is the number
	// of fractional bits for the new fixpoint number.
	// First, determine the new fraction bit count:
	m_nFractBits = min( fpA.m_nFractBits, fpDiv.m_nFractBits );

    // second step: divide the remainder
    //bit      = FP_NUM_PART_BITS-1;
	bit = m_nFractBits-(fpA.m_nFractBits - fpDiv.m_nFractBits)-1;

    while ( bit > 0 )
    {
        if ( remainder >= (Uint32)div )
        {
            result |= 0x00000001u;

            remainder -= div;
        }
        
        result <<= 1;
        remainder <<= 1;
        
        bit--;

    }

	// Take away the sign.
    result = abs( result );

	//DEBUG:
//	result <<= m_nFractBits-1;

	// And apply it correctly.
    if (neg_sign != 0)
        result = -result;  
    
	m_nValue = result;

	CHECK_DIV( fpA, fpDiv, *this );
}

// *************************************************************************

void CVisFixpoint::Sqrt( const CVisFixpoint & fp )
{
	Uint32 guess = 0x00008000 ; // the half of the maximum sqare root of a 32 bit number
    Uint32 square;
    Uint32 bitmask;

	// Abort if value is negative
	if ( fp.m_nValue < 0 )
	{
		m_nValue = 0;
		return;
	}
    
	// What we do here is a simple binary search algorithm that starts with a guess,
	// multiplies and compares it with the input number. Then, subsequently, all bits
	// from MSB to LSB are set.
    bitmask = guess;  
    while (bitmask != 0)
    {
		// Add the bit to the guess and compute the square
        guess |= bitmask;
        square = guess * guess;
        
		// Now if this is too high, clear the bit.
        if (square > (Uint32)fp.m_nValue)
            guess &= ~bitmask;   
           
		// next bit, until LSB
        bitmask >>= 1;
    }

	// Now shift the value, since we've lost some fraction bits.
	m_nFractBits = fp.m_nFractBits;
    m_nValue = guess << (fp.m_nFractBits/2);
}

// *************************************************************************

CVisFixpoint CVisFixpoint::Sqrt() const
{
	CVisFixpoint fp;
	fp.Sqrt( *this );
	return fp;
}

// *************************************************************************

Int CVisFixpoint::GetIntegerValue() const
{
	return (m_nValue >> m_nFractBits);
}

// *************************************************************************

float CVisFixpoint::GetFloatValue() const
{
	return ((float)(m_nValue) / (float)(1<<m_nFractBits));
}

// *************************************************************************

Int CVisFixpoint::GetFixedValue() const
{
	return m_nValue;
}

// *************************************************************************
