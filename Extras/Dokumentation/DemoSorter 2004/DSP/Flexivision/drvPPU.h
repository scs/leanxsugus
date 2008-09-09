/**
* @file
* @brief The ppu establishes and handles communication to and from the Picture Processing Unit (PPU).
* @author Bernhard Mäder
*
* The PPU driver handles data transfer from and to the PPU. Since the PPU is connected as programmable
* synchronous memory to the DSP, writing single registers is easily mapped through by the driver.
* 
* Acquisition of streaming image data is a bit tougher and makes heavy use of the C64's EDMA channels.
* Data transfer is triggered frame-wise, the frame's size must be set at compile time at the PPU as well
* as in the DSP software (although that could be done in software as well... but it doesn't make sense
* to change the framesize dynamically). Since the last frame of a picture might not be of the same size
* as the preceeding frames, two EDMA channels are used which are linked together.
*
* Finishing of the EDMA transfer is handled through the libEDMAManager functions, which allow the PPU
* driver to register an SWI at a certain EDMA completion event. This SWI is triggered each time a whole 
* picture could be transfered.
*
* After each picture the driver checks if it's still synchronous to the PPU. If not, the driver flushes
* the PPU and waits for a new picture.
*
* The driver also supports serial data transfer directly to the camera though the PPU.
*/

#ifndef _DRVPPU_H_
#define _DRVPPU_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>
#include <que.h>
#include <log.h>
#include <sem.h>

#include "FlexiVisioncfg.h"

#include <csl_irq.h>
#include <csl_edma.h>
#include <stdio.h>

#include "libBufferQueue.h"
#include "libRingBuffer.h"
#include "libEDMAManager.h"
#include "libPicture.h"
#include "libLED.h"

#define PPU_MAXQUEUESIZE		8		///< The maximum number of buffers in the channels' queues.
#define PPU_MAXCHANNELS			3		///< The maximum number of picture channels from the PPU.

#define PPU_NUMSERIAL			3		///< The maximum number of serial UARTS from the PPU.
#define PPU_SERIALFIFOSIZE		256		///< The serial fifo's size in number of characters.
#define PPU_SERIALINTERVAL		10		///< The interval with which the serial driver will poll the ppu for new data.

/**
* The identifiers for the IOCTL command.
*/
enum PPU_IOCTL_Types
{
	PPU_IOCTL_TRIGGER_IMG_ACQUISITION,
	PPU_IOCTL_SET_READY_LINE,
	PPU_IOCTL_SET_GAIN
};

/**
* The types of cameras that can be connected to the board.
*/
typedef enum PPU_CameraType 
{
	PPUCT_LINE,				///< The camera is a line camera
	PPUCT_FRAME 			///< The camera is a frame camera
} PPU_CameraType ;

/**
* Defines the camera that is connected to the ppu.
*/
typedef struct PPU_CameraInfo 
{
	PPU_CameraType		CameraType; 		///< The type of the camera (line, pic...)
	Uint32				Caps; 				///< The camera's extended capabilities
	Int					TotalWidth; 		///< The total width of the frames/lines supported by the camera.
	Int					TotalHeight;        ///< The total height of the picture supported by the camera (1 if it is a line camera).
	Int					BPP;				///< The maximum number of bits per pixel the camera can deliver.
} PPU_CameraInfo;                           
	                                        
/**
* Collects information about a channel.
*/
typedef struct PPU_ChannelInfo {
	Int					Width;              ///< The width of the biggest image the channel supports.
	Int					Height;	            ///< The height of the biggest image the channel supports. 1 if this is only a line.
	Int					BPP;	            ///< Bits per pixel of the biggest image the channel supports.
	Int					QueueSize;	        ///< The channel queue's size. 
} PPU_ChannelInfo;                           

