/*! @file support.h
 * @brief Global header file for the template application.
 * 
 * This header file includes some header files that define globally used constants and functions and defines some of them itself.
 */

#ifndef SUPPORT_H_
#define SUPPORT_H_

#include "inc/oscar.h"
#include <stdio.h>

typedef int16 t_index;
typedef uint32 t_time;

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define loop while (TRUE)

/*! @brief Gets the length of a field. This does not work for pointers, which are the same as fields. */
#define length(a) ((sizeof (a)) / sizeof *(a))

#define WIDTH_CAPTURE OSC_CAM_MAX_IMAGE_WIDTH
#define HEIGHT_CAPTURE 272 /* OSC_CAM_MAX_IMAGE_HEIGHT */

#endif /* SUPPORT_H_ */
