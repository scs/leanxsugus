
#include "libPicture.h"

#include <stdio.h>

// *************************************************************************

Uint32	picGetType( Int bpp, Bool bIndexed )
{
	Uint32 res = 0;
	
	switch (bpp)
	{
		case 1:
			res = PICT_GREY1;
			break;
			
		case 2:
			if ( bIndexed )
				res = PICT_INDEX2;
			else
				res = PICT_GREY2;
			break;
			
		case 4:
			if ( bIndexed )
				res = PICT_INDEX4;
			else
				res = PICT_GREY4;
			break;
			
		case 8:
			if ( bIndexed )
				res = PICT_INDEX8;
			else
				res = PICT_GREY8;
			break;
			
		case 15:
			res = PICT_RGB555;
			break;	
			
		case 16:
			res = PICT_RGB565;
			break;
			
		case 24:
			res = PICT_RGB888;
			break;
			
		case 32:
			res = PICT_RGBX8888;
			break;
	}
	
	return res;
}

// *************************************************************************

Uint32 picGetPictureSize( Uint32 unPicType, Uint32 unWidth, Uint32 unHeight )
{
	Uint32 bpp;
	Uint32 size;
	
	bpp 	= picGetBPP( unPicType );	
	size 	= (unWidth*unHeight*bpp + 7) / 8 + PIC_HEADERSIZE;
	
	return size;
}

// *************************************************************************


Bool	picCreate( PictureHandle * hhpic, Int width, Int height, Int bpp, Int segID)
{
	void 	* buffer;
	Int		size;
	
	size =  (width*height*bpp + 7) / 8 + PIC_HEADERSIZE;
	
	buffer = MEM_alloc( segID, size, 0);
	
	if ( buffer == NULL)
	{
		printf("Error creating image.\n");
		return FALSE;
	}
	else
	{
		*hhpic = buffer;
		
		(*hhpic)->TotalSize = size;
		(*hhpic)->TotalWidth = width;
		(*hhpic)->TotalHeight = height;
		(*hhpic)->OffsetX = 0;
		(*hhpic)->OffsetY = 0;
		(*hhpic)->Width = width;
		(*hhpic)->Height = height;
		(*hhpic)->Type = picGetType( bpp, FALSE );
		return TRUE;
	}
}

// *************************************************************************

Int				picGetBPP( Uint32 picType )
{
	switch (picType)
	{
		case PICT_GREY1:
			return 1;

		case PICT_GREY2:
		case PICT_INDEX2:
			return 2;
		
		case PICT_GREY4:
		case PICT_INDEX4:
			return 4;
		
		case PICT_GREY8:
		case PICT_INDEX8:
			return 8;
		
		case PICT_RGB888:
			return 24;
				
		case PICT_RGB555:			
			return 16;
		
		case PICT_RGB565:
			return 16;
			
		case PICT_RGBX8888:
			return 32;
			
		case PICT_DATA:
			return 8;
		
		default:
			return 0;
	}
}

// *************************************************************************

Int				picGetBufferSize( PictureHandle hpic )
{
	// Calculate the buffer size and round up to multiple of 4.
	return ((hpic->Width * hpic->Height * picGetBPP( hpic->Type ) + 31 ) / 8) & ~0x07;
}

// *************************************************************************

void			picSetPixel( PictureHandle hpic, Int x, Int y, PIC_COLORREF value)
{
	Int pixel;
	
	pixel = x + y*hpic->Width;
	
	switch (hpic->Type )
	{
		case PICT_GREY1:
			hpic->Data[ pixel/8 ] = hpic->Data[ pixel/8 ] & ~(0x01 << (x & 0x07));		// mask out previous pixel
			if (value)
				hpic->Data[ pixel/8 ] = hpic->Data[ pixel/8 ] | (0x01 << (x & 0x07));	// set new pixel
			break;
			
		case PICT_GREY2:
			hpic->Data[ pixel/4 ] = hpic->Data[ pixel/4 ] & ~(0x03 << ((x & 0x03)*2));
			hpic->Data[ pixel/4 ] = hpic->Data[ pixel/4 ] | (((value & 0xC0) >> 6) << (x & 0x03)*2);
			break;
		
		case PICT_GREY4:
			hpic->Data[ pixel/2 ] = hpic->Data[ pixel/2 ] & ~(0x0F << ((x & 0x01)*4) );
			hpic->Data[ pixel/2 ] = hpic->Data[ pixel/2 ] | (((value & 0xF0) >> 4) << (x & 0x01)*4);
			break;
		
		case PICT_GREY8:
			hpic->Data[ pixel ] = value & 0xFF;
			break;
		
		case PICT_RGB888:
			hpic->Data[ pixel*3 ] = PIC_RED(value);
			hpic->Data[ pixel*3+1 ] = PIC_BLUE(value);
			hpic->Data[ pixel*3+2 ] = PIC_GREEN(value);
			break;
				
		case PICT_RGB555:			
			hpic->Data[ pixel*2 ]	= ( ((PIC_RED(value) & 0xF8) >> 1)  | ((PIC_BLUE(value) & 0xC0) >> 6));
			hpic->Data[ pixel*2 + 1]= ( ((PIC_BLUE(value) & 0x38) << 5) | ((PIC_GREEN(value) & 0xF8) >> 3));
			break;
		
		case PICT_RGB565:
			hpic->Data[ pixel*2 ]	= ( ((PIC_RED(value) & 0xF8))  		| ((PIC_BLUE(value) & 0xE0) >> 5));
			hpic->Data[ pixel*2 + 1]= ( ((PIC_BLUE(value) & 0x1C) << 5) | ((PIC_GREEN(value) & 0xF8) >> 3));
			break;
			
		case PICT_RGBX8888:
			hpic->Data[ pixel*4 ] = PIC_RED(value);
			hpic->Data[ pixel*4+1 ] = PIC_GREEN(value);
			hpic->Data[ pixel*4+2 ] = PIC_BLUE(value);
			break;
		
		default:
			break;
	}
}

