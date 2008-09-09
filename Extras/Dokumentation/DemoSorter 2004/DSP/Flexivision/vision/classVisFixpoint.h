/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISFIXPOINT_H_
#define _CLASSVISFIXPOINT_H_

#include "classVisObject.h"

/**
* A high presicion 32x32 integer "multiplication and right shift" function.
*/
inline Int MultShift( Int32 a, Int32 b, Int shift )
{
	Uint32 A, B;
	Uint32 P1,P2,P3,P4;
	Uint32 unSign;
	Int nRes;

	A = (a<0)?-a:a;
	B = (b<0)?-b:b;

	unSign = (a ^ b) >> 31;

	// Calculate products of al,ah,bl and bh.
	P1 = (A & 0x0000FFFFu) * (B & 0x0000FFFFu);	
	P2 = (A & 0x0000FFFF) * (B>>16);
	P3 = (A>>16) * (B & 0x0000FFFF);
	P4 = (A>>16) * (B>>16);

	// Sum up the products and shift them.
	nRes = (Int)(	(P1>>(shift))
					+ (P2>>(shift-16))
					+ (P3>>(shift-16))
					+ (P4<<(32-shift)) ); 

	// Apply sign.
	if (unSign)
		nRes = -nRes;

#ifdef _WINDOWS
	Int trueres = (Int)(((__int64)a * (__int64)b) >> shift);

	// We'll have an error of two LSBs because of the 3 additions.
	ASSERT( 3 >= abs(nRes - trueres) );
			
#endif

	return nRes;
}


/**
* Define some macros that are used to check the fixed point operations. They may be 
* enabled and disabled using the following define.
*/
#ifdef _DEBUG
#define CHECK_FPOP
#endif

#define CHECK_THRESHOLD	0.0001

#ifndef CHECK_FPOP

// If the checks are disabled, define the macros blank.
#define CHECK_ADD( fpa, fpb, fpres )
#define CHECK_SUB( fpa, fpb, fpres  )
#define CHECK_MULT( fpa, fpb, fpres  )
#define CHECK_DIV( fpa, fpb, fpres )
#define CHECK_ASSIGN( fpa, fpres )

#else

#include <math.h>

#define CHECK_ADD( fpa, fpb, fpres )		fpCheckAdd( (fpa).GetFloatValue(), (fpb).GetFloatValue(), (fpres).GetFloatValue() )
#define CHECK_SUB( fpa, fpb, fpres  )		fpCheckSub( (fpa).GetFloatValue(), (fpb).GetFloatValue(), (fpres).GetFloatValue() )
#define CHECK_MULT( fpa, fpb, fpres  )		fpCheckMult( (fpa).GetFloatValue(), (fpb).GetFloatValue(), (fpres).GetFloatValue() )
#define CHECK_DIV( fpa, fpb, fpres )		fpCheckDiv( (fpa).GetFloatValue(), (fpb).GetFloatValue(), (fpres).GetFloatValue() )
#define CHECK_ASSIGN( fpa, fpres )			fpCheckAssign( (fpa).GetFloatValue(), (fpres).GetFloatValue() )


inline void	fpCheckAdd( float a, float b, float res )
{
	if ( fabs(a + b - res) > CHECK_THRESHOLD )
		TRACE("Fixpoint test failed: %f + %f != %f\n", a,b,res );
}

inline void	fpCheckSub( float a, float b, float res )
{
	if ( fabs(a - b - res) > CHECK_THRESHOLD )
		TRACE("Fixpoint test failed: %f - %f != %f\n", a,b,res );
}

inline void	fpCheckMult( float a, float b, float res )
{
	if ( fabs(a * b - res) > CHECK_THRESHOLD )
		TRACE("Fixpoint test failed: %f * %f != %f\n", a,b,res );
}

inline void	fpCheckDiv( float a, float b, float res )
{
	if ( fabs(a / b - res) > CHECK_THRESHOLD )
		TRACE("Fixpoint test failed: %f / %f != %f\n", a,b,res );
}

