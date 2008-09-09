
#include "drvHPI.h"

#include "libEDMAManager.h"
#include "libDebug.h"

#ifdef _DEBUG
Bool				hint = FALSE;
#endif

//#define _HPI_LOG_ENABLE				///< Undef, if the HPI driver should not log anything.

#ifdef _HPI_LOG_ENABLE
#define HPI_LOG(x) 			LOG_printf(&trace, x)
#define HPI_LOG1(str, arg) 	LOG_printf(&trace, str, arg)
#else
#define HPI_LOG(x) 			
#define HPI_LOG1(str, arg) 	
#endif


extern Int SDRAM;								///< Define the SDRAM segment which is configured by the config tool.

/**
* Define the "shared memory" mailbox structure that is used to pass messages between the etrax
* and the dsp.
*
* The mailbox acts as shared mem between the two processors. There is a buffer for messages to the
* DSP (InBuffer) and one for the opposite direction (OutBuffer).
*/
typedef struct HPI_Mailbox
{
	volatile Byte	InBuffer[HPI_PACKETBUFFER_SIZE];			///< The input buffer (Etrax to DSP)
	volatile Byte	OutBuffer[HPI_PACKETBUFFER_SIZE];			///< The output buffer (DSP to Etrax)
} HPI_Mailbox;

#pragma DATA_SECTION(hpiMailbox, ".sdram_mbx"); 		///< Put the mailbox to the mailbox segment. This segment is defined in the linker command file.
volatile HPI_Mailbox	hpiMailbox;						///< The shared memory used to pass messages between DSP and Etrax.


/**
* Allocate the one and only hpi device object. Since the driver doesn't suppoert
* multiple task accessing the same device (and thus no multiple open() calls), one
* statically allocated object suffices.
*/
HPI_DevObject			hpiDevObject;			

const Uint8				hpiOutPacketMsg_Open = 0x01;	///< A message to tell the out packet task to start operation.
const Uint8				hpiOutPacketMsg_Close = 0x02;	///< A message to tell the out packet task to cease operation.

Bool					hpiIsOpened = FALSE;			///< A flag that tells if the driver is opened. The flag is also used for the writer task.


/** The IRQ event that is used for incoming interrupts from the ETRAX. */
#define HPI_DSPINT		IRQ_EVT_DSPINT


/**
* The device driver's outgoing packets writer task function. The task will write packets on the out queue to the
* shared memory mailbox, trigger the host interrupt and wait until the packet gets invalidated by the etrax.
* This has to be a TSK (as opposed to a HWI or SWI thread), since it has to poll on the mailbox (while 
* it's waiting for the host) and it's best to do the polling with a while() { sleep() } statement. Sleeping,
* however, is only possible in TSKs.
*
* The writer task will pend on the SemOut semaphore to capture the event of incoming packets from the application.
* It will post on the SemOut_back whenever it has finished a packet.
*
* @return			SYS_OK.
*/
Int	hpiOutPacket_tsk_funct();

/**
* The DSPINT hardware interrupt handler. All it does is disable DSPINT and post execution of the in packet
* SWI handler hpiDSPINT_swi.
*/
void hpiDSPINT_hwi_funct();

/**
* The device driver's incoming packet reader task function. The swi will read packets from the mailbox and put them
* on the in queue. It's triggered by the DSPINT interrupt and is directly started by the DSPINT HWI. The SWI
* will pend on the SemIn_back queue until a new free buffer is made available (i.e. consumed by the application).
* 
* The reader task will pend on the SemIn_back semaphore, so that when an incoming message cannot be received
* because there's no place in the QueueIn, the task can block until a new buffer is available. It will
* also post to the SemIn to inform the waiting application that a new packet is availabe.
*
* The swi is defined using the DSP/BIOS config utility.
*/
Int hpiDSPINT_swi_funct();

SEM_Handle hpiLock;

/**
* Driver-internal helper function to fill the packet header of a packet structure that is
* lying in the SDRAM. Thus, those accesses are done using EDMA transfers.
*/
void			hpiWritePacketHeader( HPI_Packet * pPacket, Int32 nSize, Int32 nDestAddr, Int32 nSrcAddr, Int32 nType );

