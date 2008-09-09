

#include "drvPPU.h"
#include "drvPPU_HAL.h"
#include "classWrapper.h"
#include "classStatus.h"
#include "libDebug.h"

#define _PPU_LOG_ENABLE				///< Undef, if the PPU driver should not log anything.

#ifdef _PPU_LOG_ENABLE
#define PPU_LOG(x) 			LOG_printf(&trace, x)
#define PPU_LOG1(str, arg) 	LOG_printf(&trace, str, arg)
#else
#define PPU_LOG(x) 			
#define PPU_LOG1(str, arg) 	
#endif

extern Int			SDRAM;								///< The segment ID of the SDRAM (defined by the config utility).

EDMA_Handle			ppuDMANull = 0;						///< The NULL table used for terminating linked DMA transfers

Bool				ppuIsOpened = FALSE;				///< A flag that tells if the driver is opened. The flag is also used by the serial driver task.

PPU_DevObject		ppuDevObject;						///< The single dev object for the ppu (device may only be opened once, so one object suffices).

/**
* This function is called when the EDMA completes the transfer of a picture from the PPU to the SDRAM.
* The SWI is triggered by the EDMAManager.
*/
void ppuPictureReady_swi_funct(Int channelnum);


/**
* This is the serial port driver's main task function.
*/
void ppuSerial_tsk_funct();

/**
* Configures a certain channel's EDMA for data transfer.
*/
void ppuConfigEDMA(PPU_DevHandle device, Int channelnum);



// *************************************************************************

PPU_DevHandle	ppuOpen(PPU_CameraInfo * info)
{
	Int i;
	
	// Driver may only be opened once at a time.
	if (ppuIsOpened)
	{
		PPU_LOG("PPU: driver already opened.");
		return NULL;
	}
		
	// Declare all channels as closed
	for (i=0; i<PPU_MAXCHANNELS; i++)
		ppuDevObject.Channels[i].Open = FALSE;
			
		
	// Allocate and configure the null termination table
	ppuDMANull = EDMA_allocTable(-1);
	EDMA_configArgs(ppuDMANull, 0, 0, 0, 0, 0, 0);
		
	return &ppuDevObject;
}

// *************************************************************************

Int		ppuClose(PPU_DevHandle device)
{
	Int i;
	
	// Close all opened channels
	for ( i=0 ; i<PPU_MAXCHANNELS ; i++)
		if ( ppuDevObject.Channels[i].Open )
			ppuCloseChannel(device, i);
	
	// Free the null table.
	EDMA_freeTable(ppuDMANull);
	ppuDMANull = 0;
	
	return SYS_OK;
}

// *************************************************************************

Int				ppuOpenChannel( PPU_DevHandle device, Int channelnum, Int BPP, Int width, Int height, Int queuesize, Int Framesize)
{
	PPU_ChannelHandle 	chan;
	
	chan = &device->Channels[channelnum];
	
	// Check if the channel is open already
	if (chan->Open)
		return SYS_EBADIO;
		
	
	// Fill the channel structure
	chan->Info.BPP 		= BPP;
	chan->Info.Width 	= width;
	chan->Info.Height 	= height;
	chan->Info.QueueSize = queuesize;
				
	// Calculate the size of the buffers needed.
	chan->PicSize 		= (height * width * BPP + 7)/8;							// calc the picture size
	chan->PicSize 		= (chan->PicSize + 1) & ~0x001;							// round up to a multiple of 2
	chan->BufSize 		= chan->PicSize + PIC_HEADERSIZE;						// calc the buffer size
	chan->BufSize 		= (chan->BufSize + 3) & ~0x0003;						// round it up to a multiple of 4
	chan->Framesize 	= Framesize;											// Store the framesize
	chan->NumFrames		= chan->PicSize / chan->Framesize;  					// Calculate the number of frames
	chan->LastFramesize = chan->PicSize - (chan->NumFrames * chan->Framesize); 	// Calculate the last frame's size
	
	// Write the framesize to the FPGA
	ppuWriteRegister( device, ppuHal.PictureChannel[channelnum].RegFramesize_addr, chan->Framesize / 2 );

	// Create the queue
	bfqCreate( &(chan->Queue), chan->Queue_frames,  queuesize, chan->BufSize, SDRAM);

	// Allocate the DMAs
	chan->DMA1 = EDMA_open( ppuHal.PictureChannel[channelnum].EdmaTrigger, EDMA_OPEN_RESET);
	assertLog( chan->DMA1 !=  EDMA_HINV);
	
	chan->DMA2 = EDMA_allocTable(-1);
	assertLog( chan->DMA2 !=  EDMA_HINV);
	
	// Allocate an EDMA tcc for this channel, use the TCC with the same number as the 
	// DMAs, so we certainly won't interfere with anything else (if "everything else"
	// is also applying to that rule...)
	// Add 1 so that we won't interfere with the DAT module either.
	chan->TCC = EDMA_intAlloc( ppuHal.PictureChannel[channelnum].EdmaTrigger + 1 );
	assertLog( chan->TCC != -1);
		
	// Register the HWI handler
	edmaRegisterHandler( chan->TCC, ppuHal.PictureChannel[channelnum].Ready_swi);
	EDMA_intEnable(chan->TCC);
	
	//dbgLog( "Channel opened. DMA1: %X, DMA2: %X", chan->DMA1, chan->DMA2 );
	
	// Opened the channel succesfully
	chan->Open = TRUE;
	chan->Enabled = FALSE;
	chan->Buffer = NULL;
		
	return SYS_OK;
}

