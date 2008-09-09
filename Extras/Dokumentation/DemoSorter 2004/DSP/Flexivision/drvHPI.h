
/**
* @file
* @brief The HPI driver establishes and handles communication of the DSP to and from the etrax host.
* @author Bernhard Mäder
* 
* The message passing is done through a mailbox (which has nothing to do with a DSP/BIOS mailbox!), which is a shared memory section in the DSP's 
* memory to which the host is allowed to write. 
*
* The driver's structure consist of the following:
*
* - 	Two bufferqueues for transporting buffer to and from the etrax.
*
* - 	An SWI task which is triggered by the DSPINT interrupt, with which the host signals that
*   	a new packet is available and ready to be read in from the mailbox.
*
* -		A TSK task which handles the output packets.
*
* The driver supports both blocking and non-blocking packet reception. Sending packets is non-blocking
* if the output queue is not full.
*/

#ifndef _DRVHPI_H_
#define _DRVHPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>
#include <que.h>
#include <log.h>
#include <tsk.h>
#include <mbx.h>

#include "FlexiVisioncfg.h"

#include <csl_irq.h>
#include <csl_hpi.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "drvHPI_Packet.h"
#include "libBufferQueue.h"
#include "libPicture.h"

#ifdef _DEBUG
#include "libLED.h"
#endif

/** The etrax driver's maximum queue sizes. Defines the maximum size of both, the incoming and the outgoing queues. */
#define			HPI_MAXQUEUESIZE			8

/**
* The number of system ticks the writer task waits between two checks on the address fields of the out packet in
* the mailbox. This sets the polling frequency.
*/
#define			HPI_OUT_POLLDELAY			1

/**
* Timeout delay when waiting for the etrax to clear the outgoing mailbox. If the driver has to wait longer than
* that, it will drop the packet and try the next one.
*/
#define			HPI_OUT_TIMEOUT				10000

/** Defines the size of a packet buffer in the mailbox as well as in the in/out queues. */
#define			HPI_PACKETBUFFER_SIZE		(6 * 1024 * 1024)

/**
* The etrax device driver object.
*/
typedef struct HPI_DevObject
{
	BufferQueue			QueueIn;							///< The input queue.
	BufferQueue_Frame	QueueIn_frames[HPI_MAXQUEUESIZE];	///< The frames that are used for the input queue.
	
	BufferQueue			QueueOut;							///< The output queue.
	BufferQueue_Frame	QueueOut_frames[HPI_MAXQUEUESIZE];	///< The frames that are used for the output queue.
	
} HPI_DevObject, * HPI_DevHandle;

/**
* Opens the etrax driver.
*
* @param	queuesize	The size of the incoming and outgoing message queue (#of frames).
* @return				A valid handle to the etrax device, NULL if failed.
*/
HPI_DevHandle	hpiOpen(Int queuesize);


/**
* Closes the etrax driver.
*
* @param	device		A handle to the device.
* @return				SYS_OK or error specifier.
*/
Int 			hpiClose(HPI_DevHandle device);


/**
* Gets a message from the incoming queue and blocks if there is
* none (with the timeout value specified). The buffer
* provided here must be big enough to hold a message of
* HPI_Packet_SIZE.
*
* @param	device		A handle to the device.
* @retval	packet		A pointer to a buffer of size HPI_Packet_SIZE, where the incoming packet is written to.
* @param	timeout		A timeout value in system ticks (set by config utility), SYS_FOREVER or SYS_POLL (i.e. don't wait at all).
* @return				SYS_OK, SYS_ETIMEOUT or error specifier.
*/
Int		hpiGetMessage(HPI_DevHandle device, HPI_Packet * packet, Uns timeout);

/**
* Checks whether the output queue is free so that a call to one of the the send functions
* would return TRUE at once.
*/
Bool	hpiIsOutputQueueFree( HPI_DevHandle device );

/**
* Sends a message to the etrax. This call is non-blocking
* if the outgoing queue is not empty. It will block if the queue
* is full and wait until one of the packets is sent. You can 
* specify the timeout of the blocking. The specified buffer
* must hold the packet data, which is copied when calling the function, so
* the caller can dispose the buffer or use it otherwise.
*
* @param	device		A handle to the device.
* @param	packet		A pointer to a buffer of size HPI_Packet_SIZE, where the packet's content is read from.
* @param	timeout		A timeout value in system ticks (set by config utility), SYS_FOREVER or SYS_POLL (i.e. don't wait at all).
* @return				SYS_OK, SYS_ETIMEOUT or error specifier.
*/
Int		hpiSendMessage(HPI_DevHandle device, HPI_Packet * packet, Uns timeout);

/**
* Sends a picture to the etrax. This call behaves exactly like hpiSendMessage, except
* that it'll put the packet together on its own. The benefit of this method is that the
* picture data can directly be copied to the message buffer. So, the caller can dispose the
* picture after calling the function or use it otherwise.
*
* @param	device		A handle to the device.
* @param	pic			A picture handle. The picture's content must be correctly filled (pic->TotalSize especially).
* @param	channel		The channel on which the picture should be sent.
* @param	srcaddr		The source address.
* @param	dstaddr		The destination address.
* @param	timeout		A timeout value in system ticks (set by config utility), SYS_FOREVER or SYS_POLL (i.e. don't wait at all).
* @return				SYS_OK, SYS_ETIMEOUT or error specifier.
*/
Int		hpiSendPicture(HPI_DevHandle device, PictureHandle pic, Uint32 channel, Uint32 srcaddr, Uint32 dstaddr, Uns timeout);

/**
* Sends a text message to the etrax. The text is copied to a text packet and the buffer
* may be used otherwise after calling the function.
*
* @param	device		A handle to the device.
* @param	str			String buffer containing the test to be sent.
* @param	srcaddr		The source address.
* @param	dstaddr		The destination address.
* @param	timeout		A timeout value in system ticks (set by config utility), SYS_FOREVER or SYS_POLL (i.e. don't wait at all).
* @return				SYS_OK, SYS_ETIMEOUT or error specifier.
*/
Int		hpiSendText(HPI_DevHandle device, const Char * str, Uint32 srcaddr, Uint32 dstaddr, Uns timeout);


/**
* Gets the number of messages on the incoming queue.
*
* @param	device		A handle to the device.
* @return				The number of messages that can be read by hpiGetMessage()
*/
Int		hpiGetMessageCount(HPI_DevHandle device);

#ifdef __cplusplus
}
#endif

#endif
