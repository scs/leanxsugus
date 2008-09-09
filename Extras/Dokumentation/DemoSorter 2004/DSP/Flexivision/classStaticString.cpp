
#include "classStaticString.h"

CStaticString::CStaticString( Char * strBuffer, Int nSize )
{
	m_strString = strBuffer;
	m_nBufferSize = nSize;
	
	m_nFirst = 0;
	m_nLast = nSize;
	
	
}

// *************************************************************************

const Char *  CStaticString::GetString()
{
	return m_strString + m_nFirst;
}

// *************************************************************************

const Char * CStaticString::GetSubString( Int nSubString )
{
	Int nIndex = GetSubStringIndex( GetString(), nSubString );
	
	// Return the result
	if ( nIndex != -1 )	
		return m_strString + nIndex;
	else
		return NULL;
}
// *************************************************************************

const Char *	CStaticString::GetSubString( const Char * strString, Int nSubString )
{
	Int nIndex = GetSubStringIndex( strString, nSubString );
	
	// Return the result
	if ( nIndex != -1 )	
		return strString + nIndex;
	else
		return NULL;
}

// *************************************************************************

void CStaticString::Left( Int n )
{
	Int newLast;
	
	if ( n <= 0 )
		return;
		
	newLast = m_nFirst + n - 1;
	
	if ( (newLast < m_nLast) && (newLast < m_nBufferSize-1 ) )
	{
		m_nLast = newLast;
		m_strString[ m_nLast+1 ] = '\0';
	}
}

// *************************************************************************

void CStaticString::Right( Int n )
{	
	Int newFirst;
	
	if ( n <= 0 )
		return;
		
	newFirst = m_nLast - n + 1;
	
	if ( (newFirst > m_nFirst) && (newFirst >= 0 ) )
	{
		m_nFirst = newFirst;		
	}
}

// *************************************************************************

Int CStaticString::Lenght( )
{
	Int l=0;
	
	while ( (m_nFirst + l < m_nBufferSize) && ( m_strString[m_nFirst + l] != '\0' ))
		l++;
		
	// Error: no \0 found. Correct that by inserting one.
	if (m_nFirst + l == m_nBufferSize)
	{
		m_strString[ m_nBufferSize-1 ] = '\0';
		m_nLast = m_nBufferSize-2;
		return (l-1);
	}
	
	m_nLast = m_nFirst + l - 1;
	return l;		
}

// *************************************************************************

Bool CStaticString::FindDivide( const Char c, const Char * & strRest )
{
	Int l=0;
	
	while ( (m_nFirst + l < m_nBufferSize) && ( m_strString[m_nFirst + l] != '\0' ))
	{
		if ( m_strString[m_nFirst + l] == c )
		{
			strRest = m_strString + m_nFirst + l + 1;
			m_strString[m_nFirst + l] = 0;
			m_nLast = m_nFirst + l - 1;
			return TRUE;
		}
		
		l++;
	}
	
	return FALSE;
}

// *************************************************************************

Int CStaticString::GetSubStringIndex( const Char * strString, Int nSubString )
{
	Int nIndex = 0;

	// If the string begins with a space, drop that
	while ( strString[ nIndex ] == ' ' )
	{
		nIndex++;
		if ( strString[ nIndex ] == 0 )
			return -1;
	} 
	
	// Find nSubString whitespaces
	for ( Int i=0; i<nSubString; i++)
	{
		// Advance to next space
		while ( strString[ nIndex ] != ' ' )
		{
			nIndex++;
			if ( strString[ nIndex ] == 0 )
				return -1;
		} 
		
		// Advance to next space
		while ( strString[ nIndex ] == ' ' )
		{
			nIndex++;
			if ( strString[ nIndex ] == 0 )
				return -1;
		} 		
	}
	
	// Return the result
	return nIndex;
}

// *************************************************************************