inline void	fpCheckAssign( float a, float res )
{
	if ( fabs(a - res) > CHECK_THRESHOLD )
		TRACE("Fixpoint test failed: %f != %f\n", a,res );
}
#endif



/**
* @brief A fixedpoint wrapper class.
*
* A fixedpoint wrapper class. Due to the "relatively" high memory consumption of
* a CVisObject (~20 Bytes), this class is not derived from it. This should be
* changed sometimes (i.e. the CVisObject class should reduce its memory footprint
* and move some of its current functionality to a subclass).
*
* Also, by not deriving from CVisObject, we get faster (and inline!) construction
* and destruction. 
*
* Inlining is crucial for fast fixedpoint arithmetics on a DSP. Only with inlined
* function the compiler is able to do calculations in thight loops and highly 
* optimize them.
*
* Also important: The number of fractional bits always has to be a multiple of two!
*/
class CVisFixpoint
{
public:
	/** Construct a fixed point object. */
	inline					CVisFixpoint()												{ m_nValue = 0;				m_nFractBits = FP_DEFAULT_FRACTBITS; }

	/** Constructs and copies a fixed point object from a given object. */
	inline					CVisFixpoint( const CVisFixpoint & fp )						{ m_nValue = fp.m_nValue;	m_nFractBits = fp.m_nFractBits; }

	/** Constructs a fixedpoint object specifying the number of fractional bits. */
	explicit inline			CVisFixpoint( const int & fractBits )						{ m_nValue = 0;				m_nFractBits = fractBits; }

	/** Construct the fixedpoint object specifying the number of fractional bits
	 *  and a value (which is stored in the specified fixedpoint format). */
	inline					CVisFixpoint( const int & nVal, const int & fractBits )		{ m_nFractBits = fractBits;	m_nValue = nVal; }

	/** Constructs the fixedpoint object and assigns a float value to it. */
	inline					CVisFixpoint( const float & fVal, const int & fractBits )	{ m_nFractBits = fractBits;	*(this) = fVal; }
	inline					CVisFixpoint( const double & fVal, const int & fractBits )	{ m_nFractBits = fractBits;	*(this) = fVal; }
	
	inline void				Init()														{ m_nValue = 0;				m_nFractBits = FP_DEFAULT_FRACTBITS; }

	/**
	* Initializes the fixpoint's fraction bit count to a certain number. This does NOT
	* convert the value in any case (which is presumed to be lost after this); use
	* AdjustFractionBits if you want to preserve the value. InitFractionBits() is
	* used to initially set the fraction bit of a fispoint number, when you can't
	* do that at the constructing time (e.g. in arrays).
	*/
	inline void				InitFractionBits( const int nFraction )						{ m_nFractBits = nFraction; }

	/**
	* Adjusts the number of fraction bits while preserving the fixpoint's
	* value. Depending on the current value, some of the precision may be
	* lost or the number may wrap around.
	*/
	void					AdjustFractionBits( const int nFraction );

	/**
	* Assign another fixedpoint's value to this, keeping the number of fraction bits of this
	* fixedpoint.
	*/
	inline void				AssignKeepFractionBits( const CVisFixpoint fp )				{ if ( m_nFractBits > fp.m_nFractBits ) 
																							{
																								m_nValue = fp.m_nValue<<(m_nFractBits-fp.m_nFractBits);
																							} else if ( m_nFractBits < fp.m_nFractBits )
																							{
																								m_nValue = fp.m_nValue>>(fp.m_nFractBits-m_nFractBits); 
																							} else 
																							{
																								m_nValue = fp.m_nValue;																								
																							}
																						}

	/**
	* Negates this number.
	*
	* this = -this
	*/
	inline void				Neg()														{ m_nValue = - m_nValue; };

	/**
	* Negates a copy of the given number.
	*/
	inline void				Neg( const CVisFixpoint & a )								{ m_nValue = -a.m_nValue; m_nFractBits = a.m_nFractBits; }

