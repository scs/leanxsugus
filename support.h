/*! @file support.h
 * @brief Global header file for the template application.
 * 
 * This header file includes some header files that define globally used constants and functions and defines some of them itself.
 */

#ifndef SUPPORT_H_
#define SUPPORT_H_

#include <stdio.h>
#include "oscar/staging/inc/oscar.h"

/*! @brief A type used for indexing stuff, like in loops. */
typedef int16 t_index;
/*! @brief A type used to measure time. */
typedef uint32 t_time;

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define loop while (true)

/*! @brief Gets the length of a field. This does not work for pointers, which are the same as fields. */
#define length(a) ((sizeof (a)) / sizeof *(a))

#ifdef ASSERTS_ENABLE
/*! @brief Checks wether the given expression hold true. If this is not the case a warning is printed. */
#define assert(a) if (!(a)) printf("%s: %s: Line %d: Assertion failed: %s\n", __FILE__, __func__, __LINE__, #a)
#else /* ASSERTS_ENABLE */
#define assert(a)
#endif /* ASSERTS_ENABLE */

#ifdef MARKERS_ENABLE
#define m printf("%s: Line %d\n", __func__, __LINE__);
#define p(n) printf("%s: Line %d: %s: %d\n", __func__, __LINE__ , #n, n);
#else /* ASSERTS_ENABLE */
#define m
#define p(n)
#endif /* MARKERS_ENABLE */

#ifdef BENCHMARK_ENABLE
uint32 benchmark_cyc;
#define benchmark_init benchmark_cyc = OscSupCycGet()
#define benchmark_delta { t_time cyc = OscSupCycGet(); printf("Line %d: %lu us\n", __LINE__, OscSupCycToMicroSecs(cyc - benchmark_cyc)); benchmark_cyc = cyc; }
#else /* BENCHMARK_ENABLE */
#define benchmark_init
#define benchmark_delta
#endif /* BENCHMARK_ENABLE */

/*! @brief Width of the image to be captured. */
#define WIDTH_CAPTURE OSC_CAM_MAX_IMAGE_WIDTH
/*! @brief height of the image to be captured. */
#define HEIGHT_CAPTURE 272

#endif /* SUPPORT_H_ */
