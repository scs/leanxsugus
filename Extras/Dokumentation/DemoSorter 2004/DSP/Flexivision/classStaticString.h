/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSSSTRING_H_
#define _CLASSSSTRING_H_

#include "FlexiVisioncfg.h"

/**
* @brief A simple string class.
*
* A simple string class that operates on a static string buffer instead of dynamic ones.
*/
class CStaticString
{
public:
	
	/**
	* Constructs the static string with a given char buffer.
	* Note: the static string will not be responsible for the deletion of the buffer.
	*/
	CStaticString( Char * strBuffer, Int nSize );
	
	const Char *			GetString();
	
	const Char *			GetSubString( Int nSubString );
	
	static const Char *		GetSubString( const Char * strString, Int nSubString );
	
	void					Left( Int n );
	void					Right( Int n );
	Int						Lenght( );
	
	Bool					FindDivide( Char c, const Char * & strRest );	
	
	
protected:
	static Int				GetSubStringIndex( const Char * strString, Int nSubString );
	
	
	
	Char *					m_strString;
	
	Int						m_nFirst;
	Int						m_nLast;
	
	Int						m_nBufferSize;	

};

#endif
