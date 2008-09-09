
#include "classControl.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <csl_irq.h>
#include <clk.h>

#include "classStaticString.h"

#include "libHelpers.h"
#include "libLED.h"
#include "libBufferQueue.h"

#include "drvPPU.h"
#include "drvPPUSerial.h"

// DEBUG::
#include "tstEDMAManager.h"

#include "classWatchdog.h"
#include "classStatus.h"

// Define the version information
#define VERSION_PROJECT 		"SCS DemoSorter"
#define VERSION_MAJOR			0
#define VERSION_MINOR			1
#define VERSION_ADDITION		"alpha"

#define REPLY_QUEUE_SIZE		128
#define REPLY_QUEUE_BUFFERSIZE	256

#define DEBUG_QUEUE_SIZE		128
#define DEBUG_QUEUE_BUFFERSIZE	512


extern Int SDRAM;								///< Define the SDRAM segment which is configured by the config tool.
extern Int SDRAM_cached;						///< Define the SDRAM segment which is configured by the config tool.

//HPI_DevHandle 				CControl::m_hHPI;
//CImageProcessing *			CControl::m_pImageProcessing;

/**
* Import the global control object.
*/
extern CControl g_Control;

/**
* Define the function type for the message handlers.
*/
typedef Bool (CControl::*HandlerFunction)(const Char * strArgs );

/**
* Define the structure for the handler table. 
*/
typedef struct CommandHandler
{
	const Char * 		strCommandText;
	HandlerFunction		fHandler;
	const Char *		strSyntax;
} CommandHandler;

     
// *************************************************************************
// The command map
// *************************************************************************
/**
* Define the map of string commands to CControl member functions. 
* All functions must be defined the same way.
*/
CommandHandler g_CommandHandlers [] =
{	
	{	"help", 			&CControl::cmdHelp,			"help" },
	{	"mode",				&CControl::cmdMode,			"mode [idle/processing/service/calibration]"},
	{	"chan", 			&CControl::cmdChannel,		"chan [open/close/snap/event/enum] nChan strComp.strPort [nDropRate/nEvent]" },
	{	"prop", 			&CControl::cmdProperty, 	"prop [set/get/enum] strComp.strProperty nValue" },
	{	"stats", 			&CControl::cmdStats, 		"stats [reset]" },
	{	"cam",				&CControl::cmdCamera,		"cam [setlevels/send] [red green blue/sendstr]"},
	{	"ping",			 	&CControl::cmdPing,			"ping" },
	{	"status",			&CControl::cmdStatus,		"status" },
	{ 	"profile",			&CControl::cmdProfile,		"profile"},
	{ 	"log",				&CControl::cmdLog,			"log strLogtext"},
	{	"debug",			&CControl::cmdDebug,		"debug [on/off/maxfps/frametime] [fps/frametime_us]" },
	{	"service",			&CControl::cmdService,		"service [jets]" },
	{	"test",				&CControl::cmdTest,			"test nTestnum" },
	{	"testcopy",			&CControl::cmdTestCopy,		"testcopy" }	
	};

/**
* Define the number of commands in the map.
*/
#define NUM_COMMANDS ( sizeof(g_CommandHandlers) / sizeof(CommandHandler) )

    
// *************************************************************************

// TODO: these functions goes into the cmdCalibration class, if there will be one.
Uint8 HexToInt( const char * strHex )
{
	Uint8 c = 0;
	
	// high nibble
	if ( (strHex[0] >= '0') && (strHex[0] <= '9' ) )
		c += (strHex[0] - '0') << 4;
	else
		c += ((strHex[0] - 'a') + 10) << 4;
		
	// low nibble
	if ( (strHex[1] >= '0') && (strHex[1] <= '9' ) )
		c += strHex[1] - '0';
	else
		c += (strHex[1] - 'a') + 10;			
		
	return c;
}
    
// *************************************************************************

void IntToHex( const Uint8 u, char * strHex )
{
	sprintf(strHex, "%02x", u );
}
    
// *************************************************************************


CControl * CControl::m_pInstance = NULL;
     
// *************************************************************************


CControl::CControl( CImageProcessing * pImageProcessing )
{
	m_pImageProcessing = pImageProcessing;	
	
	// Create the reply queue
	bfqCreate_d( &m_qReplyMsgQueue, REPLY_QUEUE_SIZE, REPLY_QUEUE_BUFFERSIZE, SDRAM );
	
	// Create the debug buffer queue and its lock semaphore
	bfqCreate_d( &m_qDebugMsgQueue, DEBUG_QUEUE_SIZE, DEBUG_QUEUE_BUFFERSIZE, SDRAM);
	SEM_new( &m_semDebugMsgQueueLock, 1);
	
	m_pInstance = this;
	
	m_pCalibData = NULL;
	m_bDebug = TRUE;
}
     
// *************************************************************************

CControl::~CControl()
{
}

// *************************************************************************

CControl * CControl::Instance()
{
	return m_pInstance;
}

// *************************************************************************

void CControl::StartTask()
{
	ledInit();
	/*
	ledLight( 0, TRUE );	
	ledLight( 1, TRUE );
	ledLight( 2, TRUE );
	*/
	ledLight( 3, TRUE );
	
	
	// Init the edma manager
	edmaInitManager();
	
	#ifndef _SIMULATE
	TSK_sleep( 10000 );
	#endif
	
	ledLight( 3, FALSE );
	
	// Open the etrax messaging driver and allocate the receive packet buffer
	m_hHPI = hpiOpen( 4 );
	m_hpiPacket_in = (HPI_Packet*)MEM_alloc( 0, sizeof( HPI_Packet ), 0 );	
	
	// Allocate the large output buffer that (at the moment) is only used by the calibration command
	m_hpiPacket_out = (HPI_Packet*)MEM_alloc( 0, sizeof( HPI_Packet ), 0 );	
	
	// And write the welcome message
	PostDebugMsg( " %s %d.%d %s", VERSION_PROJECT, VERSION_MAJOR, VERSION_MINOR, VERSION_ADDITION );
	PostDebugMsg( " %s %s", __DATE__, __TIME__ );
	PostDebugMsg("------------------------------");
	
	// Configure the camera
	if ( ! InitCamera() )
		PostDebugMsg("Camera configuration failed.");

	// Add to watchdog
	m_unWatchId = CWatchdog::Instance()->AddToWatch( "Control", 20 );
	CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
	
	// ***********************************************
	//  Enter main message processing loop
	// ***********************************************		
	while(1)
	{
		// Handle and process host messages
		HandleHostMessages();
		
		// Send all viewchannel images. Note: it is important that this is done BEFORE
		// SendBufferedTextMessages(), since that function is likely to flood the entire
		// out packet queue.
		SendViewChannelPictures();
		
		// First send all buffered reply messages.
		Bool bDone;
		bDone = SendBufferedMessages( &m_qReplyMsgQueue );
		
		// Then, if there is still room in the output queue and debug is turned on,
		// send the debug messages with lowest priority.
		if ( m_bDebug && bDone )
			SendBufferedMessages( &m_qDebugMsgQueue );		
			
		// Wait some time, yield time to lower priority tasks.
		TSK_sleep(10);	
		
		// Clear watchdog register
		CWatchdog::Instance()->SignalAlive( m_unWatchId );
	}	
}