/**
* Driver-internal helper function to read a packet's size using an EDMA transfer, so that
* the operation is safe on packets in the SDRAM.
*/
Uint32			hpiReadPacketSize( const HPI_Packet * pPacket );

/**
* Driver-internal helper function to copy a packet that is stored in the DSP's SDRAM.
* The packet's totalsize field is taken as size for the copy operation; all accesses
* are performed using EDMA transfers and are thus safe for SDRAM packets.
*/
void			hpiCopyPacket( HPI_Packet * pDestPacket, const HPI_Packet * pSourcePacket );



// *************************************************************************

void			hpiWritePacketHeader( HPI_Packet * pPacket, Int32 nSize, Int32 nDestAddr, Int32 nSrcAddr, Int32 nType )
{
	HPI_Packet * 	packet;
	Uint8			buffer[HPIPACKET_HEADERSIZE];
	
	// Prepare the header on the stack
	packet = (HPI_Packet*)buffer;
	
	packet->Size = nSize;
	packet->DestAddr = nDestAddr;
	packet->SrcAddr = nSrcAddr;
	packet->Type = nType;
	
	// and write it to the mailbox using an EDMA
	edmaCopy( (Ptr)pPacket, (Ptr)packet, HPIPACKET_HEADERSIZE, EDMA_PRI_LOW );
}

// *************************************************************************

Uint32			hpiReadPacketSize( const HPI_Packet * pPacket )
{
	Uint32			unSize = 0;
	
	// copy the size out of the packet.
	edmaCopy( (Ptr)&unSize, (Ptr)&(pPacket->Size), sizeof( unSize ), EDMA_PRI_LOW );
	
	return unSize;
}

// *************************************************************************

void			hpiCopyPacket( HPI_Packet * pDestPacket, const HPI_Packet * pSourcePacket )
{
	Uint32			unSize;
	
	// read the source packet's size
	unSize = hpiReadPacketSize( pSourcePacket );
	
	// copy the packet.
	edmaCopy( (Ptr)pDestPacket, (Ptr)pSourcePacket, unSize, EDMA_PRI_LOW );
}

// *************************************************************************

HPI_DevHandle	hpiOpen(Int queuesize)
{
	// check if the driver is not open already.
	if (hpiIsOpened)
	{
		HPI_LOG("HPI: driver already opened.");
		return NULL;
	}
	
	hpiLock = SEM_create( 1, 0 );
	
	// Assert that we're triggering the right event
	assertLog(HPI_getEventId() == HPI_DSPINT);
	
	// Create the queues.
	bfqCreate( &hpiDevObject.QueueIn, hpiDevObject.QueueIn_frames, queuesize, HPI_PACKETBUFFER_SIZE, SDRAM);
	bfqCreate( &hpiDevObject.QueueOut, hpiDevObject.QueueOut_frames, queuesize, HPI_PACKETBUFFER_SIZE, SDRAM);
		
	// Set the address fields of the packets in the mailbox
	// to -1 to signal to the ETRAX that this packet is free to
	// use.
	hpiWritePacketHeader( (Ptr)(hpiMailbox.InBuffer), -1, -1, -1, 0 );
	hpiWritePacketHeader( (Ptr)(hpiMailbox.OutBuffer), -1, -1, -1, 0 );
	
	// Declare the driver as opened 
	hpiIsOpened = TRUE;
	
	// Post a message to the out packet task and tell it to start
	MBX_post( &hpiOutPacket_mbx, (Ptr)&hpiOutPacketMsg_Open, SYS_FOREVER );

	// and enable the reader interrupt
	HPI_setDspint(1);
	IRQ_clear(HPI_DSPINT);
	IRQ_enable(HPI_DSPINT);
	
	return &hpiDevObject;
}

// *************************************************************************

