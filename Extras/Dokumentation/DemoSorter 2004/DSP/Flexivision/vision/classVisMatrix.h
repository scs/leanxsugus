/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISMATRIX_H_
#define _CLASSVISMATRIX_H_

class CVisMatrix;

	
enum MatrixConsts
{
	MTRX_NUMCOLS 	= 4,
	MTRX_NUMROWS 	= 4,
	MTRX_FRACTBITS 	= 20
};

#include "classVisObject.h"
#include "classVisVector.h"
#include "classVisFixpoint.h"

/**
* @brief A fixedpoint matrix class
*
* A fixedpoint matrix class. The matrix's dimensions are defined by the above enum and
* cannot be changed during runtime. The matrix elements all consist of CVisFixedpoint
* objects (each of size 8 Bytes and thus acceptable...). All accesses to and from the
* matrix are done through fixedpoint numbers.
*
* The matrix also implements the basic operators and allows the multiplication with a
* vector.
*
* @note:	All values used in the vector are of CVisEqualFixpoint type and should be used
*			accordingly. I.e. don't modify them with a fixpoint number with other fraction
*			bit count than MTRX_FRACTBITS, or else the vector will probably get corrupted.
*/
class CVisMatrix : public CVisObject
{
public:
					CVisMatrix();
					CVisMatrix( const CVisMatrix & v );
					~CVisMatrix();

	/**
	* Multiplies each element of the matrix with the given scalar.
	*
	* this = m * fpScalar
	*/
	void			Mult( const CVisMatrix & m, const CVisFixpoint & fpScalar);	

	/**
	* Matrix multiplies two matrices and stores the result in this object.
	*
	* this = m1 * m2
	*/
	void			Mult( const CVisMatrix & m1, const CVisMatrix & m2 );	
		
	/**
	* Adds a scalar value to each of the matrix' elements
	*
	* this = m1 + fpScalar
	*/
	void			Add( const CVisMatrix & m, const CVisFixpoint fpScalar);	

	/**
	* Adds a matrix to another. The addition is performed element-wise.
	*
	* this = m1 + m2
	*/
	void			Add( const CVisMatrix & m1, const CVisMatrix & m2);
	
	/**
	* Clears the matrix and produces an identity matrix.
	*/
	void			MakeIdentity();
	
	/**
	* Produces a rotation matrix that rotates a vector by the given angle around the
	* given axis.
	*/
	void			MakeRotation( const CVisFixpoint & fpAxis_x, const CVisFixpoint & fpAxis_y, const CVisFixpoint & fpAxis_z, const CVisFixpoint & fpAngle );
	
	/**
	* This operator can be used to access single elements of the matrix. The elements
	* may be read and written.
	*/
	CVisFixpoint &	operator()( const Int row, const Int col )							{ return m_fparyStore[(row-1)*MTRX_NUMCOLS + col-1]; }


	/**
	* This operator provides read access to single elements of the matrix where the matrix
	* object is constant.
	*/
	const CVisFixpoint & operator()( const Int row, const Int col ) const				{ return m_fparyStore[(row-1)*MTRX_NUMCOLS + col-1]; }


	/** Assigns a scalar value to each of the matrix' elements. */
	void			operator=( const int nScalar );

	/** Assigns a scalar value to each of the matrix' elements. */
	void			operator=( const CVisFixpoint & fpScalar );

	/** Copy operator. Clones the given matrix completely. */
	void			operator=( const CVisMatrix & m );

	// CVisVector	operator*( const CVisVector & m );
	CVisMatrix		operator*( const CVisMatrix & m );

	void			operator*=( const CVisFixpoint & fpScalar );
	void			operator*=( const CVisMatrix & m );
	
	CVisMatrix		operator+ ( const CVisMatrix & m );
	void			operator+=( const CVisFixpoint & fpScalar );
	void			operator+=( const CVisMatrix & m );

	void			FormatMatrix( char * str ) const ;

	void			Log( char * str ) const;
	

protected:
	CVisEqualFixpoint	m_fparyStore[MTRX_NUMCOLS*MTRX_NUMROWS];
	

	friend class	CVisVector;
};

#endif
