
#include "libRingBuffer.h"

#include <string.h>

#define RBF_INC_INDEX(index, bufsize) 		( index + 1 == bufsize ? 0 : index + 1)
//#define RBF_ADD_INDEX(index, add, bufsize) 	( index + add >= bufsize ? index + add - bufsize : index + add)

// *************************************************************************

Bool		rbfCreate	( RingBuffer_Handle ringbuf, char * buffer, Int bufsize)
{
	ringbuf->ReadIndex = 0;
	ringbuf->WriteIndex = 0;
	ringbuf->BufferSize = bufsize;
	ringbuf->SegID = -1;
	ringbuf->Buffer = buffer;
	
	return TRUE;
}

// *************************************************************************


Bool		rbfCreate_d	( RingBuffer_Handle ringbuf, Int segid, Int bufsize)
{
	Ptr p;
	
	p = MEM_alloc( segid, bufsize, 8 );
	
	rbfCreate( ringbuf, (char*)p, bufsize );
	
	ringbuf->SegID = segid;
	
	return TRUE;
}


// *************************************************************************

void		rbfDelete	( RingBuffer_Handle ringbuf )
{
	// If the buffer was created with create_a, delete it here
	if (ringbuf->SegID != -1)
	{	
		MEM_free( ringbuf->SegID, ringbuf->Buffer, ringbuf->BufferSize );
	}
}

// *************************************************************************

Int			rbfRead		( RingBuffer_Handle ringbuf, char * dest, Int numchars)
{
	Int readable;
	Int read;
	Int read_first;
	
	// Determine, how many chars of the requested can actually be read.
	readable = rbfNumFull( ringbuf );
	read = ( numchars > readable ? readable : numchars );
	
	if (read == 0) return 0;
	
	// Copy the characters - distinguish two cases.
	if ( (ringbuf->ReadIndex + read) > ringbuf->BufferSize)
	{		
		// read to the end of the buffer from the actual read position
		read_first = ringbuf->BufferSize - ringbuf->ReadIndex;
		memcpy((Ptr)dest, (Ptr)(ringbuf->Buffer + ringbuf->ReadIndex), read_first );
		
		// read the rest, from the start of the buffer.
		memcpy((Ptr)(dest + read_first), (Ptr)ringbuf->Buffer, read-read_first);
		
		// adjust the index
		ringbuf->ReadIndex = read-read_first;
	}
	else
	{
		// Read the chars and adjust the index.
		memcpy((Ptr)dest, (Ptr)(ringbuf->Buffer + ringbuf->ReadIndex), read );
		ringbuf->ReadIndex += read;
	}
		
	return read;
}

// *************************************************************************

Bool		rbfReadChar ( RingBuffer_Handle ringbuf, char * dest )
{
	if ( ringbuf->ReadIndex != ringbuf->WriteIndex )
	{
		// read char
		*dest = ringbuf->Buffer[ringbuf->ReadIndex];
	
		// increment index and do wrap-around	
		ringbuf->ReadIndex = RBF_INC_INDEX(ringbuf->ReadIndex, ringbuf->BufferSize);
			
		return TRUE;
	}
	
	return FALSE;
}

// *************************************************************************

Bool		rbfPeekChar ( RingBuffer_Handle ringbuf, char * dest )
{
	if ( ringbuf->ReadIndex != ringbuf->WriteIndex )
	{
		// Peek char
		*dest = ringbuf->Buffer[ringbuf->ReadIndex];
		return TRUE;
	}
	
	return FALSE;
}

// *************************************************************************

Int			rbfWrite	( RingBuffer_Handle ringbuf, const char * src, Int numchars)
{
	Int writable;
	Int write;
	Int write_first;
	
	volatile Int d1,d2;
	
	// Determine, how many chars of the requested can actually be written.
	writable = rbfNumEmpty( ringbuf );
	write = ( numchars > writable ? writable : numchars );
	
	if (writable == 0) return 0;
	
	// Copy the characters - distinguish two cases.
	if ( (ringbuf->WriteIndex + write) > ringbuf->BufferSize)
	{		
		// write to the end of the buffer from the actual write position
		write_first = ringbuf->BufferSize - ringbuf->WriteIndex;
		memcpy((Ptr)(ringbuf->Buffer + ringbuf->WriteIndex), (Ptr)src, write_first );
		
		// write the rest, from the start of the buffer.
		memcpy((Ptr)ringbuf->Buffer, (Ptr)(src + write_first), write-write_first);
		
		// adjust the index
		ringbuf->WriteIndex = write-write_first;
	}
	else
	{
		// Write the chars and adjust the index.
		memcpy((Ptr)(ringbuf->Buffer + ringbuf->WriteIndex), (Ptr)src,  write );
		ringbuf->WriteIndex += write;
		
		// If the write index is equal to the buffersize, handle that
		if ( ringbuf->WriteIndex == ringbuf->BufferSize )
			ringbuf->WriteIndex = 0;
	}
		
	return write;
	
}

// *************************************************************************

Bool		rbfWriteChar( RingBuffer_Handle ringbuf, char src )
{
	Int next_index;
	
	next_index = RBF_INC_INDEX(ringbuf->WriteIndex, ringbuf->BufferSize);
	
	if ( next_index != ringbuf->ReadIndex )
	{
		// write char
		ringbuf->Buffer[ringbuf->WriteIndex] = src;
		
		// increment index
		ringbuf->WriteIndex = next_index;
		
		return TRUE;
	}
	
	return FALSE;
}

// *************************************************************************

Int			rbfNumEmpty	( RingBuffer_Handle ringbuf )
{
	return (ringbuf->BufferSize - rbfNumFull( ringbuf ) - 1);
}

// *************************************************************************

Int			rbfNumFull	( RingBuffer_Handle ringbuf )
{
	Int res;
	// Determine, how many chars of the requested can actually be read.
	if ( ringbuf->WriteIndex >= ringbuf->ReadIndex)	
		res = ringbuf->WriteIndex - ringbuf->ReadIndex;							// no wrap-around
	else
		res = (ringbuf->BufferSize - ringbuf->ReadIndex) + ringbuf->WriteIndex;	// wrap-around
	
	return res;
}

// *************************************************************************

void		rbfFlush( RingBuffer_Handle ringbuf )
{
	// Let the read index reach the write index
	ringbuf->ReadIndex = ringbuf->WriteIndex = 0;
}

// *************************************************************************
