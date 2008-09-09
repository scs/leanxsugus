
#include "libEDMAManager.h"

#include <csl_dat.h>
#include <csl_edma.h>
#include <csl_edmahal.h>
#include <stdio.h>

#include "libDebug.h"

#define HEDMA2NUM(hDMA) ( (hDMA & 0x00FF0000) >> 16 )


/**
* The structure that makes up a edma hwi handler.
*/
typedef struct _EDMA_Handler
{
	Int32		tcc;
	SWI_Handle 	swiHandler;
	
} EDMA_Handler;


/** 
* The structure of an EDMA copy channel.
*/
typedef struct _EDMA_CopyChannel
{
	
	/** The transfer complete code for associated with the last DMA channel in the chain */
	Int					nTCC;
	
	/** The first DMA handle. */
	EDMA_Handle			hDMA1;	
	
	/** 
	* The transfer complete code associated with the second DMA channel, must be equal
	* to the DMA's number, since it is used for chaining. We won't use it directly but
	* must allocate it to reserve it.
	*/
	Int					nTCC2;
	
	/** The second DMA handle, which is used when we need a chained DMA structure. */
	EDMA_Handle			hDMA2;				
	
	/** Semaphore can be used to suspend a task until the copying process is completed. */
	SEM_Obj				semComplete;
	
	/** Flag to indicate that this channel is already in use. */
	Bool				bUsed;
	
	/** 
	* Flag to indicate that, if the channel is in use, the accompanying semaphore has
	* been proberly set up and must be used for waiting for the copy completion.
	*/
	Bool				bUsingSemaphore;
	
	/**
	* Flag to indicate that a second DMA is in use (for the rest frame). That means that
	* we'll have to check for two TCCs instead of just one.
	*/
	Bool				bSecondDMAUsed;
	
	
} EDMA_CopyChannel;


/**
* The main EDMA manager structure that stores all the channels and TCC Handlers.
*/
typedef struct _EDMA_Manager
{
	/** Flag to indicate whether the EDMA manager has already been initialized. */
	Bool				bInitialized;
	
	/** The array that stores the handlers. */
	EDMA_Handler		aryHandlers[EDMA_MAXNUMHANDLERS];
	
	/** The array that stores the copy channel structs. */
	EDMA_CopyChannel	aryCopyChannels[EDMA_NUMCOPYCHANNELS];
	
	/** A ring-buffer like structure to store the indices of the free copy channels. */
	Uint32				aryFreeChannels[ EDMA_NUMCOPYCHANNELS+1 ];
	
	/** The index in the ringbuffer of the next free channel. */
	Uint32				unNextFreeChannel;
	
	/** 
	* The index to the position RIGHT AFTER the last free channel. Like this,
	* the buffer is easily detected to be empty if (unNextFreeChannel == unLastFreeChannel).
	*/	
	Uint32				unLastFreeChannel;
	
	/**
	* These are the mask for the TCC bits of the copy channels.
	*/
	Uint32				unCopyChannelTCCMaskL;
	Uint32				unCopyChannelTCCMaskH;
	
} EDMA_Manager;

static EDMA_Manager	 	edmaManager = { FALSE };

/**
* This is the locally used hwi handler. It will call the other handlers according
* the tcc bits. This function <b>must be registered by the dsp/bios config tool! </b>
*/
void 	edmaInterrupt_hwi();

/**
* This is the locally used swi handler for the Copy engine. These are called by the HWI
* handler upon completion of one of the copy commands.
*/
void	edmaCopyDone_swi_funct();

/**
* Opens a DMA channel in a given range. This is used to not interfere with the 
* hardware-connected DMAs (like in drvPPU...)
*/
EDMA_Handle edmaOpenRange( Int nFirst, Int nLast, Uint32 unFlags );


// *************************************************************************