// *************************************************************************

Int				ppuCloseChannel( PPU_DevHandle device, Int channelnum )
{
	PPU_ChannelHandle 	chan;
	
	chan = &device->Channels[channelnum];
	
	// Check if the channel is really open
	if ( ! chan->Open) return SYS_EBADIO;
		
	// Disable channel first and free the DMAs
	ppuDisableChannel(device, channelnum);
	EDMA_freeTable(chan->DMA2);
	EDMA_close(chan->DMA1);
	
	// unregister the tcc at the EDMA manager and free it.
	EDMA_intDisable(chan->TCC);
	edmaFreeHandler(chan->TCC);
	EDMA_intFree(chan->TCC);
	

	// Succesfully closed.
	chan->Open = FALSE;

	return SYS_OK;
}

// *************************************************************************

Int			ppuEnableChannel(PPU_DevHandle device, Int channelnum )
{
	Bool							res;
	PPU_ChannelHandle 				chan;
	const PPU_PictureChannelHAL *	hal;
	
	chan = &device->Channels[channelnum];
	hal = &(ppuHal.PictureChannel[channelnum]);
	
	
	// Check if the channel is realy open and not enabled.
	if ( ! chan->Open)
		return SYS_EBADIO;
	
	if ( chan->Enabled )
		return SYS_OK;
		
	// Get a buffer for the picture.
	res = bfqAllocBuffer( &(chan->Queue), (Ptr*)&(chan->Buffer), 0);
	assertLog(res != 0);
		
	
	// Configure the DMA and enable it. 
	ppuConfigEDMA( device, channelnum );
	EDMA_clearChannel( chan->DMA1 );
	EDMA_enableChannel( chan->DMA1 );
	
	// Enable the interrupt in the PPU -> Set the appropriate bit in the interrupt mask register.
	ppuMemory[ hal->RegIntEnable_addr ] |= hal->RegIntEnable_mask;
	
	// Select the camera as source for this channel
	ppuMemory[ hal->RegSourceSelect_addr ] |= hal->RegSourceSelect_mask;
	
	// Enable the channel in the PPU -> Set the appropriate bit in the control register.
	// DEBUG::: don't yet do this, since this triggers a new image.
	ppuMemory[ hal->RegChannelEnable_addr ] |= hal->RegChannelEnable_mask;
	
	
	chan->Enabled = TRUE;
	
	return SYS_OK;
}

// *************************************************************************