// *************************************************************************

Bool CControl::InitCamera()
{
	// The camera configuration string
	const char * CAM_CONFIG_STR	= "TR=1\r\nRC=0\r\nSH=6\r\nSC=2\r\nGA=130\r\n";
	const char * CAM_REPLY_STR = "COMPLETE\r\nCOMPLETE\r\nCOMPLETE\r\nCOMPLETE\r\nCOMPLETE\r\n";
	
	char str[256];
	Int read;
	
	// Open the serial channel 2, which leads to the camera and configure it.	
	m_hCameraSerial = serOpen( 2, 2048 );
	serConfigChannel( m_hCameraSerial, 9600, FALSE, FALSE, FALSE );	
	
	// Flush the UART
	serFlushRx( m_hCameraSerial );	
	
	// Write the configuration string to the camera
	serWrite( m_hCameraSerial, CAM_CONFIG_STR, strlen(CAM_CONFIG_STR) );
	TSK_sleep( 1000 );
	
	// Read its response
	read = serRead( m_hCameraSerial, str, 255 );
	str[read] = 0;
	
	// Compare the response
	if ( strcmp( str, CAM_REPLY_STR ) != 0 )
	{
		PostDebugMsg("Read %s instead of %s", str, CAM_REPLY_STR );
		return FALSE;
	}
	else
		return TRUE;

}

// *************************************************************************

void CControl::HandleHostMessages()
{

	// Message received from host?
	while (hpiGetMessageCount( m_hHPI ) != 0)
	{
		// Get the message
		hpiGetMessage( m_hHPI, m_hpiPacket_in, SYS_FOREVER);
		
		// Add a trailing \0 to the end of the packet
		// so that no string operation runs to infinity.
		((Uint8 *)(m_hpiPacket_in))[sizeof(HPI_Packet)-1] = 0;
				
		// Now parse it
		Int cmdInd;
		Int offs;
		offs = ParseCommand( m_hpiPacket_in->Data.Command.txtTxt, cmdInd );
		
		// Branch to the message handler if we found any
		Bool bRes;
		if ( offs != 0)
		{
			HandlerFunction fn = (g_CommandHandlers[cmdInd].fHandler);
			bRes = (g_Control.*fn)( m_hpiPacket_in->Data.Command.txtTxt + offs );
			
			if ( !bRes )
			{
				PostReplyMsg( "Wrong arguments: '%s'", m_hpiPacket_in->Data.Command.txtTxt + offs );
				PostReplyMsg( "Syntax: %s", g_CommandHandlers[cmdInd].strSyntax );
			}						
		}
		else
		{
			PostReplyMsg( "Wrong command" );
		}
		
		PostReplyMsg( "Done" );
	}		
}

// *************************************************************************

Bool CControl::cmdHelp( const Char * strCommandText )
{
	for ( Int i=0; i<NUM_COMMANDS; i++)
		PostReplyMsg( g_CommandHandlers[i].strSyntax );
	
	return TRUE;
}


// *************************************************************************

Bool CControl::cmdMode( const Char * strCommandText )
{
	Char 	strMode[128];
	Int 	nNumArgs;
	
	const char * strIdle = "idle";
	const char * strService = "service";
	const char * strProcessing = "processing";
	const char * strCalibration = "calibration";
	
	
	nNumArgs = sscanf( strCommandText, "%s", strMode );
	
	// If no mode is specified, the current mode is read back.
	if ( nNumArgs < 1 )
	{
		const char * strCurrentMode = "unknown";
		switch( m_pImageProcessing->GetMode() )
		{
			case CImageProcessing::IPM_IDLE:
				strCurrentMode = strIdle;
				break;
				
			case CImageProcessing::IPM_SERVICE:
				strCurrentMode = strService;
				break;
				
			case CImageProcessing::IPM_PROCESSING:
				strCurrentMode = strProcessing;
				break;
				
			case CImageProcessing::IPM_CALIBRATION:
				strCurrentMode = strCalibration;
				break;
		}	
		
		PostReplyMsg("mode = %s", strCurrentMode );
		return TRUE;
	}
		
	
	// *******************************************
	//  idle
	// *******************************************
	if ( strcmp( strMode, strIdle ) == 0 )
	{
		if ( ! m_pImageProcessing->SetMode( CImageProcessing::IPM_IDLE ) )
			PostReplyMsg("Error!");
	}
	
	// *******************************************
	//  processing
	// *******************************************
	else if ( strcmp( strMode, strProcessing ) == 0 )
	{
		if ( ! m_pImageProcessing->SetMode( CImageProcessing::IPM_PROCESSING ) )
			PostReplyMsg("Error!");
	}
	
	// *******************************************
	//  service
	// *******************************************
	else if ( strcmp( strMode, strService ) == 0 )
	{
		if ( ! m_pImageProcessing->SetMode( CImageProcessing::IPM_SERVICE ) )
			PostReplyMsg("Error!");
	}
	
	// *******************************************
	//  calibration
	// *******************************************
	else if ( strcmp( strMode, strCalibration ) == 0 )
	{
		if ( ! m_pImageProcessing->SetMode( CImageProcessing::IPM_CALIBRATION ) )
			PostReplyMsg("Error!");
	}
	else
		return FALSE;
		
	return TRUE;
	
}

// *************************************************************************

