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

#define loop while (TRUE)

/*! @brief Gets the length of a field. This does not work for pointers, which are the same as fields. */
#define length(a) ((sizeof (a)) / sizeof *(a))

#define WIDTH_CAPTURE OSC_CAM_MAX_IMAGE_WIDTH
#define HEIGHT_CAPTURE 272 /* OSC_CAM_MAX_IMAGE_HEIGHT */

typedef int16 t_index;

/* from main.c */
OSC_ERR Unload();

/* from process_frame.c */
void processFrame_init();
void processFrame(uint8 const * const pRawImg);

/* from config.c */
void config_init();
void config_read();

/* from modbus.c */
void modbus_init();
void modbus_sendMessage(uint16 const valves);

#endif /*LEANXSUGUS_H_*/
