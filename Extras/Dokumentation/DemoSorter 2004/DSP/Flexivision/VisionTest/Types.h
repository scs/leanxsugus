/**
* @file
* @author Bernhard Mäder
*
* @brief Defines the DSP/BIOS' data types for compilation on Windows.
*
* Defines the DSP/BIOS' data types for compilation on Windows.
*/

#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef _WINDOWS
/** Define the restrict keyword, which is only used in the TI compiler. */
#define restrict
#endif

#define FALSE 0
#define TRUE  1

typedef int				Bool;
typedef void *			Ptr;
typedef int				Int;
typedef int				Int32;
typedef unsigned int	Uint32;
typedef short			Int16;
typedef unsigned short	Uint16;
typedef char			Int8;
typedef unsigned char	Uint8;
typedef char			Char;
typedef float			Float;


#define NULL 0


#endif
