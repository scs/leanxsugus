
/**
* @file 
*
* @brief The Buffer Queue Library
* @author Bernhard Mäder
*
* A library for buffer transportation in queues. The BufferQueue handles buffer allocation and freeing of the buffer that belong to 
* the queue. The queue object itself is allocated by the user prior to calling bfqCreate(). It may either be allocated statically or
* dynamically.
*
* Since the frames that are used for the queue mechanism are allocated withing the queue object (and will most probably be allocated
* statically), there is a limit concerning the maximum number of buffers in a queue (BFQ_MAXBUFCOUNT).
*
* The data flow withing the pipe is from its end to the front:
* 	- 	At the end, the applciation may
*		-	alloc() empty buffers to write data to them.
*		-	put() full buffers to the queue. Those buffers will flow through the pipe towards the front.
*		.
*		Each alloc() must be followed by a put, eventually. An application may allocate multiple buffers before it puts them back to
*		the queue.
*	-	At the front, the application may:
*		-	get() a full buffer and process it.
*		-	release() it afterwards.
*		.
*		Similarly, a call to get() must be followed by a call to release().
* 
* Actually, there are 3 queues withing the queue, one for transporting full buffers, one for transporting empty buffers back and one for
* keeping free frames.
*
* WARNING: there's a bug in the queue that will result in a faulty MEM_free() call and thus in a system failure if the queue is
*		   being deleted without releasing and putting all buffers back to the queue. In fact, one buffer may be checked out, but
*		   not more. That behaviour is because buffers are not receiving the same frames when they are returned to the queue.
*	       Workarounds (either one will solve the problem): 
*			-#	Don't delete the queue.
*			-#	Return all buffers before closing the queue.
*/

#ifndef _LIBBUFFERQUEUE_H_
#define _LIBBUFFERQUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>
#include <sem.h>
#include <que.h>
#include <mem.h>

#include <assert.h>

//#define BFQ_MAXBUFCOUNT		16

#ifdef _TESTBUFFERQUEUE
#define _BFQ_INIT_BUFFERS					///< If defined, the buffers are initialized with 0xFF, which helps while debugging.
#endif

/**
* A frame that is used for accessing the DSP/BIOS' QUE module.
*/
typedef struct BufferQueue_Frame
{
	QUE_Elem	queElem;					///< Required first element. See the QUE module description.
	Ptr			Buffer;						///< A pointer to the data storage buffer.
} BufferQueue_Frame;

/**
* The BufferQueue object.
*/
typedef struct BufferQueue
{
	QUE_Obj				queFullBuffers;		///< A queue with full buffers, which are transported in the data stream direction (to the receiver).
	QUE_Obj				queEmptyBuffers;	///< A queue with the empty buffers, heading the other direction, back to the sender.
	QUE_Obj				queEmptyFrames;		///< A queue to collect empty frames.
	
	SEM_Obj				semFullBuffers;		///< A semaphore which counts the number of full buffers in the queue.
	SEM_Obj				semEmptyBuffers;	///< A semaphore which counts the number of empty buffers in the queue.
	
	Int					BufSize;			///< The size of the buffers.
	Int					BufCount;			///< The number of buffers.
	Int					SegId;				///< The segment ID of the memory segment the buffers are allocated in.
	
	BufferQueue_Frame	* Frames;			///< A pointer to an array of BufferQueue_Frames, which must contain BufCount frames.
	
	Bool				bDynamicFrames;		///< A flag to indicate whether the frames were allocated dynamically or statically.
	
} BufferQueue, * BufferQueue_Handle;

/**
* Creates a buffer queue. Allocates all buffers needed in a specific memory segment, with a specific size. 
* As opposed to the other bfqCreate function, this one also creates the frames needed for the buffers, they're
* allocated in segment 0, since fast access is required.
*/
Bool 		bfqCreate_d( BufferQueue_Handle queue, Int BufCount, Int BufSize, Int SegID);

