
#include "classViewChannel.h"
#include "libDebug.h"

#include <stdio.h>
#include <string.h>

#include <csl_cache.h>

// *************************************************************************

CViewChannel::CViewChannel()
{
	m_pVision = NULL;
	m_bEnabled 	= FALSE;
	m_bTakeSnapshot = FALSE;
	m_bWaitEvent = FALSE;
	m_bCollectEventPictures = FALSE;
}

// *************************************************************************

CViewChannel::~CViewChannel()
{
	bfqDelete( &m_qQueue );
}

// *************************************************************************

Bool CViewChannel::Init( int nChannelNum, CVisVision * vision, Int nNumBuffers, Int nBufsize, Int nSeg, Int nCollectorSize )
{
	m_nChannelNum = nChannelNum;
	
	// Store the vision pointer
	m_pVision = vision;	
	
	// Create the queue
	if ( ! bfqCreate_d( &(m_qQueue), nNumBuffers, nBufsize, nSeg ) )
		return FALSE;
		
	m_nDropRate = 32;
	m_nCurFrame = 0;
	m_pCurrentPicture = 0;
	
	m_bEnabled 	= FALSE;
	m_bTakeSnapshot = FALSE;
	m_bWaitEvent = FALSE;
	m_bCollectEventPictures = FALSE;
		
	// Allocate the first buffer
	AllocBuffer( );
	
	// Allocate collector if needed
	if (nCollectorSize > 0)
	{
		m_bHaveEventPicturesCollector = TRUE;
		m_EventPicturesCollector.Alloc( nSeg, nCollectorSize );
	}
	else
		m_bHaveEventPicturesCollector = FALSE;
		
	return TRUE;
}

// *************************************************************************

Bool CViewChannel::Configure( const char * strPort, int nDropRate)
{
	char strcmp[32];
	char strprt[32];
	Int i;
	Uint32 w,h,bpp;
	bool ind;
	bool data;
	
	// Return if not yet initialized
	if ( m_pVision == NULL )
		return FALSE;
	
	// Parse the port string. Find the '.'
	i = 0;
	while ( strPort[i] != '.')
	{
		i++;
		if ( i== 32 )
			return FALSE;	
	}

	// Copy the sub strings.
	strcpy( strcmp, strPort );		
	strcmp[i] = '\0';
	
	strcpy( strprt, strPort + i + 1 );
	
	if ( ! m_pVision->EnableViewPort( m_nChannelNum, strcmp, strprt ) )
		return FALSE;
		
	if ( ! m_pVision->GetViewPortImageInfo( m_nChannelNum, w, h, bpp, data, ind ) )
		return FALSE;

	// Is this a data port? Then we must set the pictype accordingly
	if ( data )
		m_nPicType 	= PICT_DATA;
	else
		m_nPicType 	= picGetType( bpp, ind );

	// The rest of the parameters are the same for real pictures and data
	// buffers.	
	m_nCurFrame = 0;
	m_nDropRate = nDropRate;
	m_nWidth 	= w;
	m_nHeight 	= h;	
	m_nPicSize 	= picGetPictureSize( 	m_nPicType,
										m_nWidth,
										m_nHeight );
	m_bEnabled 	= FALSE;
	m_bTakeSnapshot = FALSE;
	
	// Setup the collector if there is one
	if ( m_bHaveEventPicturesCollector )
	{
		// Only re-configure the collector if the size differs.
		if ( m_EventPicturesCollector.BufferSize() != m_nPicSize )
			m_EventPicturesCollector.Configure( m_nPicSize );
	}
		

	return TRUE;		
}

// *************************************************************************

Bool CViewChannel::TakeSnapshot()
{
	if ( m_bEnabled )
		return FALSE;
		
	// Set the flag and wait for a picture
	m_bTakeSnapshot = TRUE;
	
	return TRUE;
}

// *************************************************************************

Bool CViewChannel::TakeEventSnapshot( const Int nEventId )
{
	if ( m_bEnabled )
		return FALSE;
		
	// Set the flag and wait for an event
	m_bWaitEvent = TRUE;	
	m_nEventId = nEventId;

	return TRUE;	
}

// *************************************************************************

Bool CViewChannel::Enable( Bool bEnable )
{
	// If it is about to be enabled, reset the counter
	if ( bEnable )
		m_nCurFrame = 0;
	
	// Set the flag accordingly
	m_bEnabled	= bEnable;
	
	// Clear the snapshot flag, just in case...
	m_bTakeSnapshot	= FALSE;
	m_bWaitEvent = FALSE;
			
	return TRUE;
}

// *************************************************************************

Bool CViewChannel::IsEnabled()
{
	return m_bEnabled;
}

// *************************************************************************

Bool CViewChannel::GetPicture( Picture * & pPicture )
{
	Bool 	bRes;
	Ptr 	p;
		
	bRes = bfqGetBuffer( 	&(m_qQueue), 
							&p, 
							0 );
							
	if ( bRes )
		pPicture = (Picture*)p;
	
	return bRes;
}

// *************************************************************************

Bool CViewChannel::ReleasePicture( Picture * pPicture )
{
	bfqReleaseBuffer( &(m_qQueue), (Ptr)pPicture);
	
	return TRUE;
}

// *************************************************************************