Int			ppuEnableDataGenerator(PPU_DevHandle device, Int channelnum, Bool bEnable )
{
	PPU_ChannelHandle 				chan;
	const PPU_PictureChannelHAL *	hal;
	
	chan = &device->Channels[channelnum];
	hal = &(ppuHal.PictureChannel[channelnum]);
	
	
	// Check if the channel is realy open and not enabled.
	if ( ! chan->Open)
		return SYS_EBADIO;
	
	if ( ! chan->Enabled )
		return SYS_EBADIO;
		
	// if the data generator should be enabled, clear the corresponding bit.
	if ( bEnable )
	{
		ppuMemory[ hal->RegSourceSelect_addr ] &= ~(hal->RegSourceSelect_mask);
	}
	else
	{
		ppuMemory[ hal->RegSourceSelect_addr ] |= hal->RegSourceSelect_mask;
	}
	
	return SYS_OK;
}

// *************************************************************************

void ppuIoCtl( PPU_DevHandle device, Uint32 type, Uint32 param )
{
	volatile Int i;	
	
	switch( type )
	{
	case PPU_IOCTL_TRIGGER_IMG_ACQUISITION:
		ppuMemory[ ppuHal.RegControl_addr ] &= ~(REG_CTRL_TRIGGERIMG_MSK);	
		for (i=0; i<100; i++);	
		ppuMemory[ ppuHal.RegControl_addr ] |= REG_CTRL_TRIGGERIMG_MSK;
		break;
		
	case PPU_IOCTL_SET_READY_LINE:
		if ( param != 0)
			ppuMemory[ ppuHal.RegControl_addr ] |= REG_CTRL_READYLINE_MSK;
		else
			ppuMemory[ ppuHal.RegControl_addr ] &= ~REG_CTRL_READYLINE_MSK;	
		break;
		
	case PPU_IOCTL_SET_GAIN:
		ppuMemory[ PPUREG_GAINRED_ADDR ] = PIC_RED( param ) ;
		ppuMemory[ PPUREG_GAINGREEN_ADDR ] = PIC_GREEN( param );
		ppuMemory[ PPUREG_GAINBLUE_ADDR ] = PIC_BLUE( param );
		break;		
	}
}

// *************************************************************************

void			ppuDisableChannel(PPU_DevHandle device, Int channelnum )
{
	PPU_ChannelHandle 				chan;
	const PPU_PictureChannelHAL *	hal;
	
	chan = &device->Channels[channelnum];	
	hal = &(ppuHal.PictureChannel[channelnum]);	
	
	// Check if the channel is realy open
	if ( ! chan->Open)
		return;
		
	// Disable the interrupt in the PPU -> clear the enable bit in the interrupt mask register.
	ppuMemory[ hal->RegIntEnable_addr ] &= ~(hal->RegIntEnable_mask);
	ppuMemory[ hal->RegChannelEnable_addr ] &= ~(hal->RegChannelEnable_mask);
		
	// Disable the DMA channel
	EDMA_disableChannel(chan->DMA1);	
	
	// Put the unfinished buffer if there is one.
	if ( chan->Buffer != NULL)
		bfqPutBuffer( &(chan->Queue), chan->Buffer);
	chan->Buffer = NULL;
	
	chan->Enabled = FALSE;
}

// *************************************************************************

