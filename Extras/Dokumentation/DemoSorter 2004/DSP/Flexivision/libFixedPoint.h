
#ifndef _LIBFIXEDPOINT_H_
#define _LIBFIXEDPOINT_H_

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Converts an integer to a fixed point number.
 */
#define int2fp(a, fact_bits) (a <<  fact_bits)

/**
 * Converts a fixed point to an integer number.
 */
#define fp2int(a, fact_bits) (a >> fact_bits)

/**
 * Converts a fixed point number to a float. 
 */
#define fp2float(x, fact_bits) (((float)(x)) * ( 1.0 / (float)(1 << fact_bits) ) )

/**
 * Converts a float to a fixed point number.
 */
#define float2fp(x, fact_bits) ((Int32) ((x) * (1 << fact_bits)))


#ifdef __cplusplus
}
#endif


#endif
