
#include "classVisVector.h"

#include <stdio.h>


CVisVector::CVisVector()
	: 	CVisObject( CT_MATH )
{
	// Initialize all fixedpoint numbers' fraction bits.
	for ( int i=0; i<MTRX_NUMCOLS; i++)
	{
		m_fparyStore[i].InitFractionBits( MTRX_FRACTBITS );
		m_fparyStore[i] = 0.0;
	}
}

// *************************************************************************

CVisVector::CVisVector( const CVisVector & v )
	: 	CVisObject( "Vector", CT_MATH )
{
	*this = v;
}

// *************************************************************************

CVisVector::~CVisVector()
{
}

// *************************************************************************

CVisFixpoint CVisVector::DotProd( const CVisVector & v ) const
{
	CVisEqualFixpoint acc( MTRX_FRACTBITS );
	CVisEqualFixpoint mul( MTRX_FRACTBITS );
	acc = 0;
	
	for ( int row=0; row < 3/*MTRX_NUMCOLS*/; row++ )
	{
		mul.Mult( m_fparyStore[row], v.m_fparyStore[row] );
		acc.Add( mul );				
	}

	/*
	char str1[512], str2[512];
	FormatMatrix(str1);
	v.FormatMatrix(str2);
	LogMsg("Dot Product:\n%s\n.\n%s\n = %f", str1, str2, (float)acc );
	*/	
	
	return acc;
}	
	
// *************************************************************************

void CVisVector::CrossProd( const CVisVector & v1, const CVisVector & v2 )
{
	CVisEqualFixpoint mul1( MTRX_FRACTBITS );
	CVisEqualFixpoint mul2( MTRX_FRACTBITS );

	mul1.Mult( v1.m_fparyStore[1], v2.m_fparyStore[2] );
	mul2.Mult( v1.m_fparyStore[2], v2.m_fparyStore[1] );
	m_fparyStore[0].Sub(  mul1, mul2 );

	mul1.Mult( v1.m_fparyStore[2], v2.m_fparyStore[0] );
	mul2.Mult( v1.m_fparyStore[0], v2.m_fparyStore[2] );
	m_fparyStore[1].Sub(  mul1, mul2 );

	mul1.Mult( v1.m_fparyStore[0], v2.m_fparyStore[1] );
	mul2.Mult( v1.m_fparyStore[1], v2.m_fparyStore[0] );
	m_fparyStore[2].Sub(  mul1, mul2 );

	m_fparyStore[3] = 0;
}	

// *************************************************************************

void CVisVector::Add( const CVisVector & v1, const CVisVector & v2 )
{
	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{
		m_fparyStore[row].Add( v1.m_fparyStore[row], v2.m_fparyStore[row] );
	}
}

// *************************************************************************

void CVisVector::Add( const CVisVector & v )
{
	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{
		m_fparyStore[row].Add( v.m_fparyStore[row] );
	}
}

// *************************************************************************

void CVisVector::Sub( const CVisVector & v1, const CVisVector & v2 )
{
	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{
		m_fparyStore[row].Sub( v1.m_fparyStore[row], v2.m_fparyStore[row] );
	}
}

// *************************************************************************

void CVisVector::Mult( const CVisVector & v, CVisFixpoint fpScalar )
{
	// Convert the scalar, which can be any fixpoint, to the equal fixpoint format
	// we use here.
	CVisEqualFixpoint eq( MTRX_FRACTBITS );
	eq = fpScalar;

	for ( int i=0; i<MTRX_NUMCOLS; i++)
		m_fparyStore[i].Mult( v.m_fparyStore[i], eq );
}

// *************************************************************************

void CVisVector::Mult( const CVisMatrix & m, const CVisVector & v )
{
	// We have to use the same fraction bit count as in the matrix and vector.
	CVisEqualFixpoint acc( MTRX_FRACTBITS );
	CVisEqualFixpoint mul( MTRX_FRACTBITS );

	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{		
		acc = 0;

		for ( int i=0; i < MTRX_NUMCOLS; i++ )							   
		{
			mul.Mult( m.m_fparyStore[row*MTRX_NUMCOLS + i], v.m_fparyStore[i] );
			acc.Add( mul );
		}
		
		m_fparyStore[ row ] = acc;	
	}
}

// *************************************************************************

void CVisVector::operator=( const int nScalar )
{
	CVisFixpoint fp( MTRX_FRACTBITS );
	fp = nScalar;

	*this = fp;
}
	
// *************************************************************************

void CVisVector::operator=( const CVisFixpoint fpScalar )
{
	// Convert the scalar, which can be any fixpoint, to the equal fixpoint format
	// we use here.
	CVisEqualFixpoint eq( MTRX_FRACTBITS );
	eq = fpScalar;
	
	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{
		m_fparyStore[row] = eq;
	}
}
	
// *************************************************************************
	
void CVisVector::operator=( const CVisVector & v )
{
	for ( int row=0; row < MTRX_NUMCOLS; row++ )
	{
		m_fparyStore[row] = v.m_fparyStore[row];
	}
}
	
// *************************************************************************
	

void CVisVector::FormatMatrix( char * str ) const
{
	int offs = 0;
	for ( int row=1; row <= MTRX_NUMCOLS; row++ )
	{
		offs += sprintf( str+offs, "%.3f\n", (float)(*this)(row) );
	}
}

// *************************************************************************

void CVisVector::Log( char * str ) const
{
	LogMsg( "%s:\n[%.3f\t%.3f\t%.3f\t%.3f]", str, (float)(*this)(1), (float)(*this)(2), (float)(*this)(3), (float)(*this)(4) );
}

// *************************************************************************


// *************************************************************************

CVisVector operator*( const CVisMatrix & m, const CVisVector & v )
{
	CVisVector res;
	
	res.Mult( m, v );	
	return res;
}

// *************************************************************************

CVisFixpoint DotProduct( const CVisVector & v1, const CVisVector & v2 )
{
	CVisVector temp;
	
	temp = v1;
	return temp.DotProd( v2 );
}

// *************************************************************************

CVisVector CrossProduct( const CVisVector & v1, const CVisVector & v2 )
{
	CVisVector res;
	
	res.CrossProd( v1, v2 );
	return res;
}

// *************************************************************************