/**
* This function initializes the edma manager. It is called the first time, RegisterHandler()
* is used.
*/
void	edmaInitManager()
{
	Int i;
	
	// Abort if already initialized
	if ( edmaManager.bInitialized )
		return;
	edmaManager.bInitialized = TRUE;
	
	// Initialize the EDMA Interrupt handling feature
	// -----------------------------------------------
	
	// Clear and enable the interrupt
	IRQ_clear(IRQ_EVT_EDMAINT);
	IRQ_enable(IRQ_EVT_EDMAINT);
	
	// Clear the list of handlers
	for (i=0; i<EDMA_MAXNUMHANDLERS; i++)
	{
		edmaManager.aryHandlers[i].tcc = -1;
		edmaManager.aryHandlers[i].swiHandler = NULL;
	}	
	
	// Initialize the EDMA copy feature
	// -----------------------------------------------
	edmaManager.unCopyChannelTCCMaskH = 0;
	edmaManager.unCopyChannelTCCMaskL = 0;

	for (i=0; i<EDMA_NUMCOPYCHANNELS; i++)
	{
		// Reserve all DMA resources used
		edmaManager.aryCopyChannels[i].hDMA1 = edmaOpenRange( EDMA_RANGE_START, EDMA_RANGE_END, EDMA_OPEN_RESET);
		edmaManager.aryCopyChannels[i].nTCC  = EDMA_intAlloc( HEDMA2NUM(edmaManager.aryCopyChannels[i].hDMA1) );		
		edmaManager.aryCopyChannels[i].hDMA2 = edmaOpenRange( EDMA_RANGE_START, EDMA_RANGE_END, EDMA_OPEN_RESET);
		edmaManager.aryCopyChannels[i].nTCC2 = EDMA_intAlloc( HEDMA2NUM(edmaManager.aryCopyChannels[i].hDMA2) );
				
		assertLog( edmaManager.aryCopyChannels[i].nTCC != -1 );
		assertLog( edmaManager.aryCopyChannels[i].hDMA1 != EDMA_HINV );
		assertLog( edmaManager.aryCopyChannels[i].nTCC2 != -1 );
		assertLog( edmaManager.aryCopyChannels[i].hDMA2 != EDMA_HINV );		
				
		edmaManager.aryCopyChannels[i].bUsed = FALSE;
		edmaManager.aryCopyChannels[i].bUsingSemaphore = FALSE;
		edmaManager.aryCopyChannels[i].bSecondDMAUsed = FALSE;
		
		// Add the channel to the ringbuffer;
		edmaManager.aryFreeChannels[ i ] = i;
		
		// Clear and Enable both DMAs
		EDMA_clearChannel( edmaManager.aryCopyChannels[i].hDMA1 );
		EDMA_clearChannel( edmaManager.aryCopyChannels[i].hDMA2 );
		EDMA_enableChannel( edmaManager.aryCopyChannels[i].hDMA1 );  
		EDMA_enableChannel( edmaManager.aryCopyChannels[i].hDMA2 );
		
		// Enable the chaining by setting the appropriate bit in the CCER. We can already do 
		// this here, because nTCC2 is never used except for chained 1D transfers. So, we also
		// won't need to disable the chaining anywhere else.
		EDMA_enableChaining( edmaManager.aryCopyChannels[i].hDMA2 );
		
		// Create the TCC masks
		if ( edmaManager.aryCopyChannels[i].nTCC > 31 )
			edmaManager.unCopyChannelTCCMaskH |= ( 1 << (edmaManager.aryCopyChannels[i].nTCC - 32 ) );
		else
			edmaManager.unCopyChannelTCCMaskL |= ( 1 << edmaManager.aryCopyChannels[i].nTCC );
			
		// Create the semaphore
		SEM_new( &(edmaManager.aryCopyChannels[i].semComplete), 0 );
	}
	
	// Initialize the ringbuffer
	edmaManager.unNextFreeChannel = 0;
	edmaManager.unLastFreeChannel = EDMA_NUMCOPYCHANNELS;
	
}

// *************************************************************************

Bool	edmaRegisterHandler(Uint32 tcc, SWI_Handle hswi )
{
	Int			i;
	
	// look for an empty handler
	i = 0;
	while (  (edmaManager.aryHandlers[i].tcc != -1)  &&  (i < EDMA_MAXNUMHANDLERS) )
		i++;
	
	// No free handler found?
	if (i==EDMA_MAXNUMHANDLERS) return FALSE;
	
	// Register handler
	edmaManager.aryHandlers[i].tcc = tcc;
	edmaManager.aryHandlers[i].swiHandler = hswi;	
	
	return TRUE;
}

// *************************************************************************

