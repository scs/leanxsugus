
/**
* @file
* @brief The picture library that's used for all images on the DSP.
* @author Bernhard Mäder
*
* It can handle creation and destruction of pictures and allows some basic operations like line drawing,
* pixel set and pixel get. It also defines a picture structure that can store all information needed
* to pass images on to the etrax host, including windowing for area-of-interest applications.
*/

#ifndef _LIBPICTURE_H_
#define _LIBPICTURE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "FlexiVisioncfg.h"

//#include <stdlib.h>

#define PICT_GREY1		0x00
#define PICT_GREY2		0x01
#define PICT_GREY4		0x02
#define PICT_GREY8		0x03
#define PICT_RGB888		0x04
#define PICT_RGBX8888	0x05
#define PICT_RGB555		0x06
#define PICT_RGB565		0x07
#define PICT_INDEX2		0x08
#define PICT_INDEX4		0x09
#define PICT_INDEX8		0x0A


/**
* This is a special picture type that allows the transmission of binary data to the host.
* The data is embedded in a picture of size (buffersize x 1), using 8 bits per pixel.
* The data on pic->Data[] may be of arbitrary type and must be handled by the connected
* client.
*/
#define PICT_DATA	0xFF

/**
* Defines the picture data structure. 
* This structure is used to transport data from the driver
* to the user program (actually, unstructured buffers are used, but Picture can be used to 
* retrieve information about the picture from that buffers).
*/
typedef struct Picture
{
	Uint32			TotalSize;				///< The total size of the picture in bytes, including the header.
	Uint16			TotalWidth;				///< The width of the image.
	Uint16			TotalHeight;			///< The height of the image. 1 if this is only a line.
	Uint16			OffsetX;				///< The sub window's offset's x-coordinate. Zero if there's no sub-window.
	Uint16			OffsetY;				///< The sub window's offset's y-coordinate. Zero if there's no sub-window.
	Uint16			Width;					///< The sub window's width. Equals TotalWidth if there's not sub window.
	Uint16			Height;					///< The sub window's height. Equals TotalHeight if there's not sub window.
	Uint32			Type;					///< Bits per pixel etc...
	Uint32			Timestamp;				///< A timestamp of the image that can be used by the DSP to associate a certain time to the image.
	Uint8			Data[1];				///< The bitmap data. Data points to a buffer of size (Width*Height*BPP)/8.
} Picture, * PictureHandle;

#define PIC_HEADERSIZE 24

typedef Uint32	PIC_COLORREF;				///< This type is used for all functions that take a color as arguement / return value.

/** Return a PIC_COLORREF that corresponds to the entered R, G and B values. */
#define	PIC_RGB(r,g,b)	(PIC_COLORREF)((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF))
/** Return a PIC_COLORREF that corresponds to the entered greyscale value */
#define PIC_GREY(g)		(PIC_COLORREF)((((g) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((g) & 0xFF))
/** Return a PIC_COLORREF that corresponds either black (if g is 0) or white (if g is not 0)*/
#define PIC_BW(g)		(PIC_COLORREF)( (g != 0) ? 0xFFFFFF : 0x000000 )

/** Returns a PIC_COLORREF's red component. */
#define PIC_RED(cr)		(Int)((cr & 0xFF0000) >> 16)
/** Returns a PIC_COLORREF's green component. */
#define PIC_GREEN(cr)	(Int)((cr & 0x00FF00) >> 8)
/** Returns a PIC_COLORREF's blue component. */
#define PIC_BLUE(cr)	(Int)((cr & 0x0000FF))


/**
* Gets the value for the type field of the picture.
*
* @param	bpp			Number of bits per pixel.
* @param	bIndexed	Defines whether the image is indexed or not. This does not yet affect anything.
* @return				The bit field value for the picture's type field.
*/
Uint32			picGetType( int bpp, Bool bIndexed );

Uint32			picGetPictureSize( Uint32 unPicType, Uint32 unWidth, Uint32 unHeight );


/**
* Allocates and creates a picture in memory. Initializes all of the picture's fields with the
* appropriate values (width, height etc.).
*
* @retval	hhpic	Pointer to a picture handle. The picture pointer will be written here.
* @param	width	The picture's width.
* @param	height	The picture's height.
* @param	bpp		Bits per pixel.
* @param	segID	The segment ID of the memory space in which the picture should be allocated.
* @return			TRUE if succesful, FALSE otherwise (memory couldn't be allocated).
*/
Bool			picCreate( PictureHandle * hhpic, Int width, Int height, Int Bpp, int segID);

/**
* Gets the number of bits per pixel of an image. The number is the number of pixels actually stored in
* the buffer, so the return value for an RGB555 image is 16 as well.
*
* @param	picType	The picture type.
* @return			The number of bits per pixel.
*/
Int				picGetBPP( Uint32 picType );

/**
* Gets the size of the pixel buffer belonging to an image.
*
* @param	hpic	The picture.
* @return			The size of the pixture's pixel buffer.
*/
Int				picGetBufferSize( PictureHandle hpic);

/**
* Sets a pixel in the image.
*
* @param	hpic		Pointer to the picture.
* @param	x			The x-coordinate of the pixel.
* @param	y			The y-coordinate of the pixel.
* @param	value		The value to which the pixel should be set.
*/
void			picSetPixel( PictureHandle hpic, Int x, Int y, PIC_COLORREF value);

/**
* Gets a pixel in the image.
*
* @param	hpic		Pointer to the picture.
* @param	x			The x-coordinate of the pixel.
* @param	y			The y-coordinate of the pixel.
* @return 	value		The value of the pixel.
*/
PIC_COLORREF 	picGetPixel( PictureHandle hpic, Int x, Int y);

/**
* Draws a line in the picture. Define the start and the end point and the color.
*
* @param	hpic		Pointer to the picture.
* @param	x0			x coordinate of the start point.
* @param	y0			y coordinate of the start point.
* @param	x1			x coordinate of the end point.
* @param	y1			y coordinate of the end point.
* @param	color		The line's color.
*/
void 			picDrawLine(PictureHandle pic, Int x0, Int y0, Int x1, Int y1, PIC_COLORREF color);


#ifdef __cplusplus
}
#endif

#endif
