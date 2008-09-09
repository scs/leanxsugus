
#include "classVisMatrix.h"

#include <math.h>
#include <stdio.h>

// *************************************************************************

CVisMatrix::CVisMatrix()
	: 	CVisObject( CT_MATH )
{
	// Initialize all fixedpoint numbers' fraction bits.
	for ( int i=0; i<MTRX_NUMCOLS*MTRX_NUMROWS; i++)
	{
		m_fparyStore[i].InitFractionBits( MTRX_FRACTBITS );
		m_fparyStore[i] = 0.0;
	}
}

// *************************************************************************

CVisMatrix::CVisMatrix( const CVisMatrix & v )
	: 	CVisObject( "Matrix", CT_MATH )
{
	// Don't have to initialize the fraction bits, since this is done in the
	// copy operator.
	*this = v;
}

// *************************************************************************

CVisMatrix::~CVisMatrix()
{
}

// *************************************************************************

void CVisMatrix::Mult( const CVisMatrix & m, const CVisFixpoint & fpScalar)
{
	// Convert the scalar, which can be any fixpoint, to the equal fixpoint format
	// we use here.
	CVisEqualFixpoint eq( MTRX_FRACTBITS );
	eq = fpScalar;
	
	for ( int i=0; i<MTRX_NUMCOLS*MTRX_NUMROWS; i++)
		//m_fparyStore[i] = FP_MUL( m.m_fparyStore[i], fpScalar, MTRX_FRACTBITS );
		m_fparyStore[i].Mult( m.m_fparyStore[i], eq );
}

// *************************************************************************

void CVisMatrix::Mult( const CVisMatrix & m1, const CVisMatrix & m2 )
{
	CVisEqualFixpoint mul( MTRX_FRACTBITS );
	CVisEqualFixpoint acc( MTRX_FRACTBITS );

	for ( int row=0; row < MTRX_NUMROWS; row++ )
	{
		for ( int col=0; col < MTRX_NUMCOLS; col++ )							   
		{
			acc = 0;
			
			for ( int i=0; i<MTRX_NUMCOLS; i++)
			{		
				mul.Mult( m1.m_fparyStore[row*MTRX_NUMCOLS + i], m2.m_fparyStore[i*MTRX_NUMCOLS + col] );
				acc.Add( mul );
			}
			
			m_fparyStore[ row*MTRX_NUMCOLS + col ] = acc;
		}
	}
}


// *************************************************************************

void CVisMatrix::Add( const CVisMatrix & m, const CVisFixpoint fpScalar)	
{
	// Convert the scalar, which can be any fixpoint, to the equal fixpoint format
	// we use here.
	CVisEqualFixpoint eq( MTRX_FRACTBITS );
	eq = fpScalar;

	for ( int row=0; row < MTRX_NUMROWS; row++ )
	{
		for ( int col=0; col < MTRX_NUMCOLS; col++ )							   
		{
			m_fparyStore[ row*MTRX_NUMCOLS + col ].Add( eq );
		}
	}
}

// *************************************************************************

void CVisMatrix::Add( const CVisMatrix & m1, const CVisMatrix & m2)
{
	CVisEqualFixpoint fp1, fp2, fp3;

	fp1 = fp2 + fp3;
	
	for ( int row=0; row < MTRX_NUMROWS; row++ )
	{
		for ( int col=0; col < MTRX_NUMCOLS; col++ )							   
		{
			m_fparyStore[ row*MTRX_NUMCOLS + col ].Add( m1.m_fparyStore[ row*MTRX_NUMCOLS + col ], m2.m_fparyStore[ row*MTRX_NUMCOLS + col ] );
		}
	}
}


// *************************************************************************

void CVisMatrix::MakeIdentity()
{
	for ( int row=0; row < MTRX_NUMROWS; row++ )
	{
		for ( int col=0; col < MTRX_NUMCOLS; col++ )	
		{
			m_fparyStore[ row*MTRX_NUMCOLS + col ] = (col==row) ? 1 : 0 ;
		}
	}
}
	
// *************************************************************************

