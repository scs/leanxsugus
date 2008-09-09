
#include "classVisGraphics.h"

#define NUM_WIDTH 5
#define NUM_HEIGHT 7
const Uint8 unNumbers[12][NUM_WIDTH*NUM_HEIGHT] =
{
	// 0
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 1
	{	0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0	},
	// 2
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 1, 0, 0,		0, 1, 0, 0, 0,		1, 1, 1, 1, 1	},
	// 3
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 4
	{	0, 0, 0, 1, 0,		0, 0, 1, 1, 0,		0, 1, 0, 1, 0,		1, 0, 0, 1, 0,		1, 1, 1, 1, 1,		0, 0, 0, 1, 0,		0, 0, 0, 1, 0	},
	// 5
	{	0, 1, 1, 1, 1,		0, 1, 0, 0, 0,		0, 1, 0, 0, 0,		0, 1, 1, 1, 0,		0, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 6
	{	0, 1, 1, 1, 1,		1, 0, 0, 0, 0,		1, 0, 0, 0, 0,		1, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 7
	{	1, 1, 1, 1, 1,		0, 0, 0, 0, 1,		0, 0, 0, 0, 1,		0, 0, 0, 1, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0,		0, 0, 1, 0, 0	},
	// 8
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 0	},
	// 9
	{	0, 1, 1, 1, 0,		1, 0, 0, 0, 1,		1, 0, 0, 0, 1,		0, 1, 1, 1, 1,		0, 0, 0, 0, 1,		0, 0, 0, 0, 1,		1, 1, 1, 1, 0	},
	// .
	{	0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 0, 0, 0,		0, 0, 1, 0, 0	},
	// %
	{	0, 1, 1, 0, 0,		0, 1, 1, 0, 0,		0, 0, 0, 0, 1,		0, 1, 1, 1, 0,		1, 0, 0, 0, 0,		0, 0, 1, 1, 0,		0, 0, 1, 1, 0	}
};

// *************************************************************************

CVisGraphics::CVisGraphics( Ptr pBuffer, int nWidth, int nHeight, int nBpp)
{
	m_pBuffer = pBuffer;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nBpp = nBpp;
}

// *************************************************************************

void CVisGraphics::Clear( const Uint32 clearColor )
{
	int i;
	switch (m_nBpp)
	{
	case 8:
		for (i=0; i<m_nWidth*m_nHeight; i++)
			((Uint8*)m_pBuffer)[ i ] = (Uint8)(clearColor && 0x000000FF);
		break;

	case 16:
		for (i=0; i<m_nWidth*m_nHeight; i++)
			((Uint16*)m_pBuffer)[ i ] = (Uint16)(clearColor && 0x0000FFFF);
		break;

	case 32:
		for (i=0; i<m_nWidth*m_nHeight; i++)
			((Uint32*)m_pBuffer)[ i ] = clearColor;
		break;
	}
}

// *************************************************************************