Int 	hpiClose(HPI_DevHandle device)
{
	HPI_Packet		* packet;
	
	// check if the driver is not open already.
	if (!hpiIsOpened)
	{
		return SYS_OK;
	}
	hpiIsOpened = FALSE;
	
	// Stop the out packet task -> send a close message AND a packet, since it may be
	// pending on bfqGetBuffer()
	MBX_post( &hpiOutPacket_mbx, (Ptr)&hpiOutPacketMsg_Close, SYS_FOREVER );
	if ( bfqAllocBuffer( &device->QueueOut, (Ptr*)(&packet), 0 ) )
		bfqPutBuffer( &device->QueueOut, packet );
		
	// Disable DSPINT
	IRQ_disable(HPI_DSPINT);

	// Delete queues
	bfqDelete( &device->QueueIn );
	bfqDelete( &device->QueueOut );
		
	HPI_LOG("HPI: driver closed");
	
	return SYS_OK;
}

// *************************************************************************

Int		hpiGetMessage(HPI_DevHandle device, HPI_Packet * packet, Uns timeout)
{
	HPI_Packet 		* pckt;
	//Uint32			unPacketSize;
	
		
	// Check if driver is open.
	if (!hpiIsOpened) return SYS_ENODEV;
	
	// Wait until at least one packet is in the input queue,
	// or time out.
	HPI_LOG("HPI: GetMessage(): pending...");
	
	if ( ! bfqGetBuffer( &device->QueueIn, (Ptr*)(&pckt), timeout) )	
		return SYS_ETIMEOUT;
		
	hpiCopyPacket( packet, pckt );
	
	// Release the buffer back to the queue
	bfqReleaseBuffer( &device->QueueIn, (Ptr)pckt );
	
	// Enable the DSPINT (if it isn't already), because there is a new
	// frame available for the SWI.
	IRQ_enable(HPI_DSPINT);

	return SYS_OK;
}

// *************************************************************************

Bool	hpiIsOutputQueueFree( HPI_DevHandle device )
{
	if ( bfqGetEmptyCount( &device->QueueOut ) > 0 )
		return TRUE;
	else
		return FALSE;
}

// *************************************************************************

Int		hpiSendMessage(HPI_DevHandle device, HPI_Packet * packet, Uns timeout)
{
	HPI_Packet 	* pckt;
	
	// Check if driver is open.
	if (!hpiIsOpened) return SYS_ENODEV;
	
	// Wait until there is place on the out queue (i.e. wait until at least one packet in out_back queue),
	// or time out.
	HPI_LOG("HPI: SendMessage(): pending...");
	if ( ! bfqAllocBuffer( &device->QueueOut, (Ptr*)(&pckt), timeout) )
		return SYS_ETIMEOUT;
	
	// copy the packet's content to the new buffer.
	hpiCopyPacket( pckt, packet );
	//edmaCopy((Ptr)pckt, (Ptr)packet, packet->Size, 0);
	
	// Put the frame on the queue
	bfqPutBuffer( &device->QueueOut, (Ptr)pckt);
		
	// .. and post the sem which awakes the writer task, if
	// it's pending on it.
//	SEM_post(&hpiSemOut);
	
	return SYS_OK;
	
}

// *************************************************************************

Int		hpiSendPicture(HPI_DevHandle device, PictureHandle pic, Uint32 channel, Uint32 srcaddr, Uint32 dstaddr, Uns timeout)
{
	HPI_Packet 	* pckt;
	Int			packetsize;
	
	// Check if driver is open.
	if (!hpiIsOpened) return SYS_ENODEV;
	
	// Wait until there is place on the out queue (i.e. wait until at least one packet in out_back queue),
	// or time out.
	//dbgLog("HPI: SendPicture(): pending...");
	if ( ! bfqAllocBuffer( &device->QueueOut, (Ptr*)(&pckt), timeout) )
		return SYS_ETIMEOUT;
	
	//dbgLog("HPI: Got buffer");

	// Calc the packet's size and assert that it's not too big.
	packetsize = HPIPACKET_HEADERSIZE + HPIPACKET_IMGDATA_SIZE + pic->TotalSize;
	//assertLog( HPI_PACKETBUFFER_SIZE >= packetsize);
	
	// Fill the packet structure
	hpiWritePacketHeader( pckt, packetsize, srcaddr, dstaddr, HPIPT_IMAGE );
	
	// Copy the image data to the buffer.	
	edmaCopy( (Ptr)&(pckt->Data.Image.imgChannel), (Ptr)&channel, sizeof(channel), EDMA_PRI_LOW );
	edmaCopy( (Ptr)pckt->Data.Image.imgData, (Ptr)pic, pic->TotalSize, EDMA_PRI_LOW );
	
	// Put the frame on the queue
	bfqPutBuffer( &device->QueueOut, (Ptr)pckt);
	
	return SYS_OK;
}
// *************************************************************************