Bool CControl::cmdChannel( const Char * strCommandText )
{
	Char 	strSubcmd[128];
	Int		nChannel;
	Char	strPort[128];
	Int		nArg1;
	Int 	nNumArgs;
	CViewChannel * chan;
	
	nNumArgs = sscanf( strCommandText, "%s %d %s %d", strSubcmd, &nChannel, strPort, &nArg1 );
	
	if ( nNumArgs < 1 )
		return FALSE;
		
	// *******************************************
	//  open
	// *******************************************
	if ( strncmp( strSubcmd, "open", 4 ) == 0 )
	{		
		if ( nNumArgs < 4 )
			return FALSE;		
				
		chan = m_pImageProcessing->GetViewChannel( nChannel ) ;
		
		// Abort on error
		if ( chan == NULL )
			return TRUE;
		
		if ( ! chan->Configure( strPort, nArg1 ) )
		{
			PostReplyMsg("ERROR: Could not configure port.");
			return TRUE;
		}
		
		if ( ! chan->Enable( TRUE ) )
		{	
			PostReplyMsg("ERROR: Could not enable viewport.");
			return TRUE;
		}
		return TRUE;
	}
	
	// *******************************************
	//  snapshot
	// *******************************************
	if ( strcmp( strSubcmd, "snap" ) == 0 )
	{		
		if ( nNumArgs < 3 )
			return FALSE;
						
		chan = m_pImageProcessing->GetViewChannel( nChannel ) ;
		
		// Abort on error
		if ( chan == NULL )
			return TRUE;
		
		// Don't allow this if the channel is already in use.
		if ( chan->IsEnabled() )
		{
			PostReplyMsg("Channel already in use!");
			return TRUE;
		}
		
		if ( ! chan->Configure( strPort, 3000 ) )
		{
			PostReplyMsg("ERROR: Could not configure port.");
			return TRUE;
		}
		
		if ( ! chan->TakeSnapshot() )
		{	
			PostReplyMsg("ERROR: Could not take snapshot");
			return TRUE;
		}
		return TRUE;
	}
	
	// *******************************************
	//  event
	// *******************************************
	if ( strncmp( strSubcmd, "event", 9 ) == 0 )
	{		
		if ( nNumArgs < 4 )
			return FALSE;
		
		chan = m_pImageProcessing->GetViewChannel( nChannel ) ;
		
		// Abort on error
		if ( chan == NULL )
			return TRUE;
		
		// Don't allow this if the channel is already in use.
		if ( chan->IsEnabled() )
		{
			PostReplyMsg("Channel already in use!");
			return TRUE;
		}
		
		if ( ! chan->Configure( strPort, 3000 ) )
		{
			PostReplyMsg("ERROR: Could not configure port. ");
			return TRUE;
		}
		
		if ( ! chan->TakeEventSnapshot( nArg1 ) )
		{	
			PostReplyMsg("ERROR: Could not wait for event");
			return TRUE;
		}
		return TRUE;
	}
	
	// *******************************************
	//  close
	// *******************************************
	if ( strncmp( strSubcmd, "close", 5 ) == 0 )
	{		
		if ( nNumArgs < 2 )
			return FALSE;		
				
		chan = m_pImageProcessing->GetViewChannel( nChannel ) ;
		
		// Abort on error
		if ( chan == NULL )
			return TRUE;
		
		chan->Enable( FALSE );
		return TRUE;
	}
	
	// *******************************************
	//  enum
	// *******************************************
	if ( strncmp( strSubcmd, "enum", 4 ) == 0 )
	{	
		Int ind = -1;
		const Char * strprt;
		const Char * strcmp;
			
		PostReplyMsg( "numChannels = %d", VIEW_CHANNELS_NUM );
		
		while ( m_pImageProcessing->EnumOutputPorts( ind, strcmp, strprt ) )
			PostReplyMsg("%s.%s", strcmp, strprt );
		
		return TRUE;
	}
	
	return FALSE;
}

// *************************************************************************

Bool CControl::cmdProperty( const Char * strCommandText )
{
	Char 			strSubcmd[128];
	
	const Char * 	strprop;
	const Char * 	strcmp;
			
	Char			strProperty[128];
	
	float			fValue;
	Int				nValue;
	Int				nNumArgs;
	
	nNumArgs = sscanf( strCommandText, "%s %s %f", strSubcmd, strProperty, &fValue );
	CStaticString	sstrProperty( strProperty, 128 );
	
	if ( nNumArgs < 1 )
		return FALSE;
		
	// *******************************************
	//  set
	// *******************************************
	if ( strncmp( strSubcmd, "set", 3 ) == 0 )
	{		
		// If the floating point value could not be parsed, try to look for an integer instead
		if ( nNumArgs == 2 )
		{			
			nNumArgs = sscanf( strCommandText, "%s %s %d", strSubcmd, strProperty, &nValue );
			if ( nNumArgs < 3 )
				return FALSE;		
				
			fValue = (float)nValue;
		}
		
		sstrProperty.FindDivide( '.', strprop );
		strcmp = sstrProperty.GetString();
		
		m_pImageProcessing->AccessProperty( TRUE, strcmp, strprop, fValue );
		
		//PostReplyMsg( "set %s.%s = %d", strcmp, strprop, nValue );

		return TRUE;
	}
	
	// *******************************************
	//  get
	// *******************************************
	if ( strncmp( strSubcmd, "get", 3 ) == 0 )
	{		
		if ( nNumArgs < 2 )
			return FALSE;
		
		sstrProperty.FindDivide( '.', strprop );
		strcmp = sstrProperty.GetString();
		
		m_pImageProcessing->AccessProperty( FALSE, strcmp, strprop, fValue );
		
		PostReplyMsg( "prop %s.%s = %f", strcmp, strprop, fValue );

		return TRUE;
	}
	
	// *******************************************
	//  enum
	// *******************************************
	if ( strncmp( strSubcmd, "enum", 4 ) == 0 )
	{	
		Int ind = -1;
		float f;
		
		while ( m_pImageProcessing->EnumProperties( ind, strcmp, strprop, f ) )
			PostReplyMsg("prop %s.%s = %f", strcmp, strprop, f );
		
		return TRUE;
	}

	return FALSE;
}

// *************************************************************************

Bool CControl::cmdStats( const Char * strCommandText )
{
	Int 					nNumArgs;
	char 					strSubCmd[128];
	Bool					bReset = FALSE;

	return TRUE;	
}