Bool CViewChannel::AllocBuffer( )
{
	Bool 	bRes;
	Ptr		p;
	
	// Only allow this, if there is no current buffer.
	if ( m_pCurrentPicture != NULL)
	{
		dbgLog("VC: AllocBuffer: buffer already allocated!");
		return FALSE;
	}
		
	bRes = bfqAllocBuffer( &(m_qQueue), &p, 0 );
	
	if ( ! bRes )
		return FALSE;		
	
	// Store the buffer in the channel info.
	m_pCurrentPicture = (Picture * )p;	
	
	return TRUE;
}

// *************************************************************************

Bool CViewChannel::PutAllocBuffer( )
{
	Picture * pic;
	
	if ( m_pCurrentPicture == NULL )
	{
		dbgLog("VC: PutAllocBuffer: No buffer allocated!");
		return FALSE;
	}
	
	// Store a reference to the buffer
	pic = m_pCurrentPicture;
	
	// Now try to allocate a new buffer, and STAY WITH THE OLD ONE,
	// if that fails
	m_pCurrentPicture = NULL;
	if ( ! AllocBuffer( ) )
	{
		m_pCurrentPicture = pic;
		return FALSE;
	}
	
	// Successful, a new buffer is available in m_pCurrentPicture,
	// we now just need to fill the previous picture structure and put it to the queue.
		
	// Write the picture header.
	pic->TotalSize 		= m_nPicSize;
	pic->TotalWidth 	= m_nWidth;
	pic->TotalHeight 	= m_nHeight;
	pic->OffsetX 		= 0;
	pic->OffsetY 		= 0;
	pic->Width 			= m_nWidth;
	pic->Height 		= m_nHeight;
	pic->Type 			= m_nPicType;
	pic->Timestamp 		= 0;			
	
	CACHE_invL1d( pic, (PIC_HEADERSIZE+3) & ~0x00000003, CACHE_WAIT );

	bfqPutBuffer( &(m_qQueue) , pic);
		
	return TRUE;
}

// *************************************************************************

void CViewChannel::FeedBuffer( )
{
	// Return if not yet initialized
	if ( m_pVision == NULL )
		return;
		
	if ( m_bEnabled || m_bTakeSnapshot || m_bWaitEvent )
	{
		m_pVision->FeedViewPortBuffer( m_nChannelNum, m_pCurrentPicture->Data );
	}
}

// *************************************************************************

void  CViewChannel::ConcludeBuffer( )
{
	if ( m_bEnabled )
	{			
		// If the current frame counter reaches the number of frames to drop,
		// put the current frame to the queue and reset the counter
		if ( m_nCurFrame >= m_nDropRate )
		{
			// Put the buffer and get a new one.
			PutAllocBuffer();
			m_nCurFrame = 0;
		}
		else
		{
			// else, drop the current image by just not putting it to the
			// queue.
			m_nCurFrame++;
		}
	}
	// If the channel is not enabled, see if a snapshot is requested
	else if ( m_bTakeSnapshot )
	{
		// Clear the flag, since we only want a single image.
		m_bTakeSnapshot = FALSE;
		
		// And put the picture to the queue.
		PutAllocBuffer();
	}
	// If we're waiting for the event, there are two possibilities for getting a picture:
	//  1) There are still pictures left in the collector, which need to be sent now
	//  2) There is a direct picture coming in
	else if ( m_bWaitEvent )
	{
		// First see if we got an image in the collector, which we have to favour
		// above the current picture in the buffer. So, we need
		// to add additional images to the collector as well.
		if ( m_bHaveEventPicturesCollector && (m_EventPicturesCollector.Count() > 0) )
		{
			// Put the image to the collector, if there is a new one.
			if ( m_pVision->EventOccured( m_nEventId ) )
			{
				dbgLog("Picture AND collector not empty");	
				m_EventPicturesCollector.Put( m_pCurrentPicture );
			}
			
			dbgLog("Got picture from collector");	
			
			// Get the next image from the collector and add it to the queue
			m_EventPicturesCollector.Get( m_pCurrentPicture );
			PutAllocBuffer();
			
			// Switch to collecting mode
			m_bWaitEvent = FALSE;			
			m_bCollectEventPictures = TRUE;
		}
		
		// If there are no images on the collector, we can use the incoming image directly, if
		// there is one.
		else if ( m_pVision->EventOccured( m_nEventId ) )
		{				
			//dbgLog("Direct Event picture");	
			// And put the picture to the queue.
			PutAllocBuffer();	
			
			// Switch to collecting mode
			m_bWaitEvent = FALSE;	
			if ( m_bHaveEventPicturesCollector )		
				m_bCollectEventPictures = TRUE;
		}
	}
	// We're in collecting mode, so we just have to test for the event and, in case there
	// is one, put it in the collector.
	else if ( m_bCollectEventPictures )
	{
		// If there is an event at this time, we don't put it directly to
		// the queue, but rather to the collector.
		if ( m_bHaveEventPicturesCollector && m_pVision->EventOccured( m_nEventId ) )
		{
			dbgLog("Collected event picture");	
			m_EventPicturesCollector.Put( m_pCurrentPicture );
		}
		
		
	}
	
}

// *************************************************************************

// *************************************************************************

// *************************************************************************
