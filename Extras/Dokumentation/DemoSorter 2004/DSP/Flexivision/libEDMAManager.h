/**
* @file
* @brief The EDMA manager mainly handles the edma interrupt distribution (all edma irqs are mapped to the same hwi).
* @author Bernhard Mäder
*
* SWI objects can be registered here with a corresponding transfer completion code (TCC). When an EDMA transfer completes
* and sets the according tcc bit, the EDMA manager will post the registered SWI task.
*
* The manager enables and clears the EDMA interrupt when first used.
*/
#ifndef _LIBEDMANAGER_H_
#define _LIBEDMANAGER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>

#include "FlexiVisioncfg.h"

#include <csl_irq.h>
#include <assert.h>


/** The maximum number of handlers that can be registered within the manager.*/
#define	EDMA_MAXNUMHANDLERS		16

#define	EDMA_NUMCOPYCHANNELS	8

#define EDMA_PRI_URGENT 		0x0
#define EDMA_PRI_HIGH 			0x1
#define EDMA_PRI_MED 			0x2
#define EDMA_PRI_LOW 			0x3
/*
#define EDMA_COPYDONE			0xFFFFFFFF
#define EDMA_1D1D				0
#define EDMA_1D2D				1
#define EDMA_2D1D				2
#define EDMA_2D2D				3
*/
#define EDMACOPY_DONE	0xFFFFFFFF
#define EDMACOPY_1D1D 	0
#define EDMACOPY_2D1D 	1
#define EDMACOPY_1D2D 	2
#define EDMACOPY_2D2D 	3

#define EDMA_ELEMSIZE_32 		0x0
#define EDMA_ELEMSIZE_16 		0x1
#define EDMA_ELEMSIZE_8 		0x2

#define EDMA_MAXBLOCKSIZE		0x8000

/** 
* Defines the range start for the copy channels DMAs. We'll start at the utopia DMAs, so we don't
* interfere with any hardware. 
*/
#define EDMA_RANGE_START		EDMA_CHA_UREVT

/**
* Use all DMAs available.
*/
#define EDMA_RANGE_END			64

/**
* Initializes the edma manager. This function must be called before any other function can
* be used.
*/
void	edmaInitManager();

/** 
* Registers a dma handler at the manager. It will clear the tcc bit after calling the appropriate
* handler.
*
* @param	tcc		The transfer conmpletion code for the edma.
* @param	hswi	A handle to the swi task that should be started when the corresponding tcc is set.
* @return			TRUE, if succesful, FALSE if the handler list is full.
*/
Bool	edmaRegisterHandler(Uint32 tcc, SWI_Handle hswi );

/** 
* Unregisters a dma handler at the manager. 
*
* @param	tcc		The transfer conmpletion code of the handler that should be unregistered.
* @return			TRUE, if succesful, FALSE if the handler was not found in the list.
*/
Bool	edmaFreeHandler(Uint32 tcc);

void 	edmaCopy( Ptr dest, Ptr source, Uint32 bytecount, Uint32 priority);
/*
int edmaCopy( Ptr source, Ptr dest, int priority, int elemsize, int elemcount, int src_inc, int dst_inc);

int edmaCopyBlock( Ptr source, Ptr dest, int priority, int elemsize, int elemcount, int src_inc, int dst_inc);
*/

		
Int		edmaStartCopy( Ptr dst, Ptr src, Uint32 unSize, Bool bUseSemaphore );
Int		edmaStartCopy2D( Uint32 type, Ptr dst, Ptr src, Uint16 lineLen, Uint16 lineCnt, Uint16 linePitch, Bool bUseSemaphore );
void	edmaWaitCopy( Int copyId );

#ifdef __cplusplus
}
#endif

#endif