Bool	edmaFreeHandler(Uint32 tcc)
{
	Int			i;
	
	// look for the handler
	i = 0;
	while (  (edmaManager.aryHandlers[i].tcc != tcc)  &&  (i < EDMA_MAXNUMHANDLERS) )
		i++;
		
	// Handler not found?
	if (i==EDMA_MAXNUMHANDLERS) return FALSE;
	
	// Unregister handler.
	edmaManager.aryHandlers[i].tcc = -1;
	edmaManager.aryHandlers[i].swiHandler = NULL;
	
	return TRUE;
}

// *************************************************************************

void 	edmaInterrupt_hwi_funct()
{
	Int 				i = 0;
	Int 				tcc;
		
	do
	{
		// search for registered handlers only.
		tcc = edmaManager.aryHandlers[i].tcc;		
		if ( tcc != -1)
		{
			// Test the tcc bit
			if ( EDMA_intTest(tcc) )
			{
				// Post the handler swi function.
				SWI_post(edmaManager.aryHandlers[i].swiHandler);				
				
				// Clear the tcc bit;
				EDMA_intClear(tcc);
			}
		}
		
		i++;
		
	} while (i < EDMA_MAXNUMHANDLERS);
	
	// Now see if any of the TCCs belong to the copy channels and post
	// the swi if so.    
	if ( 	( (EDMA_CIPRH & edmaManager.unCopyChannelTCCMaskH) != 0)
		|| 	( (EDMA_CIPRL & edmaManager.unCopyChannelTCCMaskL) != 0) )
	{
		SWI_post( &edmaCopyDone_swi );
	}
}

// *************************************************************************

void	edmaCopyDone_swi_funct()
{
	Int 				i;
	EDMA_CopyChannel * 	chan;
		
	// This SWI function is called if one of the copy channels was started using
	// a semaphore and has now finished.
	
	// So, go through all channels and see if this was the one that triggered this event.
	for (i=0; i<EDMA_NUMCOPYCHANNELS; i++)
	{
		// Get the channel
		chan = &(edmaManager.aryCopyChannels[i]);
		
		// Only examine it if it is used
		if ( chan->bUsed )
		{
			// Is it using a semaphore?
			if ( chan->bUsingSemaphore )
			{
				// Is the TCC set?
				if ( EDMA_intTest( chan->nTCC ) != 0 )
				{
					// Disable interrupt generation for this channel
					EDMA_intDisable( chan->nTCC );
					
					// Reset the semaphore flag.
					chan->bUsingSemaphore = FALSE;
					
					// Clear the interrupt
					EDMA_intClear( chan->nTCC );
					
					// And finally wake the waiting task.
					SEM_post( &(chan->semComplete) );
				}
			}
		}
	}	
}

// *************************************************************************

Int 	edmaGetFreeChannel()
{
	Int freeChannel = EDMACOPY_DONE;
	 	
 	// Disable SWIs
 	SWI_disable();
 		
	// If the ringbuffer isn't empty, we found a free channel
	if ( edmaManager.unNextFreeChannel != edmaManager.unLastFreeChannel )
	{
		// This is our free channel
		freeChannel = edmaManager.aryFreeChannels[ edmaManager.unNextFreeChannel ];
		edmaManager.aryCopyChannels[freeChannel].bUsed = TRUE;
		
		// Increment the pointers and adjust to ringbuffer size.
		edmaManager.unNextFreeChannel++;		
		if ( edmaManager.unNextFreeChannel > EDMA_NUMCOPYCHANNELS )
			edmaManager.unNextFreeChannel = 0;
	}
	
	//printf("Using channel: %d\n", freeChannel );
	
	// Re-enable SWIs
	SWI_enable();
	
	return freeChannel;
}

// *************************************************************************

void 	edmaReleaseChannel( Int channel )
{
	Int index;
	
	// Disable SWIs
 	SWI_disable();
 	
 	// Mark the channel as not used anymore
 	edmaManager.aryCopyChannels[channel].bUsed = FALSE;
 	
 	// Enter the newly free channel to the free list
	index = edmaManager.unLastFreeChannel;
	edmaManager.aryFreeChannels[ index ] = channel;	
 	
 	// Increment the LastFree pointer and adjust to buffersize
	edmaManager.unLastFreeChannel++;		
	if ( edmaManager.unLastFreeChannel > EDMA_NUMCOPYCHANNELS )
			edmaManager.unLastFreeChannel = 0;

	// Re-enable SWIs
	SWI_enable();
}