	/**
	* Adds two fixedpoint numbers and stores the result in this object. The resulting
	* number receives the smaller number of fraction bits of the two addends.
	*
	* this = a + b
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Add( const CVisFixpoint & a, const CVisFixpoint & b )
																						{ 
																						  m_nFractBits = min(a.m_nFractBits, b.m_nFractBits);
																						  m_nValue = (a.m_nValue>>(a.m_nFractBits - m_nFractBits))
																									+(b.m_nValue>>(b.m_nFractBits - m_nFractBits)); 
																						  CHECK_ADD( a,b, *this ); }

	/**
	* Adds another fixpoint number to this one. Again, the number of fraction bits is
	* reduced if required.
	*
	* this = this + a
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Add( const CVisFixpoint & a )
																						{ 
																						  Int nNewFractBits = min(a.m_nFractBits, m_nFractBits);
																						  m_nValue = (a.m_nValue>>(a.m_nFractBits - nNewFractBits))
																									+(  m_nValue>>(  m_nFractBits - nNewFractBits));
																						  m_nFractBits = nNewFractBits; 
																						  }
	/** addition of an integer. */
	inline void				Add( const int a )											{	m_nValue += (a<<m_nFractBits); }
	inline void				Add( const CVisFixpoint & fp, int a )						{	m_nValue = fp.m_nValue + (a<<fp.m_nFractBits); m_nFractBits = fp.m_nFractBits; }
	inline void				Add( const unsigned int a )									{	m_nValue += (a<<m_nFractBits); }
	inline void				Add( const CVisFixpoint & fp, unsigned int a )				{	m_nValue = fp.m_nValue + (a<<fp.m_nFractBits); m_nFractBits = fp.m_nFractBits; }

	/**
	* Subtracts two fixedpoint numbers and stores the result in this object.
	* The number of fraction bits of the resulting number is equal to the lower fraction bit
	* count of the two numbers involved.
	*
	* this = a-b
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Sub( const CVisFixpoint & a, const CVisFixpoint & b )
																						{ 
																						  m_nFractBits = min(a.m_nFractBits, b.m_nFractBits);
																						  m_nValue = (a.m_nValue>>(a.m_nFractBits - m_nFractBits))
																									-(b.m_nValue>>(b.m_nFractBits - m_nFractBits)); 
																						  CHECK_SUB( a,b, *this ); }

	/**
	* Subtracts another fixpoint number from this one. The number of fraction bits is
	* reduced if necessary.
	*
	* this = this-b
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Sub( const CVisFixpoint & b )
																						{ 
																						  Int nNewFractBits = min(m_nFractBits, b.m_nFractBits);																						  
																						  m_nValue = (  m_nValue>>(  m_nFractBits - nNewFractBits))
																									-(b.m_nValue>>(b.m_nFractBits - nNewFractBits)); }

	/** Sobtraction of an integer. */
	inline void				Sub( const int a )											{	m_nValue -= (a<<m_nFractBits); }
	inline void				Sub( const CVisFixpoint & fp, int a )						{	m_nValue = fp.m_nValue - (a<<fp.m_nFractBits); m_nFractBits = fp.m_nFractBits; }
	inline void				Sub( const unsigned int a )									{	m_nValue -= (a<<m_nFractBits); }
	inline void				Sub( const CVisFixpoint & fp, unsigned int a )				{	m_nValue = fp.m_nValue - (a<<fp.m_nFractBits); m_nFractBits = fp.m_nFractBits; }