/**
* The channel object. A channel collects picture data from the PPU and transfers it onto the DSP. Each channels corresponds to a 
* single FIFO on the PPU. The mapping of channel number to FIFO address is done in PPU_HAL.h.
* @see PPU_HAL.h.
*/
typedef struct PPU_ChannelObject {
	Bool				Open;				///< Flag that indicates that the channel is open.
	Bool				Enabled;			///< Flag that indicates that the channel is open and enabled.
	
	BufferQueue			Queue;				///< The queue for transferring full picture buffers to the user application.
	BufferQueue_Frame	Queue_frames[PPU_MAXQUEUESIZE]; ///< The frames needed for the bufferqueue.
	
	Int					TCC;				///< The transfer complete code for the picture data.
	EDMA_Handle			DMA1;				///< The first DMA, used to transfer whole frames.
	EDMA_Handle			DMA2;				///< The second DMA, used to transfer the last, partial frame. It is linked from the first DMA and linked to the NULL DMA.
	
	Int					BufSize;			///< The complete picture buffer's size (or the line buffer's size when a line camera is connected).
	Int					PicSize;			///< The picture's size in bytes (i.e. the number of bytes that must be transferred until the buffer is regarded as full).
	Int					Framesize;
	Int					LastFramesize;
	Int					NumFrames;
	Ptr					Buffer;				///< The buffer which is currently in use. NULL if no buffer is allocated yet.

	PPU_ChannelInfo		Info;				///< General information about the channel.
} PPU_ChannelObject, * PPU_ChannelHandle;

/**
* The serial channel objects. Collects all the data needed for one serial line to the PPU.
*
* Note: the bTxRegisterEmpty flag is needed because we can't directly read the tx register's
* 		state in the PPU. That and the fact that our routine may be triggered by either a software
*		call or the hardware interrupt and must thus be able to know the register's state in absence
*		of any pending hwi make that flag necessary. 
*/
typedef struct PPU_SerialChannelObject {

	char				bufSerialTx[PPU_SERIALFIFOSIZE]; 	///< The storage buffer for the serial driver's Tx queue.
	char				bufSerialRx[PPU_SERIALFIFOSIZE]; 	///< The storage buffer for the serial driver's Rx queue.
	RingBuffer			SerialTx;							///< The serial driver's Tx ringbuffer.
	RingBuffer			SerialRx;							///< The serial driver's Rx ringbuffer.
	
	Bool				bTxRegisterEmpty;					///< A flag to store the Tx register's state.
} PPU_SerialChannelObject, * PPU_SerialChannelHandle;

/**
* The PPU device driver object. It stores all neccessary data for the device driver.
*/
typedef struct PPU_DevObject
{
	PPU_ChannelObject			Channels[PPU_MAXCHANNELS];		///< The channel objects.
} PPU_DevObject, * PPU_DevHandle;

/**
* The PPU serial driver object.
*/
typedef struct PPUSerial_DevObject
{
	PPU_SerialChannelObject		SerialChannels[PPU_NUMSERIAL];
	
	SEM_Obj				semSerialTx_lock;						///< A lock that is used for mutex'ing the serial driver's Tx ringbuffer access functions.
	SEM_Obj				semSerialRx_lock;						///< A lock that is used for mutex'ing the serial driver's Rx ringbuffer access functions.
	SEM_Obj				semSerialRx_wait;						///< Semaphore, used to wait for a rx character for a certain time.
} PPUSerial_DevObject, * PPUSerial_DevHandle;


/**
* Opens the ppu driver and returns a handler to the device, if succesful.
*
* @param	info	
* @param	queuesize	The size of the queues. 
* @return				A handle to the PPU device, if the device could be opened succesfully. NULL otherwise.
*/
PPU_DevHandle	ppuOpen(PPU_CameraInfo * info);


/**
* Closes the ppu driver.
*
* @param	device		The handle to the device.
*/
Int				ppuClose(PPU_DevHandle device);

/**
* Opens a specific channel to the PPU. This will in fact allocate the required memory in the DSP's SDRAM and reserve
* the needed DMA channels and TCCs. It will also create the queues needed to transfer the data from the driver to the
* application.
*
* @param	device		The handle to the device.
* @param	channelnum	The channel's number. There are PPU_MAXCHANNELS channels in the system, which is hardware dependent.
* @param	BPP			The maximum BPP the channel should support.
* @param	width		The maximum width the channel should support.
* @param	height		The maximum height the channel should support.
* @param	queuesize	The channels queue size.
* @return				SYS_OK if all goes well, error specifier if not.
*/
Int				ppuOpenChannel( PPU_DevHandle device, Int channelnum, Int BPP, Int width, Int height, Int queuesize, Int Framesize);

/**
* Closes a specific channel. This will free all ressources needed by the channel (memory, DMAs etc.).
*
* @param	device		The handle to the device.
* @param	channelnum	The channel's number.
* @return				SYS_OK if all goes well, error specifier if not.
*/
Int				ppuCloseChannel( PPU_DevHandle device, Int channelnum );