// *************************************************************************

#define EDMA_STDOPT( prio, esize, s_2d, d_2d, tcc )\
EDMA_OPT_RMK(\
	prio, 					/* Priority 0:urgent, 3:low */ \
	esize,                  /* element size (0:32bit, 1:16 bit, 2:8bit) */ \
	s_2d,	                /* 2D source */ \
	1,	                    /* source update mode (increment) */ \
	d_2d,                   /* 2D dest */ \
	0x1,                    /* dest update mode (increment) */ \
	1, 		                /* Transfer complete interrupt flag	(1=enable) */ \
	(tcc & 0xF),            /* Transfer complete code	(4bits) */ \
	((tcc & 0x30) >> 4),    /* Transfer complete code most significant bits (2bits, 64x only) */ \
	0,                      /* Alternate complete interrupt flag */ \
	0x00,                   /* Alternate transfer complete code */ \
	0,                      /* PDT source */ \
	0, 						/* PDT dest */ \
	0,                   	/* link */ \
	1 )					  	/* Frame synchronisation	(0=element/array, 1=frame/block) */

// *************************************************************************

Int		edmaStartCopy( Ptr dst, Ptr src, Uint32 unSize, Bool bUseSemaphore)
{
	// Define the framesize. Due to the nature of the DMAs, this must be smaller than 0xFFFF.
	//const Int nFrameSize = 0x8000;
	#define nFrameSize 0x8000
	
	Uint32	nAlignment;
	
	Int		nNumElements;
	Int	 	nElemSize;		//< The element size, ready to be put into the OPT field.
	
	Ptr		pDstLastframe;
	Ptr		pSrcLastframe;
	
	Int		nNumLastFrameElements;
	Int		nNumFrames;
	
	Int		nChannel;
	EDMA_CopyChannel * chan;
	
	// Get a free channel
	nChannel = edmaGetFreeChannel();
	
	if (nChannel == EDMACOPY_DONE )
	{
		printf("ERROR: no DMA channel free!\n");
		return EDMACOPY_DONE;
	}	
	
	chan = &(edmaManager.aryCopyChannels[nChannel]);	
	
	// Determine alignment of source, destination and size and find out, if
	// any of those three is 16- or 8-bit aligned.
	nAlignment = ( (Uint32)dst | (Uint32)src | (Uint32)unSize) & 0x03;
	 
	// Determine element size
	if ( nAlignment == 0 )
	{
		// 32 bit
		nElemSize = 0;
	
		// Calculate the number of elements and the number of blocks
		nNumElements = unSize >> 2;
		nNumFrames = nNumElements / nFrameSize;
		nNumLastFrameElements = nNumElements - ( nNumFrames * nFrameSize );		
		
		// Calculate the target pointers for the last frame
		pSrcLastframe = (Ptr)( (Int)src + (nNumFrames * (nFrameSize << 2)) );
		pDstLastframe = (Ptr)( (Int)dst + (nNumFrames * (nFrameSize << 2)) );
	
	}
	else if ( nAlignment == 2 )
	{
		// 16 bit
		nElemSize = 1;
		
		// Calculate the number of elements and the number of blocks
		nNumElements = unSize >> 1;
		nNumFrames = nNumElements / nFrameSize;
		nNumLastFrameElements = nNumElements - ( nNumFrames * nFrameSize );		
		
		// Calculate the target pointers for the last frame
		pSrcLastframe = (Ptr)( (Int)src + (nNumFrames * (nFrameSize << 1)) );
		pDstLastframe = (Ptr)( (Int)dst + (nNumFrames * (nFrameSize << 1)) );
	}
	else
	{
		// (nAlignment == 3) || (nAlignment == 1)
		// 8 Bit
		nElemSize = 2;
		
		// Calculate the number of elements and the number of blocks
		nNumElements = unSize;
		nNumFrames = nNumElements / nFrameSize;
		nNumLastFrameElements = nNumElements - ( nNumFrames * nFrameSize );		
		
		// Calculate the target pointers for the last frame
		pSrcLastframe = (Ptr)( (Int)src + (nNumFrames * (nFrameSize)) );
		pDstLastframe = (Ptr)( (Int)dst + (nNumFrames * (nFrameSize)) );
	}
	
	// Clear the channels before writing anything to it
	EDMA_clearChannel( chan->hDMA1 );
	EDMA_clearChannel( chan->hDMA2 );
	
	if ( nNumFrames == 0 )
	{
		// Only a last frame must be copied. Do a simple 1D, frame synchronized transfer here.		
		EDMA_configArgs( 	chan->hDMA1,
								    // prio, esize, s_2d, d_2d, tcc )
							EDMA_STDOPT( 1, nElemSize, 0, 0, chan->nTCC  ),
					    	EDMA_SRC_RMK(src),										// Source Address
					    	EDMA_CNT_RMK(0, nNumLastFrameElements),					// Transfer Counter - Numeric  
					    	EDMA_DST_RMK(dst),										// Destination Address - Numeric   
					    	EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    	EDMA_RLD_RMK(0,0)); 									// No "Element Count Reload", but link to next DMA.		

	}
	else if ( nNumLastFrameElements == 0 )
	{
		// We've got to copy more than one frame, but there is no rest. Do a 2D, block synchronized
		// transfer, with the EDMA array size being the size of our frame.
		EDMA_configArgs( 	chan->hDMA1,
								 	// prio, esize, s_2d, d_2d, tcc )
							EDMA_STDOPT( 1, nElemSize, 1, 1, chan->nTCC  ),
					    	EDMA_SRC_RMK(src),										// Source Address
					    	EDMA_CNT_RMK(nNumFrames-1, nFrameSize),					// Transfer Counter - Numeric  
					    	EDMA_DST_RMK(dst),										// Destination Address - Numeric   
					    	EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    	EDMA_RLD_RMK(0,0)); 									// No "Element Count Reload", but link to next DMA.
					    					    	
	}
	else
	{
		// Clear the TCC bits that will trigger the chained DMA
		// TODO: Try to remove this and see what happens. Could work without.
		EDMA_intClear( chan->nTCC2 );
		
		// We've got to copy multiple frames AND there is a rest. We use a 2D block synchronized
		// transfer to copy the whole frames and a 1D frame synchronized transfer to copy the
		// rest. The second DMA is chained to the first one so that it is triggered when the
		// first finishes.
		EDMA_configArgs( 	chan->hDMA1,
									// prio, esize, s_2d, d_2d, tcc )
							EDMA_STDOPT( 1, nElemSize, 1, 1, chan->nTCC2  ),			// TCC has to point to second DMA
					    	EDMA_SRC_RMK(src),										// Source Address
					    	EDMA_CNT_RMK(nNumFrames-1, nFrameSize),					// Transfer Counter - Numeric  
					    	EDMA_DST_RMK(dst),										// Destination Address - Numeric   
					    	EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    	EDMA_RLD_RMK(0,0)); 									// No "Element Count Reload", but link to next DMA.		
	
		EDMA_configArgs( 	chan->hDMA2,
									// prio, esize, s_2d, d_2d, tcc )
							EDMA_STDOPT( 1, nElemSize, 0, 0, chan->nTCC  ),
					    	EDMA_SRC_RMK(pSrcLastframe),							// Source Address
					    	EDMA_CNT_RMK(0, nNumLastFrameElements),					// Transfer Counter - Numeric  
					    	EDMA_DST_RMK(pDstLastframe),							// Destination Address - Numeric   
					    	EDMA_IDX_RMK(0,0),	    								// Index register - Numeric  
					    	EDMA_RLD_RMK(0,0)); 									// No "Element Count Reload", but link to next DMA.							    	
		
	}
	
	// If a semaphore is requested for this transfer, we also need an SWI on completion.
	// So, enable interrupt generation for the TCC. 
	// To optimize the function for the case where no semaphore is used ( which is usually
	// the case for all SWIs and processing TSKs calling), the interrupt is generally disabled
	// and will again be disabled in the SWI handler when using semaphores. So, if no
	// semaphores are used, don't do anything with the int enable. The same applies for
	// the bUsingSemaphore flag.
	if (bUseSemaphore)
	{
		EDMA_intEnable( chan->nTCC );		
		chan->bUsingSemaphore = TRUE;
		
		// Assert that the semaphore has, if used, been consumed.
		assertLog( SEM_count( &(chan->semComplete) ) == 0 );
	}
	
	// Start DMA
	EDMA_setChannel( chan->hDMA1 );
	
	// Return the channel's index
	return nChannel;
}