	/**
	* Multiplies two fixedpoint numbers and stores the result in this object. The resulting number's
	* number of fraction bits is equale to the average of the two multplicands.
	*
	* this = a * b
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Mult( const CVisFixpoint & a, const CVisFixpoint & b )
																						{ 
																							m_nFractBits	= (a.m_nFractBits/2) + (b.m_nFractBits/2);
																							m_nValue		= MultShift( a.m_nValue, b.m_nValue, m_nFractBits );
																							CHECK_MULT( a,b, *this ); }

	/**
	* Multiplies this fixedpoint number with another and stores the result in this object. The resulting number's
	* number of fraction bits is equale to the average of the two multplicands.
	*
	* this = this * a 
	*
	* Note: This is the high-speed interface that should be used instead of the operators
	*		when speed is critical.
	*/
	inline void				Mult( const CVisFixpoint & a )
																						{ 
																							m_nFractBits	= (a.m_nFractBits/2) + (m_nFractBits/2);
																							m_nValue		= MultShift( a.m_nValue, m_nValue, m_nFractBits );
																						}
	/** Multiplication with an integer. */
	inline void				Mult( const int a )											{	m_nValue *= a; }
	inline void				Mult( const CVisFixpoint & fp, const int a )				{	
																							m_nValue = fp.m_nValue * a; 
																							m_nFractBits = fp.m_nFractBits; 
																						}
	
	inline void				Mult( const unsigned int a )								{	m_nValue *= a; }
	inline void				Mult( const CVisFixpoint & fp, const unsigned int a )		{	m_nValue = fp.m_nValue * a; m_nFractBits = fp.m_nFractBits; }

	/**
	* Divides two fixedpoint numbers and stores the result in this object. 
	*
	* this = a / b
	*
	* Note: that the division is based on a binary division algorithm and produces nearly the
	* maximum available precision (when correctly used), but is really slow and not inlined.
	*/
	void					Div( const CVisFixpoint & fpA, const CVisFixpoint & fpDiv );

	/** Division with integer. */
	inline void				Div( const int a )											{	m_nValue /= a; }
	inline void				Div( const CVisFixpoint & fp, const int a )					{	m_nValue = fp.m_nValue / a; m_nFractBits = fp.m_nFractBits; }
	inline void				Div( const unsigned int a )									{	m_nValue /= a; }
	inline void				Div( const CVisFixpoint & fp, const unsigned int a )		{	m_nValue = fp.m_nValue / a; m_nFractBits = fp.m_nFractBits; }

	/**
	* Computes the square root of a fixedpoint number and stores it in this number. Again,
	* this is the faster function since no object copying is done.
	*
	* this = sqrt(a)
	*/
	void					Sqrt( const CVisFixpoint & fp );

	/**
	* Computes the square root of this number. This is slower because of the copying.
	*
	* return sqrt(this)
	*/
	CVisFixpoint			Sqrt() const;

	/**
	* Returns the integer value of this fixedpoint.
	*/
	Int						GetIntegerValue() const;

	/**
	* Returns a floating point representation of this fixedpoint.
	*/
	float					GetFloatValue() const;

	/**
	* Returns the stored value (Q) of this fixedpoint. This function may be used to feed 
	* fixedpoint values to routines that don't work with CVisFixedpoint objects. Mind the
	* fractional bits, though.
	*/
	Int						GetFixedValue() const;

	// Assignment operators
	inline void				operator=(const CVisFixpoint fp )							{	AssignKeepFractionBits( fp );
																							CHECK_ASSIGN( fp, *this );
																						}
		

	inline void				operator=(const int & nVal)									{ m_nValue = nVal << m_nFractBits; }
	inline void				operator=(const unsigned & unVal)							{ m_nValue = (signed)(unVal << m_nFractBits); }
	inline void				operator=(const float & fVal )								{ m_nValue = ((Int32)( (float)(fVal) * (float)(1<<m_nFractBits) ) ); }
	inline void				operator=(const double & fVal )								{ m_nValue = ((Int32)( (float)(fVal) * (float)(1<<m_nFractBits) ) ); }

	// unary operators
	inline CVisFixpoint		operator-() const											{ CVisFixpoint res; res.Neg(*this); return res; }
	
	// Addition operators
	inline CVisFixpoint		operator+(const CVisFixpoint & fp )	const					{ CVisFixpoint res; res.Add( *this, fp); return res; }
	inline CVisFixpoint		operator+(const int a) const								{ CVisFixpoint res; res.Add( *this, a); return res; }
	inline CVisFixpoint		operator+(const unsigned int a) const						{ CVisFixpoint res; res.Add( *this, a); return res; }
	inline void				operator+=(const CVisFixpoint & fp )						{ Add( fp ); }
	inline void				operator+=(const int a )									{ Add( a ); }
	inline void				operator+=(const unsigned int a )							{ Add( a ); }

