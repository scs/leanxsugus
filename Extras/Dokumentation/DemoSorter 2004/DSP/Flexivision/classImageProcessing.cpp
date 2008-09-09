#include "classImageProcessing.h"

#include <stdio.h>

#include <clk.h>

#include "drvHighResTimer.h"

#include "libLED.h"
#include "libEDMAManager.h"
#include "libDebug.h"
#include "libHelpers.h"

#include "classOutputDispatcher.h"
#include "classWatchdog.h"

#include "vision/classVisGraphics.h"

/** Define the SDRAM segment which is configured by the config tool. */
extern Int SDRAM;											
extern Int SDRAM_cached;

// *************************************************************************

CImageProcessing::CImageProcessing( ) :
	m_Control(0)
{
	m_pVision = NULL;
	
	m_nMaxFPS = 1000;
	
	// Set the frame time to 38'000 us
	m_nFrameTime = 36700;
	
	
	m_bEnableProcessing = TRUE;
}

// *************************************************************************

CImageProcessing::~CImageProcessing()
{
}

// *************************************************************************

void CImageProcessing::SetControl( CControl * control )
{
	m_Control = control;
}

// *************************************************************************

void CImageProcessing::StartTask()
{
	Picture	* 	pic;
	Uint32		res;	
	Uint32		unCurrentImageTime = 0;
	Uint32		unNextImageTime = 0;	
	Uint32		unTotalTimeMs = 0;
	
#ifndef _SIMULATE
	TSK_sleep( 20000 );
#endif
	
	edmaInitManager();	
	
	// Open the PPU driver		
	m_hPPU = ppuOpen( NULL );	
	if ( m_hPPU == NULL )
		return;
		
	dbgLog(" FPGA version:");
	dbgLog(" %d/%d/%d", ppuReadRegister( m_hPPU, 0xFE), ppuReadRegister( m_hPPU, 0xFF), ppuReadRegister( m_hPPU, 0xFD) );
	dbgLog("------------------------------");
	
	// Allocate the potato vision ojects
	m_pVision = new CVision();
	
	// Initialize the output dispatcher
	COutputDispatcher::Instance()->Init();
			
	// Initialize the view channel. Cannot do this in the constructor since this object
	// is allocated statically and thus MEM_create may not be called in the constructor.
	InitViewChannels();	
		
	// Add to watchdog
	m_unWatchId = CWatchdog::Instance()->AddToWatch( "ImageProcessing", 4 );
	CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
	
	#define IMG_WIDTH	(1380)
	#define IMG_HEIGHT	(256)
	#define FRAMESIZE	4*1380//2*1380
	
	// Open channel 0
	ppuOpenChannel( m_hPPU, 0, 32, IMG_WIDTH, IMG_HEIGHT, 3, FRAMESIZE );
	ppuEnableChannel( m_hPPU, 0 );
	
	// Tell the controller that we're ready.
	ppuIoCtl( m_hPPU, PPU_IOCTL_SET_READY_LINE, 1 );
	
	// Calibrate the camera
	ppuIoCtl( m_hPPU, PPU_IOCTL_SET_GAIN, PIC_RGB( 40, 0, 30 ));
	
	// Setup Profiler
	m_unOverheadProfilerId = CProfiler::Instance()->NewProfileTask( "Overhead" );
	m_unMainProfileId = CProfiler::Instance()->NewProfileTask( "Image Processing" );
	
	CProfiler::Instance()->StartProfileTask( m_unOverheadProfilerId );
	
	while(1)
	{
		// Reset watchdog counter
		CWatchdog::Instance()->SignalAlive( m_unWatchId );
		
		// Update the timing counters
		unNextImageTime = timeGetHighResTime();			
		unTotalTimeMs += timeToMs( unNextImageTime - unCurrentImageTime );		
		unCurrentImageTime = unNextImageTime;		
			
		// Start acquisition of next image.
		ppuIoCtl( m_hPPU, PPU_IOCTL_TRIGGER_IMG_ACQUISITION, 0 );			

		// Try to get a picture.
		res = ppuGetPicture( m_hPPU, 0, &pic, 5000 );
		
		// Is there a picture ?
		if ( res == SYS_OK )
		{			
			// DEBUG:
			/*
			static int delme=0;
			delme++;
			
			COutputDispatcher::Instance()->Channel( delme & 0x0F ).AddCommand( unNextImageTime + timeFromMs(100), true );
			COutputDispatcher::Instance()->Channel( delme & 0x0F ).AddCommand( unNextImageTime + timeFromMs(160), false );
			
			
			if ( ((delme+1) & 0x01) == 0 )
			{
				COutputDispatcher::Instance()->Channel( 29 ).AddCommand( unNextImageTime + timeFromMs(100), true );
				COutputDispatcher::Instance()->Channel( 29 ).AddCommand( unNextImageTime + timeFromMs(110), false );
			}
			*/
			// /DEBUG
		
			// Only go on if processing is enabled.
			if ( m_bEnableProcessing == TRUE )
			{						
								
				// Set the time 
				//m_pVision->SetCurrentImageTime( unTotalTimeMs );
				m_pVision->SetCurrentImageTime( unCurrentImageTime );
								
				// Feed the buffer to the vision object
				m_pVision->FeedImage( pic->Data );
				
				// Feed the viewport buffers to the vision object
				FeedViewChannelBuffers();
				
				// Do the processing
				CProfiler::Instance()->StopProfileTask( m_unOverheadProfilerId );
				CProfiler::Instance()->StartProfileTask( m_unMainProfileId );				
				m_pVision->DoProcessing();				
				CProfiler::Instance()->StopProfileTask( m_unMainProfileId );
				CProfiler::Instance()->StartProfileTask( m_unOverheadProfilerId );

				// And conclude the viewchannel buffers by first copying them out of the network and
				// then putting them to the queues.
				m_pVision->CopyViewPorts();
				ConcludeViewChannelBuffers();
				
			} // if enabled
									
			// Release picture
			ppuReleasePicture( m_hPPU, 0, pic);
		} // if picture
		else
		{
			// Here, we've got a timeout of the picture get function, so we'll just
			// trigger a new picture and try again.
			ppuIoCtl( m_hPPU, PPU_IOCTL_TRIGGER_IMG_ACQUISITION, 0 );
		}

		// Now wait until we reach the desired framerate
		Uint32 curTime;
		do
		{
			TSK_sleep(1);			
			curTime = timeGetHighResTime();
		} while( timeToUs(curTime - unCurrentImageTime) < m_nFrameTime );
		
		/*
		dbgLog( "Total time: %d ms, Frametime: %d ms, now: %d ms, delta: %d ms", 
					unTotalTimeMs, timeToMs( unCurrentImageTime ), timeToMs( timeGetHighResTime() ),
					timeToMs( timeGetHighResTime() - unCurrentImageTime ) );
		*/
				
	} // while(1)
	
}