// *************************************************************************

Int		edmaStartCopy2D( Uint32 type, Ptr dst, Ptr src, Uint16 lineLen, Uint16 lineCnt, Uint16 linePitch, Bool bUseSemaphore)
{
	Uint32	nAlignment;
	
	Int		nNumElementsLine;
	Int		nIncrement;
	Int	 	nElemSize;		//< The element size, ready to be put into the OPT field.

	Int		nChannel;
	EDMA_CopyChannel * chan;
	
	Uint32	unOPT;
	
	// Get a free channel
	nChannel = edmaGetFreeChannel();
	
	if (nChannel == EDMACOPY_DONE )
	{
		printf("ERROR: no DMA channel free!\n");
		return EDMACOPY_DONE;
	}	
	
	chan = &(edmaManager.aryCopyChannels[nChannel]);	
	
	// Determine alignment of source, destination and size and find out, if
	// any of those three is 16- or 8-bit aligned.
	nAlignment = ( (Uint32)dst | (Uint32)src | lineLen | linePitch ) & 0x03;
	 
	// Determine element size
	if ( nAlignment == 0 )
	{
		// 32 bit
		nElemSize = 0;
	
		// Calculate the number of elements and the number of blocks
		nNumElementsLine = lineLen >> 2;
	}
	else if ( nAlignment == 2 )
	{
		// 16 bit
		nElemSize = 1;
	
		// Calculate the number of elements and the number of blocks
		nNumElementsLine = lineLen >> 1;
	}
	else
	{
		// 8 bit
		nElemSize = 2;
	
		// Calculate the number of elements and the number of blocks
		nNumElementsLine = lineLen;
	}
	
	// Calculate the increment
	nIncrement = linePitch - lineLen;
	
	// Clear the channel before writing anything to it
	EDMA_clearChannel( chan->hDMA1 );
	
	// Build the OPT field depending on the requested transfer type.
	switch( type )
	{
	case EDMACOPY_1D1D:
		// If a linear transfer is requested with a 2D copy issue, we just
		// have to set the increment to zero. We can't just set both 2D flags
		// to zero since the DMA would becom a 1D1D and thus frame synchronized 
		// (not block synchronized) which means we'd have to trigger each line.
		nIncrement = 0;
						 // prio, esize, s_2d, d_2d, tcc )
		unOPT = EDMA_STDOPT( 1, nElemSize, 1, 1, chan->nTCC  );
		break;
		
	case EDMACOPY_1D2D:
		unOPT = EDMA_STDOPT( 1, nElemSize, 0, 1, chan->nTCC  );
		break;
		
	case EDMACOPY_2D1D:
		unOPT = EDMA_STDOPT( 1, nElemSize, 1, 0, chan->nTCC  );
		break;
		
	case EDMACOPY_2D2D:
		unOPT = EDMA_STDOPT( 1, nElemSize, 1, 1, chan->nTCC  );
		break;
	
	}
	
	// Now configure the DMA.
	EDMA_configArgs( 	chan->hDMA1,
							unOPT,
					    	EDMA_SRC_RMK(src),										// Source Address
					    	EDMA_CNT_RMK(lineCnt-1, nNumElementsLine),				// Transfer Counter
					    	EDMA_DST_RMK(dst),										// Destination Address - Numeric   
					    	EDMA_IDX_RMK(nIncrement,0),								// Index register - Numeric  
					    	EDMA_RLD_RMK(0,0)); 
	
	// Like in edmaStartCopy(), we only enable the interrupt if a semaphore is
	// requested
	if (bUseSemaphore)
	{
		EDMA_intEnable( chan->nTCC );		
		chan->bUsingSemaphore = TRUE;
		
		// Assert that the semaphore has, if used, been consumed.
		assertLog( SEM_count( &(chan->semComplete) ) == 0 );
	}
	
	// Start DMA
	EDMA_setChannel( chan->hDMA1 );
	
	// Return the channel's index
	return nChannel;
}