	// Subtraction operators
	inline CVisFixpoint		operator-(const CVisFixpoint & fp )	const					{ CVisFixpoint res; res.Sub( *this, fp); return res; }
	inline CVisFixpoint		operator-(const int a) const								{ CVisFixpoint res; res.Sub( *this, a); return res; }
	inline CVisFixpoint		operator-(const unsigned int a) const						{ CVisFixpoint res; res.Sub( *this, a); return res; }
	inline void				operator-=(const CVisFixpoint & fp )						{ Sub( fp ); }
	inline void				operator-=(const int a )									{ Sub( a ); }
	inline void				operator-=(const unsigned int a )							{ Sub( a ); }

	// Multiplication operators
	inline CVisFixpoint		operator*(const CVisFixpoint & fp )	const					{ CVisFixpoint res; res.Mult( *this, fp); return res; }
	inline CVisFixpoint		operator*(const int a) const								{ CVisFixpoint res; res.Mult( *this, a); return res; }
	inline CVisFixpoint		operator*(const unsigned int a) const						{ CVisFixpoint res; res.Mult( *this, a); return res; }
	inline void				operator*=(const CVisFixpoint & fp )						{ Mult( fp ); }
	inline void				operator*=(const int a )									{ Mult( a ); }
	inline void				operator*=(const unsigned int a )							{ Mult( a ); }

	// Division operators
	inline CVisFixpoint		operator/(const CVisFixpoint & fp )	const					{ CVisFixpoint res; res.Div( *this, fp); return res; }
	inline CVisFixpoint		operator/(const int a) const								{ CVisFixpoint res; res.Div( *this, a); return res; }
	inline CVisFixpoint		operator/(const unsigned int a) const						{ CVisFixpoint res; res.Div( *this, a); return res; }
	inline void				operator/=(const CVisFixpoint & fp )						{ Div( *this, fp ); }
	inline void				operator/=(const int a )									{ Div( a ); }
	inline void				operator/=(const unsigned int a )							{ Div( a ); }

	// comparison operators
	inline bool				operator==( const int & i ) const							{ return (m_nValue == (i<<m_nFractBits)); }

	// Shift operators
	inline CVisFixpoint		operator>>( const int i ) const								{ CVisFixpoint res( *this ); res >>= i; return res; }
	inline CVisFixpoint		operator<<( const int i ) const								{ CVisFixpoint res( *this ); res <<= i; return res; }
	inline void				operator>>=( const int i )									{ m_nValue >>= i; }
	inline void				operator<<=( const int i )									{ m_nValue <<= i; }

	// Comparison operators
	inline bool				operator>( const CVisFixpoint & fp ) const					{ CVisFixpoint temp; temp.AdjustFractionBits( m_nFractBits ); temp.AssignKeepFractionBits(fp); return (m_nValue > fp.m_nValue); }
	inline bool				operator<( const CVisFixpoint & fp ) const					{ CVisFixpoint temp; temp.AdjustFractionBits( m_nFractBits ); temp.AssignKeepFractionBits(fp); return (m_nValue < fp.m_nValue); }
	inline bool				operator==( const CVisFixpoint & fp ) const					{ CVisFixpoint temp; temp.AdjustFractionBits( m_nFractBits ); temp.AssignKeepFractionBits(fp); return (m_nValue == fp.m_nValue); }
	
	// conversion operators
	inline					operator int() const										{ return GetIntegerValue(); }
	inline					operator float() const										{ return GetFloatValue(); }

	enum FixpointConsts
	{
		FP_DEFAULT_FRACTBITS = 16
	};

protected:
	
	/** The value that is stored with this fixedpoint number. */
	Int						m_nValue;