Uint32 CVisGraphics::DrawInteger( const Uint32 unNumber, const Int32 x, const Int32 y, const Uint32 color, const Int nScale)
{
	Uint8 unNumArray[16];
	Uint32 index = 0;
	Uint32 num = unNumber;

	// First, arrange the number array
	do
	{
		unNumArray[index] = num % 10;
		num = num/10;
		index++;
	} while ( num > 0 );


	// Then print it to the image
	Uint32 xpos = x;
	for ( Uint32 i=0; i<index; i++ )
		xpos += DrawDigit( unNumArray[index-i-1], xpos, y, color, nScale );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisGraphics::DrawFixedpointPercent( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color, const Int nScale)
{
	Uint32 xpos = x;

	xpos += DrawFixedpoint( unNumber * 100, unBase, unPrecision, xpos, y, color, nScale );
	xpos += DrawDigit( 11, xpos, y, color, nScale );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisGraphics::DrawFixedpoint( const Uint32 unNumber, const Uint32 unBase, const Uint32 unPrecision,
											const Int32 x, const Int32 y, const Uint32 color, const Int nScale)
{
	Uint8 unNumArray[16];
	Uint32 index = 0;
	Uint32 num;
	Uint32 prec;
	Uint32 xpos = x;

	// First, draw the digits before the separator
	num = unNumber >> unBase;
	xpos += DrawInteger( num, xpos, y, color, nScale );

	// Draw the separator
	xpos += DrawDigit( 10, xpos, y, color, nScale );

	// Then prepare the digits after the separator
	prec = 0;
	num = unNumber - ( (unNumber >> unBase) << unBase );
	do
	{
		// Multiply by 10.
		num = num * 10;

		// Get the digit.
		unNumArray[index] = num >> unBase;

		// Subtract the whole numbers
		num = num - ( (num >> unBase) << unBase );

		prec++;
		index++;
	} while ( (prec < unPrecision) && (index<16) );

	// Then print it to the image
	for ( Uint32 i=0; i<index; i++ )
		xpos += DrawDigit( unNumArray[i], xpos, y, color, nScale );

	return xpos - x;
}

// *************************************************************************

Uint32 CVisGraphics::DrawDigit( const Char cDigit, const Int32 x, const Int32 y, const Uint32 color, const Int nScale)
{
	for ( Uint32 k = 0; k<NUM_WIDTH*nScale; k++)
	{
		for ( Uint32 l=0; l<NUM_HEIGHT*nScale; l++)
		{
			Int32 xpos = x + k;
			Int32 ypos = l + y;
			
			if ( unNumbers[ cDigit ] [(k/nScale)+(l/nScale)*NUM_WIDTH] != 0)
				SetPixel( xpos, ypos, color );			
		}
	}

	return NUM_WIDTH*nScale + 1;
}

// *************************************************************************

void CVisGraphics::DrawHorizLine( const Int32 x_0, const Int32 x_1, const Int32 y, const Uint32 color)
{
	DrawLine( x_0, y, x_1, y, color );
}
					
// *************************************************************************

void CVisGraphics::DrawVertLine( const Int32 x, const Int32 y_0, const Int32 y_1, const Uint32 color)
{
	DrawLine( x, y_0, x, y_1, color );
}

// *************************************************************************

#define SWAP(a,b, temp)	temp=a; a=b; b=temp;

void CVisGraphics::DrawLine( const Int x_0, const Int y_0, const Int x_1, const Int y_1, const Uint32 color)
{
	Int deltaX, deltaY;
	Int temp;

	Int x0 = x_0;
	Int x1 = x_1;
	Int y0 = y_0;
	Int y1 = y_1;
	
	// y0 has to be smaller or equal y1, swap points if necessary.
	if (y0>y1)
	{
		SWAP(y0,y1, temp);
		SWAP(x0,x1, temp);
	}

	deltaX = x1 - x0;
	deltaY = y1 - y0;

	// 2 cases possible: from left to right or from right to left. Distinguish those.
	if (deltaX > 0)
	{
		// 2 more cases possible: dy is bigger than dx and vice versa. We have to use 
		// a particular function for each of those.
		if ( deltaX > deltaY )
			DrawLineOctant0( x0, y0, deltaX, deltaY, 1, color);
		else
			DrawLineOctant1( x0, y0, deltaX, deltaY, 1, color);
	}
	else
	{
		// invert deltaX
		deltaX= -deltaX;
		
		// 2 more cases possible: dy is bigger than dx and vice versa. We have to use 
		// a particular function for each of those.
		if (deltaX > deltaY)
			DrawLineOctant0( x0, y0, deltaX, deltaY, -1, color);
		else
			DrawLineOctant1( x0, y0, deltaX, deltaY, -1, color);
	}
}

// *************************************************************************

void CVisGraphics::SetPixel( const Int x, const Int y, const Uint32 color)
{
	if ( (x>= 0)
		&& (y >= 0)
		&& (x < m_nWidth)
		&& (y < m_nHeight) )
	{
		switch (m_nBpp)
		{
		case 8:
			((Uint8*)m_pBuffer)[ x + y*m_nWidth ] = (Uint8)(color & 0x000000FF);
			break;

		case 16:
			((Uint16*)m_pBuffer)[ x + y*m_nWidth ] = (Uint16)(color & 0x0000FFFF);
			break;

		case 32:
			((Uint32*)m_pBuffer)[ x + y*m_nWidth ] = color;
			break;
		}
	}					
}

	
// *************************************************************************

Uint32 CVisGraphics::GetPixel( const Int x, const Int y)
{
	if ( x>= 0 
		&& y >= 0
		&& x < m_nWidth
		&& y < m_nHeight )
	{
		switch (m_nBpp)
		{
		case 8:
			return ((Uint8*)m_pBuffer)[ x + y*m_nWidth ];

		case 16:
			return ((Uint16*)m_pBuffer)[ x + y*m_nWidth ];

		case 32:
			return ((Uint32*)m_pBuffer)[ x + y*m_nWidth ];
		}
	}					

	return 0;
}
	
// *************************************************************************

void CVisGraphics::FillRect( const Int x, const Int y, const Int width, const Int height, const Uint32 color )
{
	for ( int j=y; j<y+height; j++ )
		for (int i=x; i<x+width; i++)
			SetPixel( i,j, color );
		
}

// *************************************************************************

void CVisGraphics::HorizFlood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor )
{
	int y=y_0;
	int x=x_0;
	
	// Abort if we're on a set pixel!
	if ( (GetPixel(x,y_0) == floodColor ) || (GetPixel(x,y_0) == stopColor ) )
		return;

	// Go up on y until we reach a border
	while ( (GetPixel(x_0,y) != floodColor ) && (GetPixel(x_0,y) != stopColor )  && (y>0))
		y--;

	// now go down and flood each horizontal line.
	while ( (GetPixel(x_0,y) != floodColor ) && (GetPixel(x_0,y) != stopColor )  && (y<m_nHeight-1) )
	{
		x = x_0;

		// move to the left.
		while ( (GetPixel(x,y) != floodColor ) && (GetPixel(x,y) != stopColor ) && (x>0) )
			x--;

		// Fill all up while moving right
		x+=1;
		while ( (GetPixel(x,y) != floodColor ) && (GetPixel(x,y) != stopColor ) && (x<m_nWidth-1) )
		{
			SetPixel( x,y, floodColor );
			x++;
		}
		
		// Next line
		y++;
	}
}
	
// *************************************************************************

void  CVisGraphics::EasyFlood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor, const int dir, const int stack )
{
	int x=x_0;
	int x_left;
	int x_right;
	
	if (stack > 100)
		return;

	if (dir==0)
	{
		EasyFlood( x_0, y_0, floodColor, stopColor, -1 );
		EasyFlood( x_0, y_0+1, floodColor, stopColor, 1 );
	}

	// Abort if we're on a set pixel!
	if ( (GetPixel(x,y_0) == floodColor ) || (GetPixel(x,y_0) == stopColor ) )
		return;

	// move to the left.
	while ( (GetPixel(x,y_0) != floodColor ) && (GetPixel(x,y_0) != stopColor ) && (x>0) )
		x--;

	x_left = x;

	// Fill all up while moving right
	x+=1;
	while ( (GetPixel(x,y_0) != floodColor ) && (GetPixel(x,y_0) != stopColor ) && (x<m_nWidth-1) )
	{
		SetPixel( x,y_0, floodColor );
		x++;
	}
	
	x_right = x;

	// Now check adjacent lines. First the upper line.
	if ( (y_0 > 0) && (y_0 < m_nHeight-1 ) )
	{
		x = x_right-1;
		while ( x>x_left )
		{
			if ( ( GetPixel(x,y_0+dir) != stopColor ) && ( GetPixel(x,y_0+dir) != floodColor ) )
			{
				EasyFlood( x, y_0, floodColor, stopColor, dir, stack+1 );

				// go left until we reach a new segment
				while ( ( GetPixel(x,y_0+dir) != stopColor ) && ( GetPixel(x,y_0+dir) != floodColor ) && (x>x_left) )
					x--;
			}

			x--;
		}
	}
}