void ppuConfigEDMA( PPU_DevHandle device, Int channelnum)
{
	Ptr					dest;
	Ptr					dest_lastframe;
	Int					framesize;
	Int					lastframesize;
	Int					numframes;
	
	PPU_ChannelHandle 				chan;
	const PPU_PictureChannelHAL *	hal;
	
	chan = &device->Channels[channelnum];
	hal = &(ppuHal.PictureChannel[channelnum]);	
	
	framesize = chan->Framesize;
	lastframesize = chan->LastFramesize;
	numframes = chan->NumFrames;
	
	//dbgLog("DMA set up on chan: %d. frame: %d, last: %d, num: %d", channelnum, framesize, lastframesize, numframes );
	//dbgLog("Picsize: %d", chan->PicSize );
	
		
	// Calculate the target pointers
	dest 			= (Ptr)( (Int)(chan->Buffer) + PIC_HEADERSIZE );
	dest_lastframe 	= (Ptr)( (Int)dest + (numframes * framesize) );
	
	
	// a macro which defines the OPT fields of the dma config. Its arguments define whether the DMA
	// will trigger a transfer completion interrupt, which tcc would be used for that interrupt and
	// if it is a linked DMA.
	#define PPU_EDMA_STDOPT(tcint, tcc, link) 	EDMA_OPT_RMK( 	0x0, 					/* Priority: urgent! //high, but not urgent.									*/ \
																0x0,                    /* element size (16 bit)											*/ \
																0,		                /* 2D source														*/ \
																00,	                    /* source update mode (Fixed address)								*/ \
																0x00,                   /* 2D dest															*/ \
																0x1,                    /* dest update mode													*/ \
																tcint,                  /* Transfer complete interrupt flag	(1=enable)						*/ \
																(tcc & 0xF),            /* Transfer complete code	(4bits)									*/ \
																((tcc & 0x30) >> 4),    /* Transfer complete code most significant bits (2bits, 64x only)	*/ \
																0,                      /* Alternate complete interrupt flag								*/ \
																0x00,                   /* Alternate transfer complete code									*/ \
																0,                      /* PDT source														*/ \
																0, 						/* PDT dest															*/ \
																link,                   /* link																*/ \
																1 )					  	/* Frame synchronisation	(0=element, 1=frame)					*/


	// Check if the DMA transfer has to be split up because the framesize doesn't fit
	if (lastframesize == 0)
	{
		// no split up required
		EDMA_configArgs( chan->DMA1,
						PPU_EDMA_STDOPT(1, chan->TCC, 1),						// Do interrupt, link to null table.
					    EDMA_SRC_RMK(hal->pFifo),								// Source Address
					    EDMA_CNT_RMK(numframes-1, framesize/4),					// Transfer Counter - Numeric  
					    EDMA_DST_RMK(dest),										// Destination Address - Numeric   
					    EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    EDMA_RLD_RMK(0, ppuDMANull) ); 							// No "Element Count Reload", but link to next DMA.
	}
	else if (numframes == 0)
	{
		// no split up (only the last frame needed.)
		EDMA_configArgs( chan->DMA1,
						PPU_EDMA_STDOPT(1, chan->TCC, 1),						// Do interrupt, link to null table.
					    EDMA_SRC_RMK(hal->pFifo),								// Source Address
					    EDMA_CNT_RMK(0, lastframesize/4),						// Transfer Counter - Numeric  
					    EDMA_DST_RMK(dest_lastframe),							// Destination Address - Numeric   
					    EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    EDMA_RLD_RMK(0, ppuDMANull) ); 							// No "Element Count Reload", but link to next DMA.
	}
	else
	{	// multiple frames, and a lastframe
		EDMA_configArgs( chan->DMA1,
						PPU_EDMA_STDOPT(0, chan->TCC, 1),						// no interrupt, but link with next dma.
					    EDMA_SRC_RMK(hal->pFifo),  								// Source Address
					    EDMA_CNT_RMK(numframes-1, framesize/4),					// Transfer Counter - Numeric  
					    EDMA_DST_RMK(dest),										// Destination Address - Numeric   
					    EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    EDMA_RLD_RMK(0, chan->DMA2)								// No "Element Count Reload", but link to next DMA.
					    );
			
		EDMA_configArgs(chan->DMA2,
						PPU_EDMA_STDOPT(1, chan->TCC, 1),						// Do generate an interrupt.
					    EDMA_SRC_RMK(hal->pFifo), 								// Source Address
					    EDMA_CNT_RMK(0, lastframesize/4),						// Transfer Counter - Numeric  
					    EDMA_DST_RMK( dest_lastframe ),							// Destination Address - Numeric   
					    EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    EDMA_RLD_RMK(0, ppuDMANull)								// No "Element Count Reload", but link to null DMA. 
					    );
	}
}

// *************************************************************************

void	ppuWriteRegister(const PPU_DevHandle device, const Int regnum, const Uint16 value)
{
	ppuMemory[regnum] = value;
}

// *************************************************************************


Uint16	ppuReadRegister(const PPU_DevHandle device, const Int regnum)
{
	return ppuMemory[regnum];
}

// *************************************************************************


void	ppuConfigure(PPU_DevHandle device)
{
	// TODO
}

// *************************************************************************

void	ppuStartProcessing(PPU_DevHandle device)
{
	// TODO
}

// *************************************************************************

void	ppuStopProcessing(PPU_DevHandle device)
{
	// TODO
}