	/** The number of fractional bits. */
	Int						m_nFractBits;
	
};

/**
* This is a special, high-performance fixpoint class that assumes that all operations
* take place on fixedpoints with equal number of fractional bits. This accelerates
* the multiplication and addition considerably.
*/
class CVisEqualFixpoint : public CVisFixpoint
{
public:
	/** All constructors have to be explicitly re-defined.... */
	inline						CVisEqualFixpoint() : CVisFixpoint()																			  { };
	inline						CVisEqualFixpoint( const CVisEqualFixpoint & fp ) : CVisFixpoint( fp )								  { };
	inline						CVisEqualFixpoint( const int & fractBits )	: CVisFixpoint( fractBits )									{ };
	inline						CVisEqualFixpoint( const int & nVal, const int & fractBits ) : CVisFixpoint( nVal, fractBits )	{ };

	/**
	* Overwrite the high-speed addition.
	*/	
	inline void					Add( const CVisEqualFixpoint & a, const CVisEqualFixpoint & b )
																										{ m_nValue = (a.m_nValue + b.m_nValue); }
	inline void					Add( const CVisEqualFixpoint & a )
																										{ m_nValue += a.m_nValue; }

	/**
	* Overwrite the high-speed subtraction.
	*/
	inline void					Sub( const CVisEqualFixpoint & a, const CVisEqualFixpoint & b )
																										{ m_nValue = (a.m_nValue - b.m_nValue); }
	inline void					Sub( const CVisEqualFixpoint & b )
																										{ m_nValue -= b.m_nValue; }


	/**
	* Overwrite the high-speed multiplication.
	*/
	inline void					Mult( const CVisEqualFixpoint & a, const CVisEqualFixpoint & b )
																										{ m_nValue		= (	(a.m_nValue >> (m_nFractBits/2)) * (b.m_nValue >> (m_nFractBits/2))); }
	inline void					Mult( const CVisEqualFixpoint & a )
																										{ m_nValue		= (	(a.m_nValue >> (m_nFractBits/2)) * (m_nValue >> (m_nFractBits/2))); }


	/** 
	* Add an extra assignment operator that converts a conventional fixpoint to this number's fraction
	* bit count.
	*/
	inline void					operator=(const CVisFixpoint fp )										{ Int n = m_nFractBits; *((CVisFixpoint*)this) = fp; AdjustFractionBits( n ); }

	/**
	* Add all assignment operators so the compiler isn't tempted to use those of the CVisFixedpoint
	* class instead (which would result in a class conversion and thus new allocation of this
	* object....)
	*/
	inline void					operator=(const CVisEqualFixpoint fp )									{ m_nValue = fp.m_nValue; m_nFractBits = fp.m_nFractBits; }
	inline void					operator=(const int & nVal)												{ m_nValue = nVal << m_nFractBits; }
	inline void					operator=(const float & fVal )											{ m_nValue = ((Int32)( (float)(fVal) * (float)(1<<m_nFractBits) ) ); }
	inline void					operator=(const double & fVal )											{ m_nValue = ((Int32)( (float)(fVal) * (float)(1<<m_nFractBits) ) ); }

	/**
	* Overwrite operators, so they can be used with this class as well (otherwise,
	* the compiler would return a CVisFixpoint object instead of a CVisEqualFixpoint...).
	*/
	inline CVisEqualFixpoint	operator*(const CVisEqualFixpoint & fp )								{ CVisEqualFixpoint res; res.Mult( *this, fp); return res; }
	inline CVisEqualFixpoint	operator+(const CVisEqualFixpoint & fp )								{ CVisEqualFixpoint res; res.Add( *this, fp); return res; }
	inline void					operator+=(const CVisEqualFixpoint & fp )								{ Add( fp ); }
	inline CVisEqualFixpoint	operator-(const CVisEqualFixpoint & fp )								{ CVisEqualFixpoint res; res.Sub( *this, fp); return res; }	
	inline void					operator-=(const CVisEqualFixpoint & fp )								{ Sub( fp ); }
};

#endif

