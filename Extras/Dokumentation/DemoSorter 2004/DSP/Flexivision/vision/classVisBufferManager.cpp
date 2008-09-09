
#include "classVisBufferManager.h"

#include <stdio.h>

// Additional windows stuff.
#ifdef _WINDOWS
#include <stdlib.h>
Uint32 SDRAM = 1;
Uint32 SDRAM_cached = 2;
#endif

// Allocate the single instance of this object
CVisBufferManager CVisBufferManager::TheManager;

extern Uint32 SDRAM;
extern Uint32 SDRAM_cached;

// *************************************************************************

CVisBufferManager::CVisBufferManager()
	: CVisObject("BufferManager", CVisObject::CT_BUFFER_MANAGER)
{		
	for (Int i=0; i<BUF_MAX_BUFFERS; i++)
		m_aryRequests[i].pObject = NULL ;
}

// *************************************************************************

CVisBufferManager::~CVisBufferManager()
{
	// Free all buffers
	for (Int i=0; i<BUF_MAX_BUFFERS; i++)
	{
		if ( m_aryRequests[i].pObject != NULL )
		{
			RequestInfo * req = &(m_aryRequests[i]);

#ifdef _WINDOWS
	free( m_aryRequests[i].pBuffer);
#else
			MEM_free( m_aryRequests[i].unSegId, m_aryRequests[i].pBuffer, m_aryRequests[i].unSize );
#endif
			m_aryRequests[i].pObject = NULL;
		}
	}
}

// *************************************************************************

CVisBufferManager * CVisBufferManager::Instance()
{
	return &TheManager;
}

// *************************************************************************

bool CVisBufferManager::RequestBuffer( CVisObject * pObject, Uint32 unSize, Ptr * ppBuffer, Uint32 unFlags, Int nOverlapId )
{
	Uint32 segId;
	
	// DEBUG:: Add a complete line
	if ( unSize > 1024 )
		unSize += 4096;

	// round size up to double-word
	unSize = ( unSize + 7) & ~(Uint32)0x07;

	// Add a guardword
	unSize += 8;
	
	// Determine memory segment
	if ( ( (unFlags & BUF_FAST) != 0) || ( (unFlags & BUF_LARGE) == 0))
		segId = 0;
	
	else if ( (unFlags & BUF_NON_CACHED) != 0)
		segId = SDRAM;
	
	else
		segId = SDRAM_cached;
	
	// Allocate buffer	
#ifdef _WINDOWS
	*ppBuffer = malloc(unSize);
#else
 	*ppBuffer = MEM_alloc( segId, unSize, 8);
#endif

	// See if there was an error.
	if ( *ppBuffer == NULL )
	{
		// Log the failure
		if ( pObject->GetClassType() == CT_PORT )
		{
			// Get additional info if this is a port
			LogMsg("Alloc %dkB on %d for '%s.%s' failed", unSize/1024, segId, ((CVisPort*)pObject)->GetComponent()->GetName(), pObject->GetName());
		}
		else
		{
			// Just write the name of the requesting object if it's not a port
			LogMsg("Alloc %dkB on %d for '%s' failed", unSize/1024, segId, pObject->GetName());
		}
		return false;	 
	}
		
	// Store request infos
	Int i=0;
	while ( m_aryRequests[i].pObject != NULL)
	{
		i++;
		if (i==BUF_MAX_BUFFERS)
			return false;
	}
	
	m_aryRequests[i].pObject = pObject ;
	m_aryRequests[i].unSegId = segId;
	m_aryRequests[i].pBuffer = *ppBuffer;
	m_aryRequests[i].unSize = unSize;	
	
	// Write two guard words to the end of the buffer
	Uint32 * p = (Uint32*)*ppBuffer;
	p[unSize/4-2] = BUF_GUARD;
	p[unSize/4-1] = BUF_GUARD;

	return true;
}

// *************************************************************************

bool CVisBufferManager::GetStatistic( Uint32 & unBytesSRAM, Uint32 & unBytesSDRAM, Uint32 & unBytesSDRAMCached )
{	
	unBytesSRAM = 0;
	unBytesSDRAM = 0;
	unBytesSDRAMCached = 0;
	
	for (Int i=0; i<BUF_MAX_BUFFERS; i++)
	{
		if ( m_aryRequests[i].pObject != NULL )
		{
			Uint32 unSize =	m_aryRequests[i].unSize;	
			
			if ( m_aryRequests[i].unSegId == 0 )
				unBytesSRAM += unSize;
				
			else if ( m_aryRequests[i].unSegId == SDRAM )
				unBytesSDRAM += unSize;	
				
			else if ( m_aryRequests[i].unSegId == SDRAM_cached )
				unBytesSDRAMCached += unSize;
		}
	}
	
	return true;
}

// *************************************************************************
	
bool CVisBufferManager::CheckMemoryBoundaries()
{
	bool bRes = TRUE;

	for (Int i=0; i<BUF_MAX_BUFFERS; i++)
	{
		if ( m_aryRequests[i].pObject != NULL )
		{
			RequestInfo * req = &(m_aryRequests[i]);

			// Check the guard words.
			Uint32 * p = (Uint32*)req->pBuffer;
			if (	p[req->unSize/4-2] != BUF_GUARD
				||	p[req->unSize/4-1] != BUF_GUARD )
			{
				CVisObject * pParent = NULL;

				if ( req->pObject->GetClassType() == CVisObject::CT_PORT )
					pParent = (CVisObject*)(((CVisPort*)req->pObject)->GetComponent());
				
				// DEBUG: disable output
				/*
				LogMsg("Guard word error! Obj: %s.%s, Buffer: %p, Size: %d., is: %08X %08X instead of %08x", 
						(pParent == NULL) ? "" : pParent->GetName(),
						req->pObject->GetName(),
						req->pBuffer,
						req->unSize,
						p[req->unSize/4-2],
						p[req->unSize/4-1],
						BUF_GUARD);
				*/
				
				// Re-write guard words
				p[req->unSize/4-2] = BUF_GUARD;
				p[req->unSize/4-1] = BUF_GUARD;

				bRes = FALSE;
			}
		}
	}

	return bRes;
}

// *************************************************************************
	
// *************************************************************************

