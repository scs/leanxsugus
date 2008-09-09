

#include "libBufferQueue.h"

#include "libDebug.h"



// *************************************************************************


Bool bfqCreate_d( BufferQueue_Handle queue, Int BufCount, Int BufSize, Int SegID)
{
	Int i;
	Ptr p;
	
	// Allocate the frames
	queue->Frames = MEM_alloc( 0, sizeof(BufferQueue_Frame) * BufCount, 8);
	queue->bDynamicFrames = TRUE;
	
	// Init the QUE and SEM objects.
	SEM_new(&(queue->semFullBuffers), 0);
	SEM_new(&(queue->semEmptyBuffers), BufCount);
	
	QUE_new( &(queue->queFullBuffers) );
	QUE_new( &(queue->queEmptyBuffers) );
	QUE_new( &(queue->queEmptyFrames) );
	
	for (i=0; i<BufCount; i++)
	{
		// TODO: handle errors.
		
		// If the _BFQ_INIT_BUFFERS is defined, fill the buffers with 0xFF,
		// leave them uninitialized, otherwise.
#ifdef _BFQ_INIT_BUFFERS
		p = MEM_valloc( SegID, BufSize, 0, 0xFF );
		queue->Frames[i].Buffer = p;
		assertLog( p != MEM_ILLEGAL);
#else
		p = MEM_alloc( SegID, BufSize, 0 );
		queue->Frames[i].Buffer = p;
		assertLog( p != MEM_ILLEGAL);
#endif
		QUE_put(&(queue->queEmptyBuffers), &(queue->Frames[i]));
	}
		
	queue->BufSize = BufSize;
	queue->BufCount = BufCount;
	queue->SegId   = SegID;
	
	return TRUE;
}


// *************************************************************************


Bool		bfqCreate			( BufferQueue_Handle queue, BufferQueue_Frame * frames, Int BufCount, Int BufSize, Int SegID)
{
	Int i;
	Ptr p;
	
	// Acquire the frames
	queue->Frames = frames;
	queue->bDynamicFrames = FALSE;
	
	// Init the QUE and SEM objects.
	SEM_new(&(queue->semFullBuffers), 0);
	SEM_new(&(queue->semEmptyBuffers), BufCount);
	
	QUE_new( &(queue->queFullBuffers) );
	QUE_new( &(queue->queEmptyBuffers) );
	QUE_new( &(queue->queEmptyFrames) );
	
	for (i=0; i<BufCount; i++)
	{
		// TODO: handle errors.
		
		// If the _BFQ_INIT_BUFFERS is defined, fill the buffers with 0xFF,
		// leave them uninitialized, otherwise.
#ifdef _BFQ_INIT_BUFFERS
		p = MEM_valloc( SegID, BufSize, 0, 0xFF );
		queue->Frames[i].Buffer = p;
		assertLog( p != MEM_ILLEGAL);
#else
		p = MEM_alloc( SegID, BufSize, 0 );
		queue->Frames[i].Buffer = p;
		assertLog( p != MEM_ILLEGAL);
#endif
		QUE_put(&(queue->queEmptyBuffers), &(queue->Frames[i]));
	}
		
	queue->BufSize = BufSize;
	queue->BufCount = BufCount;
	queue->SegId   = SegID;
	
	return TRUE;
}


// *************************************************************************

void 		bfqDelete			( BufferQueue_Handle queue )
{
	Int i;
	
	// only have to free the buffers.
	for (i=0; i<queue->BufCount; i++)
		MEM_free( queue->SegId, queue->Frames[i].Buffer, queue->BufSize );
		
	// de-allocate the frames if they were allocate within the queue
	if (queue->bDynamicFrames)
		MEM_free( 0, queue->Frames, sizeof(BufferQueue_Frame) * queue->BufCount );
}


// *************************************************************************

