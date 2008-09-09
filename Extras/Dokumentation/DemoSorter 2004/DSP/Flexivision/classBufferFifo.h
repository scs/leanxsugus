/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSBUFFERFIFO_H_
#define _CLASSBUFFERFIFO_H_

#include "FlexiVisioncfg.h"

#include "sem.h"

/**
* @brief
*
*
*/
class CBufferFifo
{
public:
						CBufferFifo();
						~CBufferFifo();
						
	void				Alloc( const Int nMemorySegment, const int nMemorySize );						
	void				Free();
	
	
	void				Configure( Int nBufferSize );
	
	
	Bool				Put( const Ptr pBuffer );
	Bool				Get( Ptr pBuffer );
	void				Clear();
	Int					Count();
	Int					BufferSize();
	
	
protected:
	Int					GetNextIndex( const int index ) const ; 
	
	/** The reference to the memory pool for this fifo. */
	Ptr					m_pPool;
	
	Int					m_nPoolSize;
	Int					m_nPoolSegment;
	
	/** The size of the buffers in the pool.*/
	Int					m_nBufferSize;
	
	Int					m_nNumBuffers;
	Int					m_nNextFree;
	Int					m_nNextFull;
	
	SEM_Obj				m_semLock;

};


#endif