Int		hpiSendText(HPI_DevHandle device, const Char * str, Uint32 srcaddr, Uint32 dstaddr, Uns timeout)
{
	HPI_Packet 	* pckt;
	Int			packetsize;
	Int			length;
	
	// Check if driver is open.
	if (!hpiIsOpened) return SYS_ENODEV;
	
	// Check if the message is not too big
	length = strlen(str) + 1;
	if (length > HPI_PACKETBUFFER_SIZE - HPIPACKET_HEADERSIZE)
		return SYS_EBADIO;
	
	// Wait until there is place on the out queue (i.e. wait until at least one packet in out_back queue),
	// or time out.
	HPI_LOG("HPI: SendText(): pending...");
	if ( ! bfqAllocBuffer( &device->QueueOut, (Ptr*)(&pckt), timeout) )
		return SYS_ETIMEOUT;
	
	// Calc the packet's size and assert that it's not too big.
	packetsize = HPIPACKET_HEADERSIZE + length;
	assertLog( HPI_PACKETBUFFER_SIZE >= packetsize);
	
	// Fill the packet structure
	hpiWritePacketHeader( pckt, packetsize, srcaddr, dstaddr, HPIPT_TEXT );
		
	// Copy the text data to the buffer.	
	edmaCopy(  (Ptr)(pckt->Data.Text.txtTxt), (Ptr)str, length, EDMA_PRI_LOW );	
	
	// Put the frame on the queue
	bfqPutBuffer( &device->QueueOut, (Ptr)pckt);
			
	return SYS_OK;
}

// *************************************************************************

Int		hpiGetMessageCount(HPI_DevHandle device)
{
	return bfqGetCount( &(device->QueueIn) );
}

// *************************************************************************


void hpiDSPINT_hwi_funct()
{
	// Disable the interrupt since we're not able to
	// receive a new signal until the old packet
	// has been copied completely.
	// PRE: 	This HWI's dispatcher does NOT mask
	//			 out any interrupt. If it would, HPI_DSPINT
	//			 could not be disabled here and stay disabled
	//			 for hpiInPacket_tsk_funct().
	
	IRQ_disable(HPI_DSPINT);
	HPI_setDspint(1);
	
	// Post to the semaphore that will trigger the in packet handler.
	SEM_post( &hpiInPacket_sem );
}


// *************************************************************************

Int hpiInPacket_tsk_funct()
{
	HPI_Packet 			* packet;	
	HPI_Packet			* mbxpacket;

	while(1)
	{
		// Wait for a signal by the Etrax. The HWI will post to the semaphore.
		SEM_pend( &hpiInPacket_sem, SYS_FOREVER );
			
		// PRE:		DSPINT was triggered by the HWI.
		//			DSPINT is disabled by the HWI handler
		// POST:	Copies the message from the InBuffer
		//			 to the next free frame on queueIn.
		//			Marks the inbuffer to be free again.
		//			Enables the interrupt if there are 
		//			 still empty buffers available.
		
	
		// Get a free buffer ready on the back queue (should be,
		// since DSPINT mustn't be enabled when there is none.
		assertLog( bfqGetEmptyCount( &hpiDevObject.QueueIn ) > 0 );
		bfqAllocBuffer( &hpiDevObject.QueueIn, (Ptr*)(&packet), 0);
		
		// HPI_LOG("HPI: DSPINT InPacket SWI handler: got frame");
		
		// Copy the packet from the mailbox to the inBuffer (note: get the packet's size by EDMA)
		mbxpacket = (HPI_Packet*)hpiMailbox.InBuffer;
		hpiCopyPacket( packet, mbxpacket); 
		
		// Put the packet on the queue
		bfqPutBuffer( &hpiDevObject.QueueIn, (Ptr)packet );
		
		// Enable DSPINT again, if there are other empty frames on QueueIn.
		// If there are none, leave the irq disabled. It will be enabled by the
		// getMessage function.
		if ( 0 < bfqGetEmptyCount( &hpiDevObject.QueueIn ) )
			IRQ_enable(HPI_DSPINT);
			
		// Set the address and size fields of the packet in the mailbox
		// to -1 to signal to the ETRAX that this packet is free to
		// use.
		hpiWritePacketHeader( (Ptr)mbxpacket, -1, -1, -1, 0 );					
	}
}

