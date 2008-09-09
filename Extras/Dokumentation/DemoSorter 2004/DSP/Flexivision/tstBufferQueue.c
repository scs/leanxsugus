
#include "tstBufferQueue.h"


extern Int SDRAM;

BufferQueue 		bfq;
BufferQueue_Frame	frames[4];

void BufferQueueTest_tsk2_funct();


void BufferQueueTest()
{
	Int 	i;
	char 	* buf;
	
	bfqCreate(&bfq, frames, 4, 16, SDRAM);	
	
	// *********************************************************
	//  Test 1
	// *********************************************************
	// Try to get a buffer from the queue's front, this must fail
	assert ( ! bfqGetBuffer( &bfq, (Ptr*)&buf, 1 ));
	
	// Fill the queue with 4 buffers to the back.
	for (i=0; i<4; i++)
	{
		assert( bfqAllocBuffer( &bfq, (Ptr*)&buf, 0 ) );
		buf[0] = i;
		bfqPutBuffer( &bfq, (Ptr)buf);
	}
	
	// Try to alloc a new one, which must fail
	assert( ! bfqAllocBuffer( &bfq, (Ptr*)&buf, 1 ) );
	
	// Get the 4 buffers back from the front
	for (i=0; i<4; i++)
	{
		assert( bfqGetBuffer( &bfq, (Ptr*)&buf, 0 ) );
		assert( buf[0] == i );
		bfqReleaseBuffer( &bfq, (Ptr)buf );
	}
	
	// Try to get the next -> must fail
	assertLED( bfqGetCount( &bfq ) == 0 );
	assertLED( ! bfqGetBuffer( &bfq, (Ptr*)&buf, 0 ) );
	
	// *********************************************************
	//  Test 2
	// *********************************************************
	// Quick read and slow writes -> Reader is waiting on the writer
	// and the queue is empty most of the time.
	
	// Start the writer task
	TSK_create((Fxn)BufferQueueTest_tsk2_funct, NULL, NULL);
	
	// Get buffers from the queue and test if everything's coming through correctly.
	// This will test the blocking read capability.
	for (i=0; i<8; i++)
	{
		assert( bfqGetBuffer( &bfq, (Ptr*)&buf, SYS_FOREVER ) );
		assert( buf[0] == i + 16 );
		bfqReleaseBuffer( &bfq, (Ptr)buf );
	}
	
	// *********************************************************
	//  Test 3
	// *********************************************************	
	// Slow read and fast writes -> Writer is waiting for the reader
	// and the queue is full most of the time.	
	
	TSK_sleep(1);
	
	for (i=0; i<8; i++)
	{
		assertLED( bfqGetBuffer( &bfq, (Ptr*)&buf, 0 ) );
		assertLED( buf[0] == i + 32 );
		TSK_sleep(1);
		bfqReleaseBuffer( &bfq, (Ptr)buf );
	}
	
	// done.	
	bfqDelete(&bfq);
	
	LOG_printf(&trace, "libBufferQueue test completed succesfully.");
}


void BufferQueueTest_tsk2_funct()
{
	Int 	i;
	char 	* buf;
	
	// *********************************************************
	//  Test 2
	// *********************************************************
	
	// write buffers to the queue for test2. There must always be
	// an empty buffer ready, since the writer task is waiting in between
	// two writes and the reader is not.
	for (i=0; i<8; i++)
	{
		assert( bfqAllocBuffer( &bfq, (Ptr*)&buf, 0 ) );
		TSK_sleep(1);
		buf[0] = i + 16;
		bfqPutBuffer( &bfq, (Ptr)buf);
	}
	
	// *********************************************************
	//  Test 3
	// *********************************************************
	// Write buffers to the queue fast. The writer will block and
	// wait for the reader.
	for (i=0; i<8; i++)
	{
		assert( bfqAllocBuffer( &bfq, (Ptr*)&buf, SYS_FOREVER ) );		
		buf[0] = i + 32;
		bfqPutBuffer( &bfq, (Ptr)buf);
	}
		
}