// *************************************************************************

void  CVisGraphics::Flood( const Int x_0, const Int y_0, const Uint32 floodColor, const Uint32 stopColor )
{
	int x=x_0;
	int x_left;
	int x_right;

	// Abort if we're on a set pixel!
	if ( (GetPixel(x,y_0) == floodColor ) || (GetPixel(x,y_0) == stopColor ) )
		return;

	// move to the left.
	while ( (GetPixel(x,y_0) != floodColor ) && (GetPixel(x,y_0) != stopColor ) && (x>0) )
		x--;

	x_left = x;

	// Fill all up while moving right
	x+=1;
	while ( (GetPixel(x,y_0) != floodColor ) && (GetPixel(x,y_0) != stopColor ) && (x<m_nWidth-1) )
	{
		SetPixel( x,y_0, floodColor );
		x++;
	}
	
	x_right = x;

	if ( x_right == m_nWidth-1 )
		return;

	// Now check adjacent lines. First the upper line.
	if ( (y_0 > 0) && (y_0 < m_nHeight-1 ) )
	{
		x = x_right-1;
		while ( x>x_left )
		{
			if ( ( GetPixel(x,y_0-1) != stopColor ) && ( GetPixel(x,y_0-1) != floodColor ) )
			{
				Flood( x, y_0-1, floodColor, stopColor );

				// go left until we reach a new segment
				while ( ( GetPixel(x,y_0-1) != stopColor ) && ( GetPixel(x,y_0-1) != floodColor ) && (x>x_left) )
					x--;
			}

			x--;
		}
	}

	// ..and check lower line.
	if ( (y_0 > 0) && (y_0 < m_nHeight-1 ) )
	{
		x = x_right-1;
		while ( x>x_left )
		{
			if ( ( GetPixel(x,y_0+1) != stopColor ) && ( GetPixel(x,y_0+1) != floodColor ) )
			{
				Flood( x, y_0+1, floodColor, stopColor );

				// go left until we reach a new segment
				while ( ( GetPixel(x,y_0+1) != stopColor ) && ( GetPixel(x,y_0+1) != floodColor ) && (x>x_left) )
					x--;
			}

			x--;
		}
	}

}

