
/**
* @file 
*
* @brief The main control task.
* @author Bernhard Mäder
*
* This is the DSP's main control task. It handles communication to and from the etrax host processor.
* It also handles the image streaming to the etrax.
*/


/**
* @ctrlpage FlexiVision DSP Software
*
*/

#ifndef _TSKCONTROL_H_
#define _TSKCONTROL_H_

#include <std.h>
#include <tsk.h>
#include <sio.h>

#include "FlexiVisioncfg.h"

#include "drvHPI_Packet.h"
#include "drvHPI.h"
#include "drvPPU.h"
#include "libPicture.h"
#include "libBufferQueue.h"
#include "tskImageProcessing.h"

#include <stdio.h>
#include <string.h>

/**
* The dsp's ctrl application task.
*/
void ctrlApplication_tsk_funct();


#endif