// *************************************************************************

Int 	ppuGetPicture(PPU_DevHandle device, Int channelnum, PictureHandle * hhpic, Uns timeout)
{
	Bool				res;
	PPU_ChannelHandle 	chan;
	
	chan = &device->Channels[channelnum];
	
	res = bfqGetBuffer( &(chan->Queue), (Ptr*)hhpic, timeout);
	
	if (res)
		return SYS_OK;
	else
		return SYS_ETIMEOUT;
}

// *************************************************************************

Int		ppuReleasePicture(PPU_DevHandle device, Int channelnum, PictureHandle hpic)
{
	PPU_ChannelHandle 	chan;
	
	chan = &device->Channels[channelnum];
	
	bfqReleaseBuffer( &(chan->Queue), (Ptr)hpic);
	
	return SYS_OK;
}

// *************************************************************************

Bool ppuWriteSRAM( Uint32 unBaseAddr, Uint16 * pBuffer, Uint32 unNumElements )
{
	volatile Uint16 unWriteDone;
	Int	i;
	Uint32 	unCurOffs = 0;
	Uint32	unCurNum;
	
	Uint32 	unTimeout;
		
	// Write the address to the addr register
	ppuMemory[ ppuHal.RegSramWriteAddrLo_addr ] = (Uint16)(unBaseAddr & 0xFFFF);
	ppuMemory[ ppuHal.RegSramWriteAddrHi_addr ] = (Uint16)((unBaseAddr>>16) & 0x00FF);

	do
	{		
		// Determine current block size
		if ( unNumElements > ppuHal.unSramFifoSize )
			unCurNum = ppuHal.unSramFifoSize;
		else
			unCurNum = unNumElements;
			
		// Write the elements to the fifo.
		for ( i=unCurOffs; i<unCurOffs+unCurNum; i++ )
		{
			ppuMemory[ ppuHal.RegSramFifo_addr ] = pBuffer[i];
		}
			
		unCurOffs += unCurNum;
		unNumElements -= unCurNum;
				
		// Loop	until the write succeeded.
		do
		{
			unWriteDone = ppuMemory[ ppuHal.RegSramFifoStatus_addr ] & ppuHal.RegSramFifoStatusWriteDone_mask;
		} while ( (unWriteDone == 0) && (unTimeout != 0) );		
		
	} while ( unNumElements > 0 );
	
	return TRUE;
	
}

// *************************************************************************

Bool ppuReadSRAM( Uint32 unBaseAddr, Uint16 * pBuffer, Uint32 unNumElements )
{
	Uint16 unReadReady;
	Int	i;
	Uint32 	unCurOffs = 0;
	Uint32	unCurNum;
	volatile Uint16 dummy;
		
	// Write the address to the addr register
//	ppuMemory[ ppuHal.RegSramReadAddrLo_addr ] = (Uint16)(unBaseAddr & 0xFFFF);
//	ppuMemory[ ppuHal.RegSramReadAddrHi_addr ] = (Uint16)((unBaseAddr>>16) & 0x00FF);
	
	do
	{
		ppuMemory[ ppuHal.RegSramReadAddrLo_addr ] = (Uint16)((unBaseAddr+unCurOffs) & 0xFFFF);
		ppuMemory[ ppuHal.RegSramReadAddrHi_addr ] = (Uint16)(((unBaseAddr+unCurOffs)>>16) & 0x00FF);
		
		// Wait until the data is ready.
		do
		{
			unReadReady = ppuMemory[ ppuHal.RegSramFifoStatus_addr ] & ppuHal.RegSramFifoStatusReadReady_mask;
		} while ( unReadReady == 0 );	
		
		// Determine current block size
		if ( unNumElements > ppuHal.unSramFifoSize )
			unCurNum = ppuHal.unSramFifoSize;
		else
			unCurNum = unNumElements;
			
		// Make a dummy read
		//dummy = ppuMemory[ ppuHal.RegSramFifo_addr ];
			
		// Read the elements from the fifo.
		for ( i=unCurOffs; i<unCurOffs+unCurNum; i++ )
		{
			pBuffer[i] = ppuMemory[ ppuHal.RegSramFifo_addr ] ;
		}
			
		unCurOffs += unCurNum;
		unNumElements -= unCurNum;
		
	} while ( unNumElements > 0 );
		
	return TRUE;
}