/**
* Creates the buffer queue. Allocates all buffers needed in a specific memory segment, with a specific size. The user must
* specify a pointer to an array of frames, which must be allocated prior to calling bfqCreate. The array must have BufCount elements
* (it may have more, but the excess frames would not be used). The purpose of which is to have
* the possibility to <b>statically</b> allocate queues of different sizes. The frames may as well be allocated dynamically, though.
* All other needed memory is located in the BufferQueue object and must be allocated by the usser using MEM_alloc or statically.
*
* Example of properly creating a queue:
*
* <code> 
* BufferQueue_Frame myFrames[myQueueSize]; <br>
* BufferQueue       myQueue; <br>
*
* bfqCreate( &myQueue, myFrames, myQueueSize, bufsize, segid); </code>
*
* @param		queue		A handle to the queue that should be created. The handle may point to a statically allocated object as
*							well as to a dynamically allocated one. Note that bfqCreate() does <b>not</b> allocate the memory for
*							the object itself, this must be done in advance and by the user.
* @param		frames		A pointer to an array of BufferQueue_Frames on which the queue may operate.
* @param		BufCount	The number of buffers that must be allocated within the queue. Must not exceed BFQ_MAXBUFCOUNT.
* @param		BufSize		The size of each of the allocated buffers.
* @param		SegID		The segment ID of the memory where the buffers must be allocated.
* @return					TRUE if succesful, FALSE otherwise.
*/
Bool		bfqCreate			( BufferQueue_Handle queue, BufferQueue_Frame * frames, Int BufCount, Int BufSize, Int SegID);

/**
* Deletes the dynamically allocated elements (i.e. only the buffers) of the buffer queue.
*
* @param		queue		A handle to the queue object.
*/
void 		bfqDelete			( BufferQueue_Handle queue );

/**
* Gets a full buffer from the front of the queue. The function may be either blocking or non-blocking, depending on the value of
* timeout. A value of 0 will return immediately, whereas SYS_FOREVER will wait until there is a new buffer ready. Everything in 
* between will maximally wait for the specified number of ticks until it returns.
*
* @param		queue		A handle to the queue object.
* @retval		buffer		A handle to the buffer pointer. The new buffer pointer will be written here.
* @param		timeout		The timeout specifier, in system ticks.
* @return					TRUE, if a new buffer could be got, FALSE if not (i.e. *buffer didn't change).
*/
Bool		bfqGetBuffer		( BufferQueue_Handle queue, Ptr * buffer, Uns timeout );

/** 
* Releases a buffer after it has been processed.
*
* @param		queue		A handle to the queue object.
* @param		buffer		A handle to the buffer.
*/
void		bfqReleaseBuffer 	( BufferQueue_Handle queue, Ptr buffer );

/**
* Drops the buffer on the front of the queue (if any), so that there is a new empty buffer for the writing
* task to use.
*
* @param		queue		A handle to the queue object.
* @return					TRUE if succesful, FALSE otherwise.
* Naah, don't use this, it's not thread-safe.
*/
//Bool		bfqDropFrontBuffer	( BufferQueue_Handle queue );

/** 
* Gets the count of full buffers in the queue.
*
* @param		queue		A handle to the queue object.
* @return					The number of full buffers in the queue.
*/
Int			bfqGetCount			( BufferQueue_Handle queue );

/** 
* Gets the count of empty buffers in the queue.
*
* @param		queue		A handle to the queue object.
* @return					The number of empty buffers in the queue.
*/
Int			bfqGetEmptyCount	( BufferQueue_Handle queue );


/**
* Get a new empty buffer from the end of the queue.
*
* @param		queue		A handle to the queue object.
* @retval		buffer		A handle to the buffer pointer. The new buffer pointer will be written here.
* @param		timeout		The timeout specifier, in system ticks.
* @return					TRUE, if a new buffer could be got, FALSE if not (i.e. *buffer didn't change).
*/
Bool		bfqAllocBuffer		( BufferQueue_Handle queue, Ptr * buffer, Uns timeout );

/**
* Get a new empty buffer from the end of the queue. If there is no empty buffer, it drops the front (i.e. full)
* buffer and allocates that buffer. If there is no front buffer either (i.e. all buffers are either allocated or
* got), it returns FALSE.
*
* @param		queue		A handle to the queue object.
* @retval		buffer		A handle to the buffer pointer. The new buffer pointer will be written here.
* @param		timeout		The timeout specifier, in system ticks.
* @return					TRUE, if a new buffer could be got, FALSE if not.
*/
Bool		bfqForceAllocBuffer	( BufferQueue_Handle queue, Ptr * buffer, Uns timeout );

/**
* Puts a newly filled buffer and puts it on the queue.
*
* @param		queue		A handle to the queue object.
* @param		buffer		A handle to the buffer.
*/
void		bfqPutBuffer		( BufferQueue_Handle queue, Ptr buffer );

#ifdef __cplusplus
}
#endif

#endif
