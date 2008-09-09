
#ifndef _CLASSVISCOLOR_H_
#define _CLASSVISCOLOR_H_

#include "classVisObject.h"

class CVisColor 
{
public:
	CVisColor();


	static void	RGBToHSL(	const Uint8 R, const Uint8 G, const Uint8 B, 
							Uint8 & H, Uint8 & S, Uint8 & L );
	static void	HSVtoRGB(	const Uint8 H, const Uint8 S, const Uint8 L,
							Uint8 & R, Uint8 & G, Uint8 & B );
};

#endif

