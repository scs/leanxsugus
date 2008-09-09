/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSCONTROL_H_
#define _CLASSCONTROL_H_

class CControl;

#include "FlexiVisioncfg.h"

#include "drvHPI.h"
#include "drvPPUSerial.h"

#include "classImageProcessing.h"
#include "classProfiler.h"


/**
* @brief The main control task class.
*
* The main control task class. It is responsible for the communication to the
* etrax controller and takes its commands, which are then passed to the vision
* task if needed. Only a single instance of this class exists in the system.
*/
class CControl
{
		
public:
							CControl( CImageProcessing * pImageProcessing );
							~CControl();
							
	static CControl *		Instance();
	
	/**
	* Enters the controller's task.
	*/
	void					StartTask();
	
	Bool					cmdMode( const Char * strCommandText );
	Bool					cmdHelp( const Char * strCommandText );
	Bool					cmdChannel( const Char * strCommandText );
	Bool 					cmdProperty( const Char * strCommandText );
	Bool					cmdStats( const Char * strCommandText );
	Bool					cmdCamera( const Char * strCommandText );
	Bool					cmdPing( const Char * strCommandText );
	Bool					cmdStatus( const Char * strCommandText );
	Bool					cmdProfile( const Char * strCommandText );
	Bool					cmdLog( const Char * strCommandText );
	Bool					cmdDebug( const Char * strCommandText );
	Bool					cmdService( const Char * strCommandText );
	Bool					cmdTest( const Char * strCommandText );
	Bool 					cmdTestCopy( const Char * strCommandText );
		
	/**
	* Posts a text message to the debug message queue. This function is static and may be used
	* by anyone. It is also thread safe (locked by a mutex).
	*/
	static void				PostDebugMsg( const char * strFormat, ... );
	
	/**
	* Posts a reply message upon a command from the host. The message is sent
	* as soon as there is place for it in the output queue.
	*/
	void 					PostReplyMsg( const char * strFormat, ... );
	
protected:	
	/**
	* Initializes the camera and sets it up before we can take any pictures.
	*/
	Bool					InitCamera();

	/**
	* Receives all messages that are pending on the HPI from the host and processed them.
	*/
	void 					HandleHostMessages();

	/**
	* Parses the command string and determines the corresponding handler's index number.
	* The return value gives the number of characters parsed so far or 0 if no matching
	* command could be found.
	*/
	Int						ParseCommand( const Char * restrict str, Int & unHandlerIndex );
	
	/**
	* Sends all reply messages waiting in the reply message queue.
	*/
	Bool					SendBufferedMessages( BufferQueue * queue );
	
	/**
	* Sends all debug text messages waiting in the text message queue to the etrax host.
	*/
	void					SendDebugMessages();
	
	void					SendViewChannelPictures();
	
	/**
	* Directly sends a text message to the etrax host, allowing printf-like formatting.
	*/
	void 					SendTextMsgf( const char * strFormat, ... );
	
	/**
	* Directly sends a text to the etrax host.
	*/
	void					SendTextMsg( const char * str );
	
	/**
	* Directly tries to send a picture to the etrax host.
	*/	
	void 					SendPicture( PictureHandle pic, Int channel, Uint32 timeout );

	
	/**
	* Sends a debug message to the etrax host, if bCondition is not true.
	*/
	void 					Assert(bool bCondition, const char * strFile, int nLine );
	
	BufferQueue				m_qReplyMsgQueue;
	
	BufferQueue				m_qDebugMsgQueue;
	SEM_Obj					m_semDebugMsgQueueLock;
	

	/** A link to the vision task object. This reference is used to set parameters from the controller.*/	
	CImageProcessing *		m_pImageProcessing;
	
	/** The handle to the HPI driver. */
	HPI_DevHandle 			m_hHPI;

	/** A debug LED toggler. */
	Bool					m_Led0;
		
	static CControl *		m_pInstance;
	
	/** The device handle to the UART that is talking to the camera over CameraLink. */
	PPUSER_DevHandle		m_hCameraSerial;
	
	/** The wathdog ID of this task object. */
	Uint32					m_unWatchId;
		
	/** The HPI packet that is used to receive messages from the etrax. */	
	HPI_Packet *			m_hpiPacket_in;
	
	/** This HPI packet is used to send large text messages to the etrax. */
	HPI_Packet *			m_hpiPacket_out;
		
	/** A buffer to temporarily store the calibration data that is either being written to
	 *  or read from the DSP board. */
	Uint8 *					m_pCalibData;					
	
	Bool					m_bDebug;	
};


#endif
