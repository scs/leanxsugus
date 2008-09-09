/**
* @file
* @brief The Ring Buffer Library
* @author Bernhard Mäder
*
* A library for a simple fifo-type ring buffer. Data can be written to the ringbuffer on one side and read from
* from the other side. The ring buffer may operate on either single characters as well as on whole strings. The basic
* data type is a character.
*
* Definitions are, that the buffer is empty, when the read and the write index point to the same location. Likewise,
* the buffer is full, when the write index points to a location of one less than the read pointer. This eliminates
* the need of storing a full/empty flag, since the distinction is easily made using the pointers. The downside is that
* one character of the buffer cannot be used for storage and the maximum numbers of stored characters is buffersize-1.
*/
#ifndef _LIBRINGBUFFER_H_
#define _LIBRINGBUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>
#include <mem.h>

#include <assert.h>

/**
* Holds the control data for the ring buffer.
*/
typedef struct RingBuffer
{
	Int		ReadIndex;		///< The current read index
	Int		WriteIndex;		///< The current write index
	Int		BufferSize;		///< The ring buffer's buffer size
	Int		SegID;			///< The segment id of the buffer, -1 if the buffer wasn't allocated by the ring buffer.
	char 	* Buffer;		///< A pointer to the storage buffer.
} RingBuffer, * RingBuffer_Handle;


/**
* Creates a ring buffer structure. This does not allocate any memory, neither for data storage, nor for
* the control data. It just sets the ring buffer structure up and initializes the pointers.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @param	buffer		A pointer to the storage buffer
* @param	bufsize		The size of the storage buffer, in number of characters.
*/
Bool		rbfCreate	( RingBuffer_Handle ringbuf, char * buffer, Int bufsize);

/**
* Creates a ringbuffer structure, allocating the needed memory dynamically.
*/
Bool		rbfCreate_d	( RingBuffer_Handle ringbuf, Int segid, Int bufsize);

/**
* Deletes the ring buffer. Deallocates the storage buffer, <b>if</b> it has been allocated in rbfCreate_a().
*
* @retval	ringbuf		A handle to the ringbuffer structure.
*/
void		rbfDelete	( RingBuffer_Handle ringbuf );

/**
* Reads a string from the ringbuffer.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @retval	dest		The destination buffer where the characters should be written to.
* @param	numchars	The number of chars that should be read from the ringbuffer.
* @return				The number of chars actually read from the ringbuffer.
*/
Int			rbfRead		( RingBuffer_Handle ringbuf, char * dest, Int numchars);

/**
* Reads a single character from the ringbuffer.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @retval	dest		A pointer to the char to which the character should be written to.
* @return				TRUE, if a character could be read, FALSE otherwise (buffer was empty).
*/
Bool		rbfReadChar ( RingBuffer_Handle ringbuf, char * dest );

/**
* Peeks on a single char from the ringbuffer without actually removing it.
*/
Bool		rbfPeekChar ( RingBuffer_Handle ringbuf, char * dest );

/**
* Writes a string to the ringbuffer.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @param	src			A pointer to the string that should be inserted into the ringbuffer.
* @param	numchars	The number of characters in src.
* @return				The number of chars actually written to the ringbuffer.
*/
Int			rbfWrite	( RingBuffer_Handle ringbuf, const char * src, Int numchars);

/**
* Writes a single character to the ringbuffer.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @param	src			The character that should be written to the buffer.
* @return				TRUE, if the character could be written, FALSE otherwise (buffer was full).	
*/
Bool		rbfWriteChar( RingBuffer_Handle ringbuf, char src );

/**
* Returns the number of characters the ringbuffer could additionally take.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @return				The number of free characters in the storage buffer.
*/
Int			rbfNumEmpty	( RingBuffer_Handle ringbuf );

/**
* Returns the number of characters in the ringbuffer.
*
* @retval	ringbuf		A handle to the ringbuffer structure.
* @return				The number of characters in the storage buffer.
*/
Int			rbfNumFull	( RingBuffer_Handle ringbuf );

/**
* Flushes the ringbuffer leaving it empty.
*/
void		rbfFlush( RingBuffer_Handle ringbuf );

#ifdef __cplusplus
}
#endif

#endif

