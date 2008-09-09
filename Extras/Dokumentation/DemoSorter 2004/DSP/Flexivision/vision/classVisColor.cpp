
#include "classVisColor.h"

#define  HSLMAX   252			/* H,L, and S vary over 0-HSLMAX */
#define  RGBMAX   255			/* R,G, and B vary over 0-RGBMAX */
								/* HSLMAX BEST IF DIVISIBLE BY 6 */
								/* RGBMAX, HSLMAX must each fit in a Uint8. */
#define UNDEFINED (HSLMAX*2/3)	/* Hue is undefined if Saturation is 0 (grey-scale) */

CVisColor::CVisColor()
{
}

// *************************************************************************

void CVisColor::RGBToHSL( const Uint8 R, const Uint8 G, const Uint8 B, 
									Uint8 & H, Uint8 & S, Uint8 & L )
{
	Uint32 	cMax, cMin;
	Uint16 	Rdelta, Gdelta, Bdelta;

	cMax = (Uint32)max( max(R,G), B);	/* calculate lightness */
	cMin = (Uint32)min( min(R,G), B);
	L = (Uint8)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	// r=g=b --> achromatic case, i.e. S and H are undefined
	if (cMax==cMin)
	{			
		S = 0;					
		H = UNDEFINED;
	} 
	// Chromatic cases:
	else 		
	{		
		// Determine Saturation			
		if (L <= (HSLMAX/2))	
			S = (Uint8)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (Uint8)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
			
		// Now, determine Hue
		Rdelta = (Uint16)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (Uint16)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (Uint16)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (Uint8)(Bdelta - Gdelta);
			
		else if (G == cMax)
			H = (Uint8)((HSLMAX/3) + Rdelta - Bdelta);
			
		else // B == cMax 
			H = (Uint8)(((2*HSLMAX)/3) + Gdelta - Rdelta);

//		if (H < 0) H += HSLMAX;     //always false
		if (H > HSLMAX) H -= HSLMAX;
	}
}
		
// *************************************************************************

void CVisColor::HSVtoRGB( const Uint8 H, const Uint8 S, const Uint8 L,
						 Uint8 & R, Uint8 & G, Uint8 & B )
{
	Int sector;
	Int fact;
	Int p,q,t;
	Int h = H;

	// achromatic case
	if ( S == 0 )
	{
		R = G = B = L;
	}
	// cromatic case
	else
	{
	
    if ( h == 360 ) 
		h = 0; 
    		
	sector = h / (HSLMAX/6);		// sector 0 to 5
	fact = 6*(h - sector*(HSLMAX/6));	// factorial part of h

	p = ( (Int)L * ( RGBMAX - (Int)S ) )									/ RGBMAX;
	q = ( (Int)L * ( RGBMAX - ((Int)S * fact)				/ RGBMAX ) )	/ RGBMAX;
	t = ( (Int)L * ( RGBMAX - ((Int)S * ( RGBMAX - fact ))	/ RGBMAX ) )	/ RGBMAX;
		
	switch( sector ) 
	{
		case 0:
			R = L;
			G = t;
			B = p;
			break;
		case 1:
			R = q;
			G = L;
			B = p;
			break;
		case 2:
			R = p;
			G = L;
			B = t;
			break;
		case 3:
			R = p;
			G = q;
			B = L;
			break;
		case 4:
			R = t;
			G = p;
			B = L;
			break;
		default:// case 5:
			R = L;
			G = p;
			B = q;
			break;
		}
		
	}
}

// *************************************************************************

/*
	void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
	{
		int i;
		float f, p, q, t;
		
		if( s == 0 ) {
			// achromatic (grey)
			*r = *g = *b = v;
			return;
		}
		
		h /= 60;// sector 0 to 5
		i = floor( h );
		f = h - i;// factorial part of h
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );
		
		switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
			case 2
				*r = p;
				*g = v;
				*b = t;
				break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
		}
		
	}
	*/
	
	