// *************************************************************************

void ppuPictureReady_swi_funct(Int channelnum)
{
	Bool 				res;
	PictureHandle		hpic;
	Uint16				dummy;
	Uint32				wordcount;
	Uint16				wordcount_hi;
	Uint16				wordcount_lo;
	void				* newBuffer;
	
	// DEBUG::
	static Bool			led2 = FALSE;
	static Bool			led3 = FALSE;
	
	PPU_ChannelHandle 				chan;
	const PPU_PictureChannelHAL *	hal;
	
	chan = &(ppuDevObject.Channels[channelnum]);
	hal = &(ppuHal.PictureChannel[channelnum]);
	
	//PPU_LOG1("PPU: Picture ready on channel %d.", channelnum);
//	dbgLog("Picture received on channel %d", channelnum );
		
	// Write picture info to picture header
	hpic = (PictureHandle)(chan->Buffer);
	hpic->TotalSize = chan->PicSize + PIC_HEADERSIZE;
	hpic->TotalWidth = chan->Info.Width;
	hpic->TotalHeight = chan->Info.Height;
	hpic->OffsetX = 0;
	hpic->OffsetY = 0;
	hpic->Width = chan->Info.Width;
	hpic->Height = chan->Info.Height;
	
	hpic->Type = picGetType( chan->Info.BPP, FALSE );
	
	// check synchronisation: read back the wordcount and compare.
	wordcount_lo = ppuMemory[hal->RegWordcountLo_addr];
	wordcount_hi = ppuMemory[hal->RegWordcountHi_addr];
	wordcount = ((Uint32)wordcount_hi << 16)  | (Uint32)wordcount_lo;
	
	// Is there a synchronisation error?
	if ( wordcount != (chan->PicSize / 2) )			// must be last word of picture
	{
		gStats.unPPUNumPicsBad++;
		gStats.unPPULastErrorWordCountIs = wordcount;
		gStats.unPPULastErrorWordCountShould = (chan->PicSize / 2);
		
		// flush the fifo. The next thing we'll hear from the FPGA is the event for
		// a new picture's frame. So, let's just re-enable and re-setup the DMA and
		// wait for that event.
		// Flushing means setting the flush bit of the current channel in the control register.
		ppuMemory[ hal->RegChannelFlush_addr ] &= ~(hal->RegChannelFlush_mask);
		ppuMemory[ hal->RegChannelFlush_addr ] |= hal->RegChannelFlush_mask;
		
		// Clear the DMA event flag
		EDMA_clearChannel( chan->DMA1 );
		
		ledLight( 3, led3);
		led3 = ~led3;
	}
	else
	{
		gStats.unPPUNumPicsGood++;
	}
	// DEBUG::: put the picture anyway
	//else
	// if synch is fine, try to put the picture and allocate a new buffer from the queue.
	{
	
		/* OLD:::
		bfqPutBuffer( &(chan->Queue), chan->Buffer );
		
		// Get a new buffer for the picture.
		res = bfqAllocBuffer( &(chan->Queue), (Ptr*)&(chan->Buffer), 0);
		// If there was none, drop the oldest from the queue and try againg
		while( !res )
		{
			res = bfqDropFrontBuffer( &(chan->Queue) );	
			res = bfqAllocBuffer( &(chan->Queue), (Ptr*)&(chan->Buffer), 0);
		} // END OLD
		*/
		
		// Get a new buffer for the picture, but stay with the old one if there is none
		res = bfqAllocBuffer( &(chan->Queue), &newBuffer, 0);
		if (res)
		{
			// Put the full buffer on the queue.
			bfqPutBuffer( &(chan->Queue), chan->Buffer );
			chan->Buffer = newBuffer;
		}
		else
		{	
			gStats.unPPUNumPicsDropped++;	
			//ledLight( 2, led2);
			//led2 = ~led2;
		}
	}			
			

	// Reconfig the DMA
	ppuConfigEDMA(&ppuDevObject, channelnum);
}

// *************************************************************************