// *************************************************************************

Int	hpiOutPacket_tsk_funct()
{
	HPI_Packet 			* packet;
	HPI_Packet 			* mbxpacket;
	Int32				nMbxPacketSize;
	
	Int					del;
	
	Uint8 				msg;
	
	while (1)
	{
		MBX_pend( &hpiOutPacket_mbx, &msg, SYS_FOREVER );
		
		if ( msg == hpiOutPacketMsg_Open )
		{
			HPI_LOG("HPI: Writer task started.");	
			
			while (1)
			{	
				
				// Wait until a new packet is placed in the 
				// queueOut.
				bfqGetBuffer( &hpiDevObject.QueueOut, (Ptr*)(&packet), SYS_FOREVER);
			
				// quit if a close message arrived in the meanwhile
				/* DEBUG::
				if ( SYS_OK == MBX_pend( &hpiOutPacket_mbx, &msg, 0 ) )
					if (msg == hpiOutPacketMsg_Close)
						break;
						*/
				
				HPI_LOG("HPI: OutPacket TSK: Writing packet to mailbox");
				// puts("HPI: OutPacket TSK: Writing packet to mailbox");
							
				// Copy the packet to the mailbox' outBuffer (note: get the packet's size by EDMA)
				mbxpacket = (HPI_Packet*)hpiMailbox.OutBuffer;
				hpiCopyPacket( mbxpacket, packet );
								
				// DEBUG:: check the packet's size
				nMbxPacketSize = hpiReadPacketSize( mbxpacket );				
				if ( (nMbxPacketSize > 1024*1024*6) || (nMbxPacketSize < 10) )
				{
					dbgLog("outTask: Size was: %d. MsgType: %d, Msg: '%s'->'%s'", nMbxPacketSize, packet->Type, packet->Data.Text.txtTxt, mbxpacket->Data.Text.txtTxt );
					dbgLog("dest: %p, source: %p", mbxpacket, packet );
				}
				
				// release the buffer
				bfqReleaseBuffer( &hpiDevObject.QueueOut, packet );
			
				// interrupt the host 
				HPI_LOG("HPI: OutPacket TSK: Interrupting host");				
				HPI_setHint(1);
			
			#ifdef _DEBUG
				hint = TRUE;	// Just for the simulation: this will trigger the simulated host task
			#endif
			
			
				// Now, the task has to wait until the host has properly
				// received the message and written -1 to the size field.
				del = 0;
				edmaCopy( &nMbxPacketSize, (Ptr)&(mbxpacket->Size), sizeof( nMbxPacketSize ), EDMA_PRI_LOW );
				while ( nMbxPacketSize != -1)
				{
					TSK_sleep(HPI_OUT_POLLDELAY);
					del++;
					
					// Timeout?
					if ( del*HPI_OUT_POLLDELAY > HPI_OUT_TIMEOUT)
					{
						// invalidate the packet on our own -> bail out.
						dbgLog("timeout");
						hpiWritePacketHeader( (Ptr)mbxpacket, -1, -1, -1, 0 );
						break;						
					}
					
					// Copy again.
					nMbxPacketSize = hpiReadPacketSize( mbxpacket );
					//edmaCopy( &nMbxPacketSize, (Ptr)&(mbxpacket->Size), sizeof( nMbxPacketSize ), EDMA_PRI_LOW );
				}
								
				// quit if a close message arrived in the meanwhile
				if ( SYS_OK == MBX_pend( &hpiOutPacket_mbx, &msg, 0 ) )
					if (msg == hpiOutPacketMsg_Close)
						break;
			
			} // while (1)	
			
			HPI_LOG("HPI: Writer task stopped.");		
		}
	} // while (1)
}

// *************************************************************************