// *************************************************************************

// The function should not be optimized in any way and always interrubtible.
#pragma FUNC_INTERRUPT_THRESHOLD (edmaWaitCopy, 1)

void	edmaWaitCopy( Int copyId )
{
	EDMA_CopyChannel * chan;
	
	if (copyId == EDMACOPY_DONE)
		return;
	
	assertLog( copyId <= EDMA_NUMCOPYCHANNELS );
	
	
	// Get the channel object
	chan = &(edmaManager.aryCopyChannels[copyId]);
	
	// See if we should wait using semaphore or loop
	if ( chan->bUsingSemaphore )
	{
		// If a semaphore is used, simply pend on it
		SEM_pend( &(chan->semComplete), SYS_FOREVER );
	}
	else
	{
		// If no semaphore is used, wait until the TCC is set.
		while( EDMA_intTest(chan->nTCC) == 0 );
		
		// Clear the bit
		EDMA_intClear( chan->nTCC );
	}
	
	// Free that channel
	edmaReleaseChannel( copyId );
}

// *************************************************************************

//void edmaBlockCopy(Ptr dest, Ptr source, int priority, int bytecount);

// *************************************************************************

void edmaCopy( Ptr dest, Ptr source, Uint32 bytecount, Uint32 priority)
{
	edmaWaitCopy( edmaStartCopy( dest, source, bytecount, FALSE ));
/*
	static Bool bOpen = FALSE;
	
	if ( ! bOpen )
	{
		bOpen = TRUE;
		DAT_open( DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D );
	}
	
	DAT_wait(DAT_copy( source, dest, bytecount ));
*/
/*
	#define MAXBLOCKSIZE 4*0x8000
	static SEM_Handle lock = NULL;
	
	if (lock == NULL)
		lock = SEM_create( 1, 0);
		
	// round up bytecount
	bytecount = (bytecount + 3) & ~0x00000003;
	
	// Use the semaphore as a mutex...	
	SEM_pend( lock, SYS_FOREVER );
	
	while(bytecount > MAXBLOCKSIZE)
	{
		edmaBlockCopy(dest, source, priority, MAXBLOCKSIZE);
		dest 	= (Ptr)((Uint32)dest + MAXBLOCKSIZE);
		source 	= (Ptr)((Uint32)source + MAXBLOCKSIZE);
		bytecount = bytecount - MAXBLOCKSIZE;
	}
	
	edmaBlockCopy(dest, source, priority, bytecount);
	
	SEM_post( lock );
*/
}