// *************************************************************************

void  CVisGraphics::DrawLineOctant0( Int x0, Int y0, Int deltaX, const Int deltaY, const Int Xdirection, const Uint32 color)
{
	Int deltaYx2;
	Int deltaYx2minusdeltaXx2;
	Int errorterm;

	deltaYx2 				= deltaY*2;
	deltaYx2minusdeltaXx2	= deltaYx2 - (Int)(deltaX*2);
	errorterm 				= deltaYx2 - (Int)deltaX;

	SetPixel( x0, y0, color );
	
	while ( deltaX-- != 0 )
	{
		if (errorterm >= 0)
		{
			y0++;
			errorterm += deltaYx2minusdeltaXx2;
		}
		else
			errorterm += deltaYx2;
	
		x0 += Xdirection;

		SetPixel( x0, y0, color );
	}
}

// *************************************************************************

void  CVisGraphics::DrawLineOctant1( Int x0, Int y0, const Int deltaX, Int deltaY, const Int Xdirection, const Uint32 color)
	{

	Int deltaXx2;
	Int deltaXx2minusdeltaYx2;
	Int errorterm;

	deltaXx2				= deltaX * 2;
	deltaXx2minusdeltaYx2	= deltaXx2 - (Int)(deltaY*2);
	errorterm				= deltaXx2 - (Int)deltaY;

	SetPixel( x0, y0, color );

	while ( deltaY-- )
	{
		if (errorterm>=0)
		{
			x0			+= Xdirection;
			errorterm	+= deltaXx2minusdeltaYx2;
		}
		else
			errorterm	+= deltaXx2;
	
		y0++;

		SetPixel( x0, y0, color );
	}
}


// *************************************************************************
		
// *************************************************************************
