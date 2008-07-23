/*! @file template.h
 * @brief Global header file for the template application.
 */
#ifndef LEANXSUGUS_H_
#define LEANXSUGUS_H_

/* - Includes - */
#include "inc/oscar.h"
#include <stdio.h>

/* - Settings - */
/*! @brief The file name of the test image on the host. */
#define TEST_IMAGE_FN "test.bmp"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

/*! @brief Gets the length of a field. This does not work for pointers, which are the same as fields. */
#define length(a) ((sizeof (a)) / sizeof *(a))

#define captureWidth OSC_CAM_MAX_IMAGE_WIDTH
#define captureHeight OSC_CAM_MAX_IMAGE_HEIGHT

typedef uint16 t_index;

/* - Functions - */
/*!
 * @brief Unload everything before exiting.
 * 
 * @return SUCCESS or an appropriate error code.
 */
OSC_ERR Unload();

void processFrame_init();
void processFrame(uint8 const * const pRawImg, t_index width, t_index height);

#endif /*LEANXSUGUS_H_*/