// *************************************************************************
/*
void edmaBlockCopy(Ptr dest, Ptr source, int priority, int bytecount)
{
	int 			opt;
	int				elemcount;
	
	//assertLog( (bytecount & 0x03) == 0);
	
	elemcount = (bytecount/4) & 0xFFFF;
	//assertLog(elemcount <= 0xFFFF);
	
	EDMA_intClear( 1 );
		
	opt = EDMA_OPT_RMK( 	(priority == 1) ? 1 : 2,// Priority: urgent! //high, but not urgent.					
							00,		               	// element size (32 bit)
							0,		                // 2D source													
							0x1,				    // source update mode (Fixed address)							
							0x00,                   // 2D dest														
							0x1,				    // dest update mode												
							1,                 		// Transfer complete interrupt flag	(1=enable)					
							(1 & 0xF),	            // Transfer complete code	(4bits)								
							0,    // Transfer complete code most significant bits (2bits, 64x only)
							0,                      // Alternate complete interrupt flag							
							0x00,                   // Alternate transfer complete code								
							0,                      // PDT source													
							0, 						// PDT dest														
							0,	                    // link															
							1 );				  	// Frame synchronisation	(0=element, 1=frame)				

	EDMA_qdmaConfigArgs( opt, (Uint32)source, elemcount, (Uint32)dest, 0);
	
	// wait for completion
	while( EDMA_intTest(1) == 0 );
		//printf("waiting\n");
}

*/
// *************************************************************************

/**
* A helper function that opens a DMA channel in a given range.
*/ 
EDMA_Handle edmaOpenRange( Int nFirst, Int nLast, Uint32 unFlags )
{
	EDMA_Handle h;
	Int i;
	
	for ( i=nFirst; i <= nLast; i++)
	{
		h = EDMA_open( i, unFlags );
		if ( h != EDMA_HINV )
			return h;
	}
	
	return EDMA_HINV;
}

// *************************************************************************