/**
* Enables and disables the data generator for a specific channel. The channel must already be opened
* and enabled for this.
*/
Int				ppuEnableDataGenerator(PPU_DevHandle device, Int channelnum, Bool bEnable );

void 			ppuIoCtl( PPU_DevHandle device, Uint32 type, Uint32 param );

/**
* Enables a channel for picture data input. This does the following:
* 	-	Enables all the DMA stuff and readies the DSP to receive data from the PPU
* 	-	Writes to the appropriate register on the PPU to enable the data stream.
*
* @param	device		The handle to the device.
* @param	channelnum	The channel's number.
* @return				SYS_OK if all goes well, error specifier if not.
*/
Int				ppuEnableChannel(PPU_DevHandle device, Int channelnum );

/**
* Disables a channel and stops reception of images from the ppu. Note that there still may be some pictures in the queue; they
* will remain there until either they are read out or the channel is closed.
*
* @param	device		The handle to the device.
* @param	channelnum	The channel's number.
*/
void			ppuDisableChannel(PPU_DevHandle device, Int channelnum );

/**
* Starts the PPU's picture processing operation. The driver will start the PPU's operation and will accept incoming images.
*
* @param	device		The handle to the device.
*/
void			ppuStartProcessing(PPU_DevHandle device);

/**
* Stops the PPU's picture processing operation.
*
* @param	device		The handle to the device.
*/
void			ppuStopProcessing(PPU_DevHandle device);


/**
* Writes a single register to the ppu. 
*
* @param	device		The handle to the device.
* @param 	regnum		The register's address.
* @param	value		The 32-bit value that should be written to the register.
*/
void	 		ppuWriteRegister(const PPU_DevHandle device, const Int regnum, const Uint16 value);


/**
* Reads a single register from the ppu.
*
* @param	device		The handle to the device.
* @param	regnum		The register's address.
* @return				The register's value.
*/
Uint16			ppuReadRegister(const PPU_DevHandle device, const Int regnum);


/**
* Configures the PPU.
*
* @param	device		The handle to the device.
*/
void			ppuConfigure(PPU_DevHandle device);


/**
* Gets the next full picture from the driver. The function may either be blocking or non-blocking, depending on the specified
* timeout value. The picture handler then points to a picture buffer. The buffer must be released to the driver by ReleasePicture()
* after it has been processed. Allocation and freeing of the picture buffers is the driver's job, the user application doesn't 
* (and mustn't...) do anything about it. Each GetPicture() must eventually be followed by a ReleasePicture(); it is, however, allowed
* to get multiple pictures and release them later, just make sure that there are enough buffers available (if all buffers were locked
* by the user application, the driver would be unable to receive any further images).
*
* @param	device		The handle to the device.
* @param	channelnum	The channel where the picture should be read from.
* @retval	hpic		A handle to the picture buffer.
* @param	timeout		A timeout value in system ticks (set by config utility), SYS_FOREVER or SYS_POLL (i.e. don't wait at all).
* @return				SYS_OK, SYS_ETIMEOUT or error specifier.
*/	
Int 			ppuGetPicture(PPU_DevHandle device, Int channelnum, PictureHandle * hhpic, Uns timeout);


/**
* Releases a picture buffer after it has been processed by the user application. This function must be called once for each call of
* GetPicture().
*
* @param	device		The handle to the device.
* @param	channelnum	The channel where the picture should be released.
* @param	pic			The handle to the picture buffer that should be released.
* @return				SYS_OK or error specifier.
*/
Int				ppuReleasePicture(PPU_DevHandle device, Int channelnum, PictureHandle hpic);

/**
* Writes data to the PPU's SRAM. After putting the data on the SRAM fifo, the call
* waits until all data has been written to the SRAM.
*/
Bool ppuWriteSRAM( /*PPU_DevHandle device,*/ Uint32 unBaseAddr, Uint16 * pBuffer, Uint32 unNumElements );

/**
* Reads data from the PPU's SRAM.
*/
Bool ppuReadSRAM( /*PPU_DevHandle device,*/Uint32 unBaseAddr, Uint16 * pBuffer, Uint32 unNumElements );


#ifdef __cplusplus
}
#endif

#endif