// *************************************************************************

CViewChannel * CImageProcessing::GetViewChannel( const int nChannel )
{
	// Return if the channel number is not valid
	if ( nChannel > VIEW_CHANNELS_NUM-1)
		return NULL;
		
	return & (m_aryViewChannels[nChannel]);	
}

// *************************************************************************

Bool CImageProcessing::EnumOutputPorts( Int & nIndex, const Char * & strComponent, const Char * & strPort )
{
	return m_pVision->EnumOutputPorts( nIndex, strComponent, strPort );
}

// *************************************************************************

Bool CImageProcessing::AccessProperty( const Bool bSet, const Char * strComponent, const Char * strProperty, float & fValue )
{
	return m_pVision->AccessProperty( bSet, strComponent, strProperty, fValue );
}

// *************************************************************************

Bool CImageProcessing::SetProperty( const Char * strComponent, const Char * strProperty, const float fValue )
{
	float f = fValue;
	return m_pVision->AccessProperty( true, strComponent, strProperty, f );
}

// *************************************************************************

Bool CImageProcessing::EnumProperties( Int & nIndex, const Char * & strComponent, const Char * & strProperty, float & fValue )
{
	return m_pVision->EnumProperties( nIndex, strComponent, strProperty, fValue );
}

// *************************************************************************

