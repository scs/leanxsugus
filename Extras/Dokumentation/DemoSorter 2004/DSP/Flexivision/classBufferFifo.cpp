
#include "classBufferFifo.h"
#include "libEDMAManager.h"
#include "libDebug.h"

// *************************************************************************

CBufferFifo::CBufferFifo( )
{	
	m_pPool = NULL;
	
	m_nBufferSize = 1;
	
	m_nNumBuffers = 0;
	m_nNextFree = 0;
	m_nNextFull = 0;
	
	// Initialize our lock
	SEM_new( &m_semLock, 1 );
}

// *************************************************************************

CBufferFifo::~CBufferFifo( )
{
	if ( m_pPool != NULL )
		Free();	
}

// *************************************************************************

void CBufferFifo::Alloc( const Int nMemorySegment, const int nMemorySize )
{
	m_nPoolSize = nMemorySize;
	m_nPoolSegment = nMemorySegment;
	m_pPool = MEM_alloc( nMemorySegment, nMemorySize, 0 );
}

// *************************************************************************

void CBufferFifo::Free()
{
	MEM_free( m_nPoolSegment, m_pPool, m_nPoolSize );
}

// *************************************************************************
	
void CBufferFifo::Configure( Int nBufferSize )
{
	m_nBufferSize = nBufferSize;
	
	m_nNumBuffers = m_nPoolSize / nBufferSize;
	m_nNextFree = 0;
	m_nNextFull = 0;
	
	dbgLog("BufferFifo: configured % kb Pool to %d buffers of %d kb", m_nPoolSize/1024, m_nNumBuffers, m_nBufferSize );
}

// *************************************************************************
	
Bool CBufferFifo::Put( const Ptr pBuffer )
{
	SEM_pend( &m_semLock, SYS_FOREVER );
	
	// See if we got still a buffer free.
	Int nextfree_ahead = GetNextIndex( m_nNextFree );	
	if ( nextfree_ahead == m_nNextFull )
	{
		dbgLog("BufferFifo: full");
		SEM_post( &m_semLock );
		return FALSE;
	}
		
	// Copy
	Ptr pDest = (Uint8*)m_pPool + m_nBufferSize*m_nNextFree;	
	edmaWaitCopy( edmaStartCopy( pDest, pBuffer, m_nBufferSize, FALSE) );
	
	// Advance index
	m_nNextFree = nextfree_ahead;
	
	SEM_post( &m_semLock );	
	
	dbgLog("BufferFifo: Put() done, count is now: %d", Count() );
	
	return TRUE;		
}

// *************************************************************************

Bool CBufferFifo::Get( Ptr pBuffer )
{		
	SEM_pend( &m_semLock, SYS_FOREVER );
	
	// See if there is a full buffer
	if ( m_nNextFull == m_nNextFree )
	{
		SEM_post( &m_semLock );
		return FALSE;
	}
		
	// Copy
	Ptr pSrc = (Uint8*)m_pPool + m_nBufferSize*m_nNextFull;	
	edmaWaitCopy( edmaStartCopy( pBuffer, pSrc, m_nBufferSize, FALSE ) );
	
	// Advance index
	m_nNextFull = GetNextIndex( m_nNextFull );
	
	SEM_post( &m_semLock );	
	return TRUE;
}

// *************************************************************************

void CBufferFifo::Clear(  )
{		
	SEM_pend( &m_semLock, SYS_FOREVER );
	
	m_nNextFree = 0;
	m_nNextFull = 0;
	
	SEM_post( &m_semLock );		
}

// *************************************************************************

Int CBufferFifo::Count()
{
	int nf = m_nNextFree;
	if ( nf < m_nNextFull )
		nf += m_nNumBuffers;
		
	return ( nf-m_nNextFull );
}

// *************************************************************************

Int CBufferFifo::BufferSize()
{
	return m_nBufferSize;
}

// *************************************************************************
	
Int CBufferFifo::GetNextIndex( const int index ) const
{
	int i = index+1;
	
	if ( i>=m_nNumBuffers )
		i=0;
		
	return i;
}

// *************************************************************************




