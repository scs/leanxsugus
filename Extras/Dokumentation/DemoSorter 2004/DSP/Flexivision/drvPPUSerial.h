/**
* @file
* @brief 
* @author Bernhard Mäder
*/

#ifndef _DRVPPUSERIAL_H_
#define _DRVPPUSERIAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>
#include <sem.h>

#include "FlexiVisioncfg.h"

#include <csl_irq.h>
#include <stdio.h>

#include "libRingBuffer.h"

/**
* Defines the number of serial channels supported by this driver.
*/
#define PPUSER_NUMCHANNELS 3

/**
* The serial channel objects. Collects all the data needed for one serial line to the PPU.
*
* Note: the bTxRegisterEmpty flag is needed because we can't directly read the tx register's
* 		state in the PPU. That and the fact that our routine may be triggered by either a software
*		call or the hardware interrupt and must thus be able to know the register's state in absence
*		of any pending hwi make that flag necessary. 
*/
typedef struct PPUSER_DevObject {

	RingBuffer			SerialTx;							///< The serial driver's Tx ringbuffer.
	RingBuffer			SerialRx;							///< The serial driver's Rx ringbuffer.
	
	Uint8				unChannelNum;
} PPUSER_DevObject, * PPUSER_DevHandle;

/**
* The PPU serial driver object.
*/
typedef struct PPUSER_Object
{
	Bool				aryChannelOpened[PPUSER_NUMCHANNELS];
	PPUSER_DevObject	aryChannels[PPUSER_NUMCHANNELS];
} PPUSER_Object;

/**
* Opens the serial device driver and returns a handler to the device object.
*/
PPUSER_DevHandle	serOpen( Int nChannel, Int bufsize );

/**
* Closes the serial device driver.
*/
void				serClose( PPUSER_DevHandle device );

/**
* Writes to the serial channel to the camera. It writes the bytes from the buffer to the driver's fifo, provided that
* the fifo isn't full yet. The function will return the number of bytes actually written.
*
* @param	device		The handle to the device.
* @param	buffer		A buffer containing the data.
* @param	count		The number of bytes in the buffer that should be written.
* @return				The number of bytes actually written to the serial fifo in the ppu.
*/
Int					serWrite( PPUSER_DevHandle device , const char * buffer, const Int count);


/**
* Reads from the serial channel input fifo. The function tries to read as many bytes as are
* in the driver's input fifo, IF the buffer provided is big enough. It might as well wait for input, using
* the specified timeout value.
*
* @param	device		The handle to the device.
* @param	buffer		A buffer to which the data is written to.
* @param	count		The number of bytes allocated in the buffer. This is the maximum amount of data that is read.
* @return				The number of bytes actually read from the ppu's serial fifo.
*/
Int					serRead( PPUSER_DevHandle device, char * buffer, const Int count);

/**
* Flushes the RX queue.
*/
void				serFlushRx( PPUSER_DevHandle device );

/**
* Flushes the TX queue.
*/
void				serFlushTx( PPUSER_DevHandle device );

int					serLevelTx( PPUSER_DevHandle device );

int					serLevelRx( PPUSER_DevHandle device );


/**
* Configures one UART channel's speed.
*/
Bool				serConfigChannel(PPUSER_DevHandle device, const Int speed, const Bool bTwoStopBits, const Bool bParity, const Bool bEvenParity );

/**
* Enables HW-Loopback of the UART.
*/
void				serLoopback( PPUSER_DevHandle device, Bool bLoop );

#ifdef __cplusplus
}
#endif

#endif