void CVisMatrix::MakeRotation( const CVisFixpoint & fpAxis_x, const CVisFixpoint & fpAxis_y, const CVisFixpoint & fpAxis_z, const CVisFixpoint & fpAngle )
{
	float cos_al = (float)cos( fpAngle.GetFloatValue() );
	float sin_al = (float)sin( fpAngle.GetFloatValue() );
	float one_cos_al = 1-cos_al;

	float a_x, a_y, a_z;

	a_x = fpAxis_x.GetFloatValue();
	a_y = fpAxis_y.GetFloatValue();
	a_z = fpAxis_z.GetFloatValue();

	// normalize axis.
	float len = (float)sqrt(a_x*a_x + a_y*a_y + a_z*a_z);
	if ( len < 0.000001 )
		return;
	if ( len != 1.0 )
	{
		a_x /= len;
		a_y /= len;
		a_z /= len;
	}
	// First, generate the identity matrix
	MakeIdentity();

	// Now generate the three columns of the rotation matrix
	(*this)( 1,1 ) = one_cos_al*a_x*a_x + cos_al;
	(*this)( 2,1 ) = one_cos_al*a_x*a_y + sin_al*a_z;
	(*this)( 3,1 ) = one_cos_al*a_x*a_z - sin_al*a_y;

	(*this)( 1,2 ) = one_cos_al*a_x*a_y - sin_al*a_z;
	(*this)( 2,2 ) = one_cos_al*a_y*a_y + cos_al;
	(*this)( 3,2 ) = one_cos_al*a_y*a_z + sin_al*a_x;

	(*this)( 1,3 ) = one_cos_al*a_x*a_z + sin_al*a_y;
	(*this)( 2,3 ) = one_cos_al*a_y*a_z - sin_al*a_x;
	(*this)( 3,3 ) = one_cos_al*a_z*a_z + cos_al;

	// Don't have to set column 4...
}

// *************************************************************************

void CVisMatrix::operator=( const int nScalar )
{
	CVisFixpoint fp( MTRX_FRACTBITS );
	fp = nScalar;

	*this = fp;
}

// *************************************************************************

void CVisMatrix::operator=( const CVisFixpoint & fpScalar )
{
	// Convert the scalar, which can be any fixpoint, to the equal fixpoint format
	// we use here.
	CVisEqualFixpoint eq( MTRX_FRACTBITS );
	eq = fpScalar;

	for ( int i=0; i<MTRX_NUMCOLS*MTRX_NUMROWS; i++)
		m_fparyStore[i] = fpScalar;
}

// *************************************************************************

void CVisMatrix::operator=( const CVisMatrix & m )
{
	for ( int i=0; i<MTRX_NUMCOLS*MTRX_NUMROWS; i++)
		m_fparyStore[i] = m.m_fparyStore[i];
}

// *************************************************************************

void CVisMatrix::operator*=( const CVisFixpoint & fpScalar )
{
	Mult( *this, fpScalar );
}

// *************************************************************************

void CVisMatrix::operator*=( const CVisMatrix & m )
{
	CVisMatrix temp;
	
	temp.Mult( *this, m );	
	*this = temp;
}

// ***********************************************************************

CVisMatrix CVisMatrix::operator*( const CVisMatrix & m )
{
	CVisMatrix temp;
	
	temp.Mult( *this, m );	
	return temp;
}

// *************************************************************************

CVisMatrix  CVisMatrix::operator+ ( const CVisMatrix & m )
{
	CVisMatrix temp;
	
	temp.Add( *this, m );
	return temp;
}

// *************************************************************************

void CVisMatrix::operator+=( const CVisFixpoint & fpScalar )
{
	CVisMatrix temp;
	
	temp.Add( *this, fpScalar );
	*this = temp;
}

// *************************************************************************

void CVisMatrix::operator+=( const CVisMatrix & m )
{
	CVisMatrix temp;
	
	temp.Add( *this, m );
	*this = temp;
}

// *************************************************************************

void CVisMatrix::FormatMatrix( char * str ) const
{
	int offs = 0;
	for ( int row=1; row <= MTRX_NUMROWS; row++ )
	{
		for ( int col=1; col <= MTRX_NUMCOLS; col++ )							   
		{
			offs += sprintf( str+offs, "%.3f\t", (float)(*this)(row,col));
		}

		offs += sprintf( str+offs, "\n");
	}
}


// *************************************************************************

void CVisMatrix::Log( char * str ) const
{
	char strm[1024];

	FormatMatrix( strm );

	LogMsg( "%s:\n%s", str, strm );
}

// *************************************************************************
