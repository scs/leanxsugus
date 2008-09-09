/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISGRAPHICS_H_
#define _CLASSVISGRAPHICS_H_

#include "classVisPort.h"

/**
* @brief A common graphics function library, packed in a class.
*
* A common graphics function library, packed in a class.
*/
class CVisGraphics
{
public:
	CVisGraphics( Ptr pBuffer, int nWidth, int nHeight, int nBpp);

	void					Clear( const Uint32 clearColor );

	void					SetPixel( const Int x, const Int y, const Uint32 color);
	Uint32					GetPixel( const Int x, const Int y);

	Uint32					DrawInteger( const Uint32 unNumber, const Int32 x, const Int32 y, const Uint32 color, const Int nScale = 1);
	Uint32					DrawFixedpointPercent( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
												const Int32 x, const Int32 y, const Uint32 color, const Int nScale = 1);
	Uint32					DrawFixedpoint( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
												const Int32 x, const Int32 y, const Uint32 color, const Int nScale = 1);
	Uint32					DrawDigit( const Char cDigit, const Int32 x, const Int32 y, const Uint32 color, const Int nScale = 1);
	
	void					DrawHorizLine( const Int32 x_0, const Int32 x_1, const Int32 y, const Uint32 color);
	void					DrawVertLine( const Int32 x, const Int32 y_0, const Int32 y_1, const Uint32 color);
	void					DrawLine( const Int x_0, const Int y_0, const Int x_1, const Int y_1, const Uint32 color);
		
	void					FillRect( const Int x, const Int y, const Int width, const Int height, const Uint32 color );

	void					HorizFlood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor );
	void					EasyFlood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor, const int dir=0, const int stack=0 );
	void					Flood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor );

protected:
	void					DrawLineOctant0( Int x0, Int y0, Int deltaX, const Int deltaY, const Int Xdirection, const Uint32 color);
	void					DrawLineOctant1( Int x0, Int y0, const Int deltaX, Int deltaY, const Int Xdirection, const Uint32 color);

	Ptr						m_pBuffer;
	int						m_nWidth;
	int						m_nHeight;
	int						m_nBpp;
};

#endif
