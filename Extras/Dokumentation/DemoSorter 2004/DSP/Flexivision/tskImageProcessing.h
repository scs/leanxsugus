
/**
* @file
* @brief The image processing and controlling task.
* @author Bernhard Mäder
*
* This is the main image processing task. It's getting images from the PPU, processing them and will be making
* the control decisions.
*
* It also provides picture queues to the main task which can be streamed to the host processor for debugging
* and monitoring reasons.
*/
#ifndef _TSKIMAGEPROCESSING_H_
#define _TSKIMAGEPROCESSING_H_

#include "drvPPU.h"
#include "libLED.h"

/**
* The image processing task function. Opens the PPU driver and starts acquiring images.
*/
void procImageProcessing_tsk_funct();


#endif

