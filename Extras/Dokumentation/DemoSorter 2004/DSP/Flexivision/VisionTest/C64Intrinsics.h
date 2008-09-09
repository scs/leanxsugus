/**
* @file
* @author Bernhard Mäder
*
* @brief Implements the C64's intrinsics for compilation on Windows environments.
*
* Implements the C64's intrinsics for compilation on Windows environments. Only those intrinsics
* are implemented that actually are used.
*/

#ifndef _C64INTRISICS_H_
#define _C64INTRISICS_H_

#include "Types.h"

Uint32 _subabs4(Uint32 x, Uint32 y);

/** 
* Implement the DOTPU4 CPU instruction.
*/
Uint32 _dotpu4 ( Uint32 n1, Uint32 n2 );

/**
* Implement the PACK2 CPU instruction.
*/
Uint32 _pack2( Uint32 n1, Uint32 n2);

Uint32 _packl4( Uint32 n1, Uint32 n2);

Uint32 _minu4( Uint32 n1, Uint32 n2 );

Uint32 _maxu4( Uint32 n1, Uint32 n2 );

#define _nassert(x) ASSERT(x)

#endif