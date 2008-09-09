/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISVECTOR_H_
#define _CLASSVISVECTOR_H_

class CVisVector;

#include "classVisObject.h"
#include "classVisMatrix.h"
#include "classVisFixpoint.h"

/**
* @brief A 4 dimensional vector class using fixpoint arithmethics
*
* A 4 dimensional vector class using fixpoint arithmethics. The vector class is optimized
* for being used along the CVisMatrix class in coordinate transforming algorithms and thus
* is limited to 3 dimensions, the fourth component being constant (either 0, for normal
* vector operation or 1, for homogenous coordinates).
*
* @note:	All values used in the vector are of CVisEqualFixpoint type and should be used
*			accordingly. I.e. don't modify them with a fixpoint number with other fraction
*			bit count than MTRX_FRACTBITS, or else the vector will probably get corrupted.
*/
class CVisVector : public CVisObject
{
public:
	/** Default constructor. Initializes the vector to 0. */
								CVisVector();

	/** Copy constructor. Neede for return values of type CVisVector. */
								CVisVector( const CVisVector & v );

	/** Destructor. Nothing to be done here. */
	virtual						~CVisVector();
	
	/**
	* Calculates the dot product of this vector against another vector and returns the
	* result as a fixpoint number.
	*
	* return this . v
	*/
	CVisFixpoint				DotProd( const CVisVector & v ) const;

	/**
	* Calculates the cross product of two vectors and stores the result in this object.
	*
	* this = v1 x v2
	*/
	void						CrossProd( const CVisVector & v1, const CVisVector & v2 );
	
	void						Add( const CVisVector & v1, const CVisVector & v2 );
	void						Add( const CVisVector & v );

	void						Sub( const CVisVector & v1, const CVisVector & v2 );

	void						Mult( const CVisVector & v, CVisFixpoint fpScalar );

	/**
	* Multiplies this matrix with a vector, resulting in a vector.
	*
	* this = m * v
	*/
	void						Mult( const CVisMatrix & m, const CVisVector & v );
	
	/** Assigns an integer scalar to all elements of this vector.*/
	void						operator=( const int nScalar );

	/** Assigns an fixpoint scalar to all elements of this vector.*/
	void						operator=( const CVisFixpoint fpScalar );

	/** Copy operator. */
	void						operator=( const CVisVector & v );
	
	/** 
	* This operator allows accessing the single elements of the vector. For better
	* speed, it is inlined. Elements accessed through this operator may also be
	* modified.
	*/
	inline CVisFixpoint &		operator()( const Int row )					{ return m_fparyStore[ row - 1 ]; }

	/** 
	* This operator allows read access to the single elements of the vector. For better
	* speed, it is inlined. This operator is only chosen when this object is constant and
	* thus the returned values may not be changed.
	*/
	const inline CVisFixpoint &	operator()( const Int row ) const			{ return m_fparyStore[ row - 1 ]; }

	/** Formats the vector into a string. */
	void						FormatMatrix( char * str ) const;

	void						Log( char * str ) const;
		

protected:
	/** Stores the fixedpoint values. */
	CVisEqualFixpoint			m_fparyStore[MTRX_NUMCOLS];
};

/** 
* Operator that defines the multiplication of a matrix and a vector. Because of the
* different classes, the operator cannot be defined in CVisVector.
*/
CVisVector				operator*( const CVisMatrix & m, const CVisVector & v );

/** Standalone function that enhances readability. @see DotProd()*/
CVisFixpoint			DotProduct( const CVisVector & v1, const CVisVector & v2 );

/** Standalone function that enhances readability. @see CrossProd()*/
CVisVector				CrossProduct( const CVisVector & v1, const CVisVector & v2 );

#endif