// *************************************************************************

PIC_COLORREF 	picGetPixel( PictureHandle hpic, Int x, Int y)
{
	Int pixel;
	
	pixel = x + y*hpic->Width;
	
	switch (hpic->Type )
	{
		case PICT_GREY1:
			return PIC_BW( (hpic->Data[ pixel / 8] & (0x01 << (x & 0x0F))) >> (x & 0x0F) );
			
		case PICT_GREY2:
			return PIC_GREY(  (hpic->Data[ pixel / 4] & (0x03 << ((x & 0x03)*2))) >> ((x & 0x03)*2) << 6);
		
		case PICT_GREY4:
			return PIC_GREY(  (hpic->Data[ pixel / 2] & (0x0F << ((x & 0x01)*4))) >> ((x & 0x01)*4) << 4);
		
		case PICT_GREY8:
			return PIC_GREY( hpic->Data[pixel]);
		
		case PICT_RGB888:
			return PIC_RGB( hpic->Data[pixel*3], hpic->Data[pixel*3+1], hpic->Data[pixel*3+2]);
				
		case PICT_RGB555:			
				// TODO:
			return 0;
		
		case PICT_RGB565:
			// TODO:
			return 0;			
		
		default:
			return 0;
			
	}
}

// *************************************************************************

void picLineOctant0( PictureHandle hpic, Int x0, Int y0, Int deltaX, Int deltaY, Int Xdirection, PIC_COLORREF color)
{
	Int deltaYx2;
	Int deltaYx2minusdeltaXx2;
	Int errorterm;

	deltaYx2 				= deltaY*2;
	deltaYx2minusdeltaXx2	= deltaYx2 - (Int)(deltaX*2);
	errorterm 				= deltaYx2 - (Int)deltaX;

	picSetPixel( hpic, x0,y0,color );
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
		picSetPixel( hpic, x0,y0,color );
	}
}

// *************************************************************************

void picLineOctant1( PictureHandle hpic, Int x0, Int y0, Int deltaX,
			  Int deltaY, Int Xdirection, PIC_COLORREF color)
	{

	Int deltaXx2;
	Int deltaXx2minusdeltaYx2;
	Int errorterm;

	deltaXx2				= deltaX * 2;
	deltaXx2minusdeltaYx2	= deltaXx2 - (Int)(deltaY*2);
	errorterm				= deltaXx2 - (Int)deltaY;

	picSetPixel( hpic, x0, y0, color );
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
		picSetPixel( hpic, x0, y0, color );
	}
}


// *************************************************************************

#define SWAP(a,b, temp)	temp=a; a=b; b=temp;

void picDrawLine(PictureHandle pic, Int x0, Int y0, Int x1, Int y1, PIC_COLORREF color)
{
	Int deltaX, deltaY;
	Int temp;
	
	// y0 has to be smalle or equal y1, swap points if necessary.
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
			picLineOctant0( pic, x0, y0, deltaX, deltaY, 1, color);
		else
			picLineOctant1( pic, x0, y0, deltaX, deltaY, 1, color);
	}
	else
	{
		// invert deltaX
		deltaX= -deltaX;
		
		// 2 more cases possible: dy is bigger than dx and vice versa. We have to use 
		// a particular function for each of those.
		if (deltaX > deltaY)
			picLineOctant0( pic, x0, y0, deltaX, deltaY, -1, color);
		else
			picLineOctant1( pic, x0, y0, deltaX, deltaY, -1, color);
	}
}

// *************************************************************************

