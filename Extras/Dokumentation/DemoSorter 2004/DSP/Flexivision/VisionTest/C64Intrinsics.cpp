
#include "C64Intrinsics.h"

#define BYTE0(x) ((Uint8)((Uint32)x & 0x000000FF))
#define BYTE1(x) ((Uint8)(((Uint32)x & 0x0000FF00) >> 8))
#define BYTE2(x) ((Uint8)(((Uint32)x & 0x00FF0000) >> 16))
#define BYTE3(x) ((Uint8)(((Uint32)x & 0xFF000000) >> 24))

#define LSB16(x) ((Uint32)x & 0x0000FFFF)
#define MSB16(x) (((Uint32)x & 0xFFFF0000) >> 16)

#define DWORD16( w0, w1 ) ( (w0) | (w1<<16) )
#define DWORD8( b0, b1, b2, b3) ( (b0) | (b1<<8) | (b2<<16) | (b3<<24))

#ifndef min
#define min(a,b)	( a<b ? a : b )
#endif

#ifndef max
#define max(a,b)	( a>b ? a : b )
#endif

// *************************************************************************

Uint32 _subabs4(Uint32 x, Uint32 y)
{
	return x-y;
}

// *************************************************************************

Uint32 _dotpu4 ( Uint32 n1, Uint32 n2 )
{
	Uint32 p1, p2, p3, p4;

	p1 = (n1 & 0x000000FF) * (n2 & 0xFF);
	p2 = ( (n1 & 0x0000FF00) >> 8) * ( (n2 & 0x0000FF00) >> 8 );
	p3 = ( (n1 & 0x00FF0000) >> 16) * ( (n2 & 0x00FF0000) >> 16 );
	p4 = ( (n1 & 0xFF000000) >> 24) * ( (n2 & 0xFF000000) >> 24 );

	return p1+p2+p3+p4;
}


// *************************************************************************

Uint32 _pack2( Uint32 n1, Uint32 n2)
{
	//return ( ( n1 & 0x0000FFFF ) << 16 ) | ( n2 & 0x0000FFFF );
	return DWORD16( LSB16(n2), LSB16(n1) );
}


// *************************************************************************

Uint32 _packl4( Uint32 n1, Uint32 n2)
{
	return DWORD8( BYTE0(n2), BYTE2(n2), BYTE0(n1), BYTE2(n1) );
}

// *************************************************************************

Uint32 _minu4( Uint32 n1, Uint32 n2 )
{
	return DWORD8(	min( BYTE0(n1), BYTE0(n2) ),
					min( BYTE1(n1), BYTE1(n2) ),
					min( BYTE2(n1), BYTE2(n2) ),
					min( BYTE3(n1), BYTE3(n2) ) );
}

// *************************************************************************

Uint32 _maxu4( Uint32 n1, Uint32 n2 )
{
	return DWORD8(	max( BYTE0(n1), BYTE0(n2) ),
					max( BYTE1(n1), BYTE1(n2) ),
					max( BYTE2(n1), BYTE2(n2) ),
					max( BYTE3(n1), BYTE3(n2) ) );
}