Bool CImageProcessing::SetMode( ImageProcessingMode ipmMode )
{
	// Disable processing so that we've got enough processor time to do the calibration
	// data preparations.
	m_bEnableProcessing = FALSE;
	
	switch ( ipmMode )
	{
		case IPM_IDLE:
			m_pVision->ChangeMode( CVision::VM_IDLE );		
			break;
			
		case IPM_SERVICE:
			m_pVision->ChangeMode( CVision::VM_SERVICE );
			break;
			
		case IPM_PROCESSING:
			m_pVision->ChangeMode( CVision::VM_CLASSIFICATION );
			break;
			
		case IPM_CALIBRATION:
			m_pVision->ChangeMode( CVision::VM_CALIBRATION );
			break;
			
		default:
			break;
	}
	
	// Now enable processing again.
	m_bEnableProcessing = TRUE;
	
	return TRUE;
	
}

// *************************************************************************

CImageProcessing::ImageProcessingMode CImageProcessing::GetMode( )
{
	switch( m_pVision->GetMode() )
	{
		case CVision::VM_IDLE:
			return IPM_IDLE;
			
		case CVision::VM_SERVICE:
			return IPM_SERVICE;
			
		case CVision::VM_CLASSIFICATION:
			return IPM_PROCESSING;
			
		case CVision::VM_CALIBRATION:
			return IPM_CALIBRATION;
			
		default:
			return IPM_IDLE;
	}
}

// *************************************************************************

void CImageProcessing::AdjustColorValues( Int nRed, Int nGreen, Int nBlue )
{
	Uint8 r,g,b;
	
	r = (nRed * 128) / 100;
	g = (nGreen * 128) / 100;
	b = (nBlue * 128) / 100;
	ppuIoCtl( m_hPPU, PPU_IOCTL_SET_GAIN, PIC_RGB( r,g,b ));
}

// *************************************************************************

void CImageProcessing::SetMaxFramerate( Int nMaxFPS )
{
	m_nMaxFPS = nMaxFPS;
}

// *************************************************************************

void CImageProcessing::SetFrameTime( Int nFrameTime_us )
{
	m_nFrameTime = nFrameTime_us;
}

// *************************************************************************

Bool CImageProcessing::InitViewChannels()
{
	Int 	i;
	Bool 	b;	
	
	b = TRUE;
	
	// Go through all view channels and initialize them.
	for ( i=0; i<VIEW_CHANNELS_NUM; i++)
	{
		Uint32		size;
		Uint32		collectorSize;
		
		// Create the buffer queue. Channel 0 is different and provides larger buffers
		if ( i == 0)
		{
			size = VIEW_CHANNEL0_MAXIMGSIZE;
			collectorSize = 20 * 1024 * 1024;
		}
		else
		{
		 	size = VIEW_CHANNELS_MAXIMGSIZE;
		 	collectorSize = 0;
		 }
		 	
		// Add some extra (safety) memory
		size += 256;
		
		// Initialize (DEBUG: don't use the collector yet.)
		b = b & m_aryViewChannels[i].Init( i, m_pVision, VIEW_CHANNELS_DEPTH, size, SDRAM, 0/*collectorSize*/ );		
	}
	
	return b;	
}


// *************************************************************************

void CImageProcessing::FeedViewChannelBuffers()
{
	Int 	i;
	
	// Go through all view channels and feed the buffers to the vision object
	for ( i=0; i<VIEW_CHANNELS_NUM; i++)
	{
		m_aryViewChannels[i].FeedBuffer();		
	}
}

// *************************************************************************

void CImageProcessing::ConcludeViewChannelBuffers()
{
	Int 	i;
	
	// Go through all view channels and conclude the buffers
	for ( i=0; i<VIEW_CHANNELS_NUM; i++)
	{
		m_aryViewChannels[i].ConcludeBuffer();		
	}
}

// *************************************************************************
