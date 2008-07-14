#ifndef PROCESS_FRAME_H_
#define PROCESS_FRAME_H_

#include "template.h"

/*********************************************************************//*!
 * @brief Process a newly captured frame.
 * 
 * In the case of this template, this consists just of debayering the
 * image and writing the result to the result image buffer. This should 
 * be the starting point where you add your code.
 * 
 * @param pRawImg The raw image to process.
 *//*********************************************************************/

void process_frame_init();
void ProcessFrame(uint8 const * const pRawImg);

#endif /* PROCESS_FRAME_H_ */