// *************************************************************************

Bool CControl::cmdCamera( const Char * strCommandText )
{
	Char 			strSubcmd[128];
	#define 		MAX_RESPONSE_SIZE 128
	const char 		strEndline [] = "\r\n";
	char 			strResponse[MAX_RESPONSE_SIZE];
	Int32			nRedLevel, nGreenLevel, nBlueLevel;
	Int				nRead;
	Int				nNumArgs;
	
	// read sub command	
	nNumArgs = sscanf( strCommandText, "%s %d %d %d", strSubcmd, &nRedLevel, &nGreenLevel, &nBlueLevel );
	
	if ( nNumArgs < 1 )
		return FALSE;
		

	// *******************************************
	//  set rgb levels
	// *******************************************
	if ( strncmp( strSubcmd, "setlevels", 9 ) == 0 )
	{
		if ( nNumArgs < 4 )
			return FALSE;
		
		m_pImageProcessing->AdjustColorValues( nRedLevel, nGreenLevel, nBlueLevel );
		
		return TRUE;
	}
	
	// *******************************************
	//  write to camera via serial line.
	// *******************************************
	if ( strncmp( strSubcmd, "send", 4 ) == 0 )
	{
		// TODO: do this +-5 right.
		serWrite( m_hCameraSerial, strCommandText + 5, strlen( strCommandText ) - 5 );
		
		PostReplyMsg("written: '%s'", strCommandText+5 );
		
		serWrite( m_hCameraSerial, strEndline, 2 );
		// Wait some time.
		TSK_sleep( 4000 );
		
		// Read the response
		nRead = serRead( m_hCameraSerial, strResponse, MAX_RESPONSE_SIZE-1 );
		if ( nRead > 0)
		{
			// terminate string
			strResponse[nRead] = 0;
			
			// and bring to screen
			PostReplyMsg( strResponse );
		}	
		
		return TRUE;
	}
	
	return FALSE;

}
     
// *************************************************************************

Bool CControl::cmdPing( const Char * strCommandText )
{
	Int i;
	
	for ( i=0; i<10; i++)
	{
		PostReplyMsg( "Pong %d", i );
	}
	
	for ( i=0; i<100; i++)
	{
		PostReplyMsg( "log<< (%00d) This is a longer log message, so we may see whether that works as well.", i);
	}
	
	return TRUE;
}

// *************************************************************************

Bool CControl::cmdStatus( const Char * strCommandText )
{
	CStatus::Instance()->Log( this );
	
	return TRUE;
}

// *************************************************************************

Bool CControl::cmdProfile( const Char * strCommandText )
{
	Int 	task;
	char 	str[128];
	Int 	nNumArgs;
	Bool	bReset;
	
	task = -1;
	
	// Parse for keywords
	nNumArgs = sscanf( strCommandText, "%s", str);
	
	// Look for the reset keyword
	bReset = FALSE;
	if ( nNumArgs > 0 )
	{
		if ( strncmp( str, "reset", 5 ) == 0 )
			bReset = TRUE;
	}
	
	// Note: we just POST the messages and don't directly send them, since there
	// might be a large size of messages to send.	
	PostReplyMsg(" ----------- Profile ----------- ");
	
	// Enumerate all tasks from the profiler and print their info.
	while ( CProfiler::Instance()->EnumTasks( task ) )
	{
		CProfiler::Instance()->GetTaskInfo( task, str, 128 );
		PostReplyMsg( str );
		
		if ( bReset )
			CProfiler::Instance()->ResetProfileTask( task );
	}
	
	return TRUE;		
}

// *************************************************************************

Bool CControl::cmdLog( const Char * strCommandText )
{
	PostDebugMsg( "log<< %s", strCommandText );
	return TRUE;
}

// *************************************************************************

Bool CControl::cmdDebug( const Char * strCommandText )
{
	Int 	nNumArgs;
	Int		nArg;
	char 	strSubcmd[128];
	
	// Parse for the subcommand
	nNumArgs = sscanf( strCommandText, "%s %d", strSubcmd, &nArg );
	
	if ( nNumArgs < 1 )
		return FALSE;
	
	
	// -------------------------
	//  identify the subcommand	
	// -------------------------
	
	// on
	if ( strcmp( strSubcmd, "on" ) == 0 )
	{
		m_bDebug = TRUE;
		return TRUE;
	}
	
	// off
	if ( strcmp( strSubcmd, "off" ) == 0 )
	{
		m_bDebug = FALSE;
		return TRUE;
	}
	
	// fps
	if ( strcmp( strSubcmd, "maxfps" ) == 0 )
	{
		m_pImageProcessing->SetMaxFramerate(nArg);
		return TRUE;
	}
	
	// frametime
	if ( strcmp( strSubcmd, "frametime" ) == 0 )
	{
		m_pImageProcessing->SetFrameTime(nArg);
		return TRUE;
	}

	return FALSE;
}

// *************************************************************************

Bool CControl::cmdService( const Char * strCommandText )
{
	Char 			strSubcmd[128];
	Char 			strSubSubcmd[128];
	Int				nValue1 = 0;
	Int				nValue2 = 0;
	Int				nNumArgs;
	
	nNumArgs = sscanf( strCommandText, "%s %s %d %d", strSubcmd, strSubSubcmd, &nValue1, &nValue2 );
		
	if ( nNumArgs < 1 )
		return FALSE;
		
	// *******************************************
	//  jets
	// *******************************************
	if ( strcmp( strSubcmd, "jets" ) == 0 )
	{	
		if ( strcmp( strSubSubcmd, "off" ) == 0 )	
		{
			return TRUE;
		}
		
		else if ( strcmp( strSubSubcmd, "checkjets" ) == 0 )
		{
			return TRUE;
		}
		
		else if ( strcmp( strSubSubcmd, "adjust" ) == 0 )
		{
			switch ( nValue1 )
			{
			case 0:
			case 1:
				break;
				
			case 2:
				break;
				
			case 3:
				break;
			}
		}	
		else
			return FALSE;		

	}
	
	return TRUE;
}

// *************************************************************************