Bool		bfqGetBuffer		( BufferQueue_Handle queue, Ptr * buffer, Uns timeout )
{
	BufferQueue_Frame * frame;
	
	// wait until a full buffer is available, timeout
	// if necessary.
	if (! SEM_pend(&(queue->semFullBuffers), timeout) )
	{
		return FALSE;
	}
	
	// Get the frame from the queue and double check it
	assertLog(! QUE_empty(&(queue->queFullBuffers)));
	frame = QUE_get(&(queue->queFullBuffers));
	assertLog((Ptr)frame != (Ptr)&(queue->queFullBuffers));
	
	// put the frame to the empty frames queue
	QUE_put( &(queue->queEmptyFrames), frame);
	
	// Copy the pointer
	*buffer = frame->Buffer;
	
	return TRUE;
}

// *************************************************************************

void		bfqReleaseBuffer 	( BufferQueue_Handle queue, Ptr buffer )
{
	BufferQueue_Frame * frame;
	
	// get a free frame (don't have to check if queue is empty).
	frame = QUE_get( (Ptr) &(queue->queEmptyFrames) );
	
	// Put the buffer to the frame.
	frame->Buffer = buffer;
	
	// Put the frame on the _back queue.
	QUE_put( &(queue->queEmptyBuffers), frame);	
	
	// ... and post the corresponding sem
	SEM_post( &(queue->semEmptyBuffers) );
}

// *************************************************************************

Bool		bfqDropFrontBuffer	( BufferQueue_Handle queue )
{
	Ptr buffer;
	Bool res;
	
	res = bfqGetBuffer( queue, &buffer, 0);
	
	if (res)
	{
		bfqReleaseBuffer( queue, buffer);
		return TRUE;
	}
	else
		return FALSE;
}

// *************************************************************************

Int			bfqGetCount			( BufferQueue_Handle queue )
{
	return SEM_count( &(queue->semFullBuffers) );
}

// *************************************************************************

Int			bfqGetEmptyCount	( BufferQueue_Handle queue )
{
	return SEM_count( &(queue->semEmptyBuffers) );
}

// *************************************************************************

Bool		bfqAllocBuffer		( BufferQueue_Handle queue, Ptr * buffer, Uns timeout )
{
	BufferQueue_Frame * frame;
	
	// wait until an empty buffer is available, timeout
	// if necessary.
	if (! SEM_pend(&(queue->semEmptyBuffers), timeout) )
	{
		return FALSE;
	}
	
	// double check
	if ( QUE_empty(&(queue->queEmptyBuffers)) )
		return FALSE;
		
	// Get the frame from the queue and double check it	
	frame = QUE_get((Ptr)&(queue->queEmptyBuffers));
	if ((Ptr)frame == (Ptr)&(queue->queEmptyBuffers))
		return FALSE;
	
	// Copy the pointer
	*buffer = frame->Buffer;
	
	// put the frame to the empty frames queue
	QUE_put( &(queue->queEmptyFrames), frame);	
	
	return TRUE;
}

// *************************************************************************

Bool		bfqForceAllocBuffer	( BufferQueue_Handle queue, Ptr * buffer, Uns timeout )
{
	Bool res;
	
	res = bfqAllocBuffer( queue, buffer, timeout );
	
	if (!res)
	{
		// If no room, drop a buffer
		if ( bfqGetEmptyCount( queue ) == 0)
			bfqDropFrontBuffer(	queue );	
			
		// and try again
		return bfqAllocBuffer( queue, buffer, timeout );
	}
	return TRUE;
}

// *************************************************************************

void		bfqPutBuffer		( BufferQueue_Handle queue, Ptr buffer )
{
	BufferQueue_Frame * frame;
	
	// get a free frame (don't have to check if queue is empty).
	frame = QUE_get( (Ptr) &(queue->queEmptyFrames) );
	
	// Put the buffer to the frame.
	frame->Buffer = buffer;
	
	// Put the frame on the _back queue.
	QUE_put( &(queue->queFullBuffers), frame);	
	
	// ... and post the corresponding sem
	SEM_post( &(queue->semFullBuffers) );
}

// *************************************************************************