Bool CControl::cmdTest( const Char * strCommandText )
{
	#define TESTSIZE 1024
	
	const char * 	teststr = "Hello UART world! This is a longer text.";
	char 			str[512];
	
	
	Uint16 * 		unpWrite;
	Uint16 * 		unpRead;
	
	#define SRAM_TESTSIZE (1<<21)
	#define SRAM_MOD(i) (~(i % 507))
	#define SRAM_INV(i) ( (i & 1) ? ~(i) : (i) )
	#define SRAM_SWAP(i) ( ((i>>8) & 0xFF) | ((i<<8) & 0xFF00))
	#define SRAM_GENTEST(i) ( SRAM_SWAP( SRAM_INV( SRAM_MOD(i))))
	//#define SRAM_GENTEST(i) ( SRAM_MOD(i))
	
	unpWrite = (Uint16*)MEM_alloc( SDRAM, sizeof(Uint16)*SRAM_TESTSIZE, 0 );
	unpRead = (Uint16*)MEM_alloc( SDRAM, sizeof(Uint16)*SRAM_TESTSIZE, 0 );
	Uint32 i;
		
	volatile Uint16 * ppuMemory = (Uint16*)0x60000000;

	
	switch( strCommandText[0] )
	{
	case '0':
		PostReplyMsg("Watchdog Test");
		TSK_sleep( 50000 );
		PostReplyMsg("Failed");
		break;
		
	case '1':
		PostReplyMsg("Time test");
		CWatchdog::Instance()->EnableWatch( m_unWatchId, FALSE );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(5000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(1000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(1000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(1000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(1000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		TSK_sleep( hlpMsToTicks(1000) );
		PostReplyMsg("Time now (ms): %d", hlpTicksToMs(CLK_getltime()) );
		CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
		break;
		
	case '2':
	/*
		PostReplyMsg("Writing....");
		ppuWriteSRAM( 0x00, (Uint16*)teststr, strlen(teststr)/2 );
		PostReplyMsg("Done... Reading...");

		ppuReadSRAM( 0x00, (Uint16*)str, strlen(teststr)/2 );
		str[ strlen(teststr) ] = 0;
		PostReplyMsg("Read: %s", str);
	*/	
		PostReplyMsg("Writing... '%s'", teststr);
		strcpy( str, "**********************************************************" );
		ppuWriteSRAM( 0x80000, (Uint16*)teststr, strlen(teststr)/2+1 );
		PostReplyMsg("Done... Reading...");
		
		ppuReadSRAM( 0x80000, (Uint16*)str, strlen(teststr)/2+1 );
		//PostReplyMsg( "Read @ 0: %04X", ((Uint16*)str)[0] );
		//str[ strlen(teststr) ] = 0;		
		/*str[ 0 ] = 'X';
		str[ 1 ] = 'X';		
		*/
		TSK_sleep( 10 );
		
		PostReplyMsg("Done: %s", str);
		
		ppuReadSRAM( 0x80000, (Uint16*)str, strlen(teststr)/2+1 );		
		PostReplyMsg("Done 2: %s", str);
		
		break;
		
	case '3':
		PostReplyMsg("PPU SRAM Test");
		Uint32 unLastError;
		Uint32 unErrors;
		
		unErrors = 0;		
		unLastError = 0;
	
		CWatchdog::Instance()->EnableWatch( m_unWatchId, FALSE );
		for ( i=0; i<SRAM_TESTSIZE; i++)
		{
			unpWrite[i] = SRAM_GENTEST(i);
			unpRead[i] = 0xbeef;
		}
		PostReplyMsg("Starting write...");
		ppuWriteSRAM( 0x00, unpWrite, SRAM_TESTSIZE );
		PostReplyMsg("Written.");
			
		ppuReadSRAM( 0x00, unpRead, SRAM_TESTSIZE );
		PostReplyMsg("Read.");
			
		for ( i=0; i<SRAM_TESTSIZE; i++)
		{
			if ( unpWrite[i] != unpRead[i] )
			{
				unErrors++;
				
				if ( unErrors < 10 )
					PostReplyMsg("Error @ %d: should: %04Xh, is: %04Xh", i, unpWrite[i], unpRead[i] );				
				unLastError = i;
			}				
		}
		if ( unErrors > 0 )
			PostReplyMsg("%d Errors found.", unErrors );
			
		if ( unLastError != 0 )
		{
			for (i=0; i<3; i++)
			{
				PostReplyMsg("Reading again...");
				unpRead[unLastError] = 0xdead;
				ppuReadSRAM( unLastError, &(unpRead[unLastError]), 1 );
				if ( unpWrite[unLastError] == unpRead[unLastError] )
					PostReplyMsg("No more error @ %d: should: %04Xh, is: %04Xh", unLastError, unpWrite[unLastError], unpRead[unLastError] );
				else
					PostReplyMsg("Error again @ %d: should: %04Xh, is: %04Xh", unLastError, unpWrite[unLastError], unpRead[unLastError] );				
			}
			
			for (i=0; i<3; i++)
			{
				PostReplyMsg("Writing again...");
				unpRead[unLastError] = 0xdead;
				ppuWriteSRAM( unLastError, &(unpWrite[unLastError]), 1 );
				ppuReadSRAM( unLastError, &(unpRead[unLastError]), 1 );
				if ( unpWrite[unLastError] == unpRead[unLastError] )
					PostReplyMsg("No more error @ %d: should: %04Xh, is: %04Xh", unLastError, unpWrite[unLastError], unpRead[unLastError] );
				else
					PostReplyMsg("Error again @ %d: should: %04Xh, is: %04Xh", unLastError, unpWrite[unLastError], unpRead[unLastError] );				
			}
		}
			
		PostReplyMsg("Done.");
		CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
		break;
		
	case '4':
		PostReplyMsg("Writing...");
		if ( ! ppuWriteSRAM( 0x00, (Uint16*)str, 120 ) )
			PostReplyMsg("Timeout!");
		else
			PostReplyMsg("Done... ");
		break;		

	case '5':
		PostReplyMsg("PPU test, using data generator");
		PostReplyMsg("Version: %d/%d/%d", ppuMemory[0xFD], ppuMemory[0xFE], ppuMemory[0xFF]);
		PostReplyMsg("Trhsh: 0x%04x", ppuMemory[0xC1]);
		PostReplyMsg("IRQ-Mask: 0x%04x", ppuMemory[0xF2]);
		
		// enable data generator
		ppuMemory[0xF3] &= ~0x04;
		
		TSK_sleep(100);
		
		// Do a 0-1 transition on the enable reg.		
		ppuMemory[0xF3] &= ~(0x01);
		for ( volatile Int k=0; k<100; k++);
		ppuMemory[0xF3] |= (0x01);
		
		// Wait some time
		TSK_sleep( 2000 );
		PostReplyMsg("wordcount low: 0x%04x", ppuMemory[0xC2]);
		PostReplyMsg("wordcount high: 0x%04x", ppuMemory[0xC3]);
		
		// Disable data generator again.				
		ppuMemory[0xF3] |= 0x04;
		break;
		
	case '6':
		PostReplyMsg("PPU test, triggering camera. NO data generator");

		// Do a 0-1 transition on the enable reg.		
		ppuMemory[0xF3] &= ~(0x01);
		for ( volatile Int k=0; k<100; k++);
		ppuMemory[0xF3] |= (0x01);
		
		// Wait some time
		TSK_sleep( 2000 );
		PostReplyMsg("wordcount low: 0x%04x", ppuMemory[0xC2]);
		PostReplyMsg("wordcount high: 0x%04x", ppuMemory[0xC3]);
		break;						
		
	case '7':
		PostReplyMsg("PPU test, triggering camera with flush at end. NO data generator");

		// Do a 0-1 transition on the enable reg.		
		ppuMemory[0xF3] &= ~(0x01);
		for ( volatile Int k=0; k<100; k++);
		ppuMemory[0xF3] |= (0x01);
		
		// Wait some time
		TSK_sleep( 2000 );
		ppuMemory[0xF3] &= ~(0x0002);
		ppuMemory[0xF3] |= 0x02;
		
		break;
		
	case '8':
		Uint16 dataReady;
		Uint16 numWords;
		volatile Uint16 data;
		
		dataReady = (ppuMemory[ 0xC0] & 0x01 );
		numWords = 0;
		while( (dataReady != 0) && (numWords < 10000) )
		{
			data = ppuMemory[ 0x00 ];
			dataReady = (ppuMemory[ 0xC0] & 0x01 );
			
			numWords++;			
		}
		PostReplyMsg("%d words read", numWords);
		break;		
		
	case '9':
		IRQ_set( IRQ_EVT_GPINT7 );
		break;
	}
	
	MEM_free( SDRAM, unpRead, sizeof(Uint16)*SRAM_TESTSIZE );
	MEM_free( SDRAM, unpWrite, sizeof(Uint16)*SRAM_TESTSIZE );	
	
	return TRUE;
}

// *************************************************************************

Bool CControl::cmdTestCopy(const Char * strCommandText )
{
	Uint8 * src;
	Uint8 * dst;
	Uint8 * cmp;
	Bool bRes;
	Bool bResTotal = TRUE;
	
	// Disable watchdog, since this test will take long.
	CWatchdog::Instance()->EnableWatch( m_unWatchId, FALSE );
	
	src = (Uint8*)MEM_alloc( SDRAM, 0x200000, 8 );
	dst = (Uint8*)MEM_alloc( SDRAM, 0x200000, 8 );
	cmp = (Uint8*)MEM_alloc( SDRAM, 0x200000, 8 );
	
	PostReplyMsg(" Copy Test");
	PostReplyMsg("----------------------");

	PostReplyMsg(" No Semaphore");
	PostReplyMsg("--------------");	
	PostReplyMsg("Plain 32 bit copy of 512 bytes");
	bRes = tstedma1DCopyTest( src, dst, 512, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
			
	PostReplyMsg("Plain 32 bit copy of 3 Frames");
	bRes = tstedma1DCopyTest( src, dst, 3*0x8000, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB");
	bRes = tstedma1DCopyTest( src, dst, 0x100000, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB + 4 bytes");
	bRes = tstedma1DCopyTest( src, dst, 0x100004, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("  Semaphore");
	PostReplyMsg("--------------");
	
	PostReplyMsg("Plain 32 bit copy of 512 bytes");
	bRes = tstedma1DCopyTest( src, dst, 512, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
			
	PostReplyMsg("Plain 32 bit copy of 3 Frames");
	bRes = tstedma1DCopyTest( src, dst, 3*0x8000, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB");
	bRes = tstedma1DCopyTest( src, dst, 0x100000, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB + 4 bytes");
	bRes = tstedma1DCopyTest( src, dst, 0x100004, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("  Unaligned");
	PostReplyMsg("--------------");
	Ptr s_u = (Ptr)(src + 1 );
	Ptr d_u = (Ptr)(dst + 1 );
	
	PostReplyMsg("Plain 32 bit copy of 512 bytes");
	bRes = tstedma1DCopyTest( s_u, d_u, 512, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
			
	PostReplyMsg("Plain 8 bit copy of 3 + 3 Bytes Frames");
	bRes = tstedma1DCopyTest( s_u, d_u, 3*0x8000+3, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB");
	bRes = tstedma1DCopyTest( s_u, d_u, 0x100000, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("Plain 32 bit copy of 1MB + 4 bytes");
	bRes = tstedma1DCopyTest( s_u, d_u, 0x100004, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("  Parallel");
	PostReplyMsg("--------------");
	
	PostReplyMsg("Plain 32 bit copy of 1MB, 8 channels parallel");
	bRes = tstedmaParallelCopyTest( src, dst, 0x200000, TRUE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("  2D");
	PostReplyMsg("--------------");
	
	PostReplyMsg("32 bit, 1D-1D");
	bRes = tstedmaCopyTest2D( EDMACOPY_1D1D, src, dst, cmp, 768, 1024, 1024, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("32 bit, 1D-2D");
	bRes = tstedmaCopyTest2D( EDMACOPY_1D2D, src, dst, cmp, 768, 1024, 1024, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("32 bit, 2D-1D");
	bRes = tstedmaCopyTest2D( EDMACOPY_2D1D, src, dst, cmp, 768, 1024, 1024, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("32 bit, 2D-2D");
	bRes = tstedmaCopyTest2D( EDMACOPY_2D2D, src, dst, cmp, 768, 1024, 1024, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("8 bit, 2D-2D");
	bRes = tstedmaCopyTest2D( EDMACOPY_2D2D, src, dst, cmp, 767, 1021, 1021, FALSE );
	bResTotal = bRes & bResTotal;
	PostReplyMsg("Done : %s", bRes ? "success" : "failure" );
	
	PostReplyMsg("--------------");
	PostReplyMsg("Done: %s", bResTotal ? "success" : "failure" );
	
	MEM_free( SDRAM, src, 0x200000 );
	MEM_free( SDRAM, dst, 0x200000 );
	MEM_free( SDRAM, cmp, 0x200000 );
	
	// Enable watchdog again.
	CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
	
	return TRUE;
}

// *************************************************************************

Int CControl::ParseCommand( const Char * restrict str, Int & unHandlerIndex )
{
	unHandlerIndex = 0;
	
	do
	{
		Int pos = 0;
		Int	len = strlen( g_CommandHandlers[unHandlerIndex].strCommandText );
		
		// Compare characters until the two strings differ
		do
		{
			// Different?
			if ( g_CommandHandlers[unHandlerIndex].strCommandText[pos] != str[pos] )
				break;
				
			pos++;
		} while ( pos < len );
		
		// They match if we've reached the end of both strings
		if ( pos==len )
		{
			// The input command string must be followed by either space (which is followed
			// by the arguments) or '\0', when the command stands for itself.
			if ( str[pos] == ' ')
				return pos + 1;
			else if (str[pos] == '\0')
				return pos;
		}
		
		unHandlerIndex++;
	} while (unHandlerIndex < NUM_COMMANDS );
	
	return 0;
}

// *************************************************************************
Bool CControl::SendBufferedMessages( BufferQueue * queue )
{
	Ptr p;

	// Return if no messages are left.
	if ( bfqGetCount( queue ) == 0)
		return TRUE;
	
	// Repeat until there is no more space in the output queue or
	// all messages have been sent.
	while ( hpiIsOutputQueueFree( m_hHPI ) )
	{
		if ( bfqGetBuffer( queue, &p, 0 ) )
		{
			// Directly send the text
			SendTextMsg( (char*)p );
			
			// and release the buffer
			bfqReleaseBuffer( queue, p );
		}
		else
		{
			return TRUE;
		}
	}	
	
	return FALSE;
}

// *************************************************************************

void CControl::SendViewChannelPictures()
{
	Picture * pic;
	
	// Go through all view channels
	for (Int i=0; i<VIEW_CHANNELS_NUM; i++)
	{
		CViewChannel * chan;	
		
		// Acquire the channel...
		chan = m_pImageProcessing->GetViewChannel( i ) ;
		
		// Abort if error
		if ( chan == NULL )
			return;
		
		// If there is a picture on the queue,...
		if ( chan->GetPicture( pic ) )
		{
			//PostReplyMsg( "Picture found on queue: %d", i );
			
			// Send it to the etrax.
			Int res;
			
			res = hpiSendPicture( m_hHPI, pic, i, 0, 0, 0);
			if ( SYS_OK != res )
				PostReplyMsg( "Picture send failed with code: %d", res );
			
			// And release the buffer.
			chan->ReleasePicture( pic );
		}
	}
}

// *************************************************************************

void CControl::PostReplyMsg( const char * strFormat, ... )
{
	Ptr p;
	char str[REPLY_QUEUE_BUFFERSIZE];
	
	// Acquire a buffer
	if ( ! bfqAllocBuffer( & m_qReplyMsgQueue, &p, 0) )
		return;
	
	// Format message to the string
	va_list ap;
	va_start(ap, strFormat);
	//vsnprintf((char*)p, LOG_QUEUE_BUFFERSIZE, strFormat, ap);	
	vsprintf(str, strFormat, ap);
  	va_end(ap);
  	
  	// Copy the string and put the buffer to the queue
  	edmaCopy( p, str, strlen( str )+1, EDMA_PRI_LOW );
  	bfqPutBuffer( & m_qReplyMsgQueue, p );	
}

// *************************************************************************

void CControl::PostDebugMsg( const char * strFormat, ... )
{
	Ptr p;
	char str[DEBUG_QUEUE_BUFFERSIZE];
	
	// Get the controller instance
	CControl * ctrl = CControl::Instance();
	
	// First, lock the semphore
	if ( ! SEM_pend( & ctrl->m_semDebugMsgQueueLock, 0) )
		return;
			
	// Then, acquire a buffer
	if ( ! bfqAllocBuffer( & ctrl->m_qDebugMsgQueue, &p, 0) )
	{
		SEM_post( & ctrl->m_semDebugMsgQueueLock );
		return;
	}
	
	// Format message to the string
	va_list ap;
	va_start(ap, strFormat);
	//vsnprintf((char*)p, LOG_QUEUE_BUFFERSIZE, strFormat, ap);	
	vsprintf(str, strFormat, ap);
  	va_end(ap);
  	
  	// Copy the string and put the buffer to the queue
  	edmaCopy( p, str, strlen( str )+1, EDMA_PRI_LOW );
  	bfqPutBuffer( & ctrl->m_qDebugMsgQueue, p );	
  	
  	// And release the lock
 	SEM_post( & ctrl->m_semDebugMsgQueueLock );
}

// *************************************************************************

void CControl::SendTextMsgf( const char * strFormat, ... )
{
	char str[512];

	// Format message to the string
	va_list ap;
	va_start(ap, strFormat);
	vsprintf(str, strFormat, ap);	
  	va_end(ap);
  	
  	// Send the string.
  	// TODO: make timeout correct
	hpiSendText( m_hHPI, str, 0,0, 100);
}

// *************************************************************************

void CControl::SendTextMsg( const char * str )
{
	// TODO: make timeout correct
	hpiSendText( m_hHPI, str, 0,0, 100 );
}

// *************************************************************************

void CControl::SendPicture(PictureHandle pic, Int channel, Uint32 timeout )
{
	hpiSendPicture(m_hHPI, pic, channel, 1,0, timeout);
}

// *************************************************************************

void CControl::Assert(bool bCondition, const char * strFile, int nLine )
{
	if ( ! bCondition )
	{
		SendTextMsgf( "ASSERT in file: %s, on line: %d", strFile, nLine );	
		TSK_sleep( 10 * 2000 );
				
		// Stop execution of that task
		while(1);
	}
		
}


// *************************************************************************

// Old tests
/*
	switch( strCommandText[0] )
	{
		
	case '1':
		for ( i=0; i<TESTSIZE; i++)	
			aryTest1[i] = i;
			
		PostReplyMsg("Test 0 (DAT test)");
		xfer = DAT_copy( (Ptr)aryTest1, (Ptr)aryTest2, TESTSIZE*sizeof(Uint32) );
		PostReplyMsg("Starting copy. Copy Id: %d. Waiting....", xfer);
		DAT_wait( xfer );
		PostReplyMsg("Done. Checking...");		
		
		for ( i=0; i<TESTSIZE; i++)	
			if ( aryTest1[i] != aryTest2[i] )
				PostReplyMsg("Error at %d", i);
				
		PostReplyMsg("Done");
		DAT_close();
		break;
		
	case '2':
		PostReplyMsg("DAT Copy large uncached");
		p1 = MEM_alloc( SDRAM, 1024*512*4, 0 );
		p2 = MEM_alloc( SDRAM, 1024*512*4, 0 );
		
		PostReplyMsg("Allocated %p and %p", p1, p2 );
		for ( i=0; i<1024*512; i++ )
			((Uint32*)p1)[i] = i;
			
		xfer = DAT_copy2d( DAT_1D2D, p1, p2, 1024*4, 512, 1024*4 );
		PostReplyMsg("Issued copy, xfer:%d", xfer);
		
		i=0;
		while( DAT_busy( xfer ) )
		{
			PostReplyMsg("Waited %d ticks", i++);
			TSK_sleep(1);
		}
		
		PostReplyMsg("Wait until done.");
		DAT_wait( xfer );
		for ( i=0; i<1024*512; i++ )
			if ( ((Uint32*)p1)[i] != ((Uint32*)p2)[i] )
				PostReplyMsg("Error at %d", i);
				
		MEM_free( SDRAM, p1, 1024*512*4 );
		MEM_free( SDRAM, p2, 1024*512*4 );
		
		DAT_close();	
		break;
		
	case '3':
		PostReplyMsg("DAT Copy large, parallel");
		p1 = MEM_alloc( SDRAM, 1024*1024*4, 0 );
		p2 = MEM_alloc( SDRAM, 1024*1024*4, 0 );
		for ( i=0; i<1024*1024; i++ )
			((Uint32*)p1)[i] = i;
			
		xfer = DAT_copy2d( DAT_1D2D, p1, p2, 1024*4, 512, 1024*4 );
		xfer2 = DAT_copy2d( DAT_1D2D, (Ptr)((Uint8*)p1 + 1024*4*512), (Ptr)((Uint8*)p2 + 1024*4*512), 1024*4, 512, 1024*4);
		PostReplyMsg("Issued copy, xfer:%d, xfer2: %d", xfer, xfer2);
		
		i=0;
		while( DAT_busy( xfer ) || DAT_busy( xfer2 ))
		{
			PostReplyMsg("Waiting  %d ticks, xfer: %d, xfer2: %d", i++, DAT_busy( xfer ), DAT_busy( xfer2 ));
			TSK_sleep(1);
		}
		
		for ( i=0; i<1024*1024; i++ )
			if ( ((Uint32*)p1)[i] != ((Uint32*)p2)[i] )
				PostReplyMsg("Error at %d", i);
				
		PostReplyMsg("Done");
				
		MEM_free( SDRAM, p1, 1024*1024*4 );
		MEM_free( SDRAM, p2, 1024*1024*4 );
		
		DAT_close();	
		break;	
		
	case '4':
		for ( i=0; i<TESTSIZE; i++)	
			aryTest1[i] = i;
			
		PostReplyMsg("Test 4: DAT odd address and size");
		xfer = DAT_copy( (Ptr)((Uint8*)aryTest1 + 1), (Ptr)((Uint8*)aryTest2 + 1), TESTSIZE*sizeof(Uint32) - 4 );
		DAT_wait( xfer );
		PostReplyMsg("Done odd addr");
		
		xfer = DAT_copy( (Ptr)((Uint8*)aryTest1), (Ptr)((Uint8*)aryTest2), TESTSIZE*sizeof(Uint32) - 1 );
		DAT_wait( xfer );
		PostReplyMsg("Done odd size");
		
		xfer = DAT_copy( (Ptr)((Uint8*)aryTest1 + 1), (Ptr)((Uint8*)aryTest2 + 1), TESTSIZE*sizeof(Uint32) - 2 );
		DAT_wait( xfer );
		PostReplyMsg("Done odd size and addr");
				
		DAT_close();
		break;	

		
	case '6':
		PostReplyMsg("PPU read from fifo");
		
		PostReplyMsg("Read: %04x, %04x, %04x, %04x, %04x, %04x, %04x, %04x",
			ppuMemory[0x00], ppuMemory[0x00], ppuMemory[0x00], ppuMemory[0x00], 
			ppuMemory[0x00], ppuMemory[0x00], ppuMemory[0x00], ppuMemory[0x00] );		
		PostReplyMsg("wordcount low: %04x", ppuMemory[0xC2]);
		PostReplyMsg("wordcount high: %04x", ppuMemory[0xC3]);
		break;
		
	case '7':
		PostReplyMsg("PPU Trigger extint6 EDMA");
		EDMA_setChannel( (EDMA_Handle)0x80060090 );
		break;

	case '8':
		PostReplyMsg("PPU set intmask to ffff");		
		ppuMemory[ 0xF2] = 0xFFFF;
		break;

	case '9':
		ppuMemory[0xF2] = 0xFF;
		
		// Do a 0-1 transition on the enable reg.		
		ppuMemory[0xF3] &= ~(0x01);
		ppuMemory[0xF3] |= (0x01);
		break;
		

	case 'd':
		CProfiler::Instance()->StartProfileTask( m_unTestProfileTask );
		TSK_sleep(10);	// wait 1000us
		CProfiler::Instance()->StopProfileTask( m_unTestProfileTask );
		CProfiler::Instance()->StartProfileTask( m_unTestProfileTask );
		TSK_sleep(15);  // wait 1500us
		CProfiler::Instance()->StopProfileTask( m_unTestProfileTask );
		CProfiler::Instance()->StartProfileTask( m_unTestProfileTask );
		TSK_sleep(7);	// wait 700us
		CProfiler::Instance()->StopProfileTask( m_unTestProfileTask );
		
		CProfiler::Instance()->GetTaskInfo( m_unTestProfileTask, str, 512 );
		PostReplyMsg(str);
		break;
	}
	
	return TRUE;
}
*/

