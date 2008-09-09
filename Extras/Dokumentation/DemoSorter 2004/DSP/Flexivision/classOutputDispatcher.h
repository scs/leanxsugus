
/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSOUTPUTDISPATCHER_H_
#define _CLASSOUTPUTDISPATCHER_H_


#include "FlexiVisioncfg.h"
#include "drvPPUSerial.h"
#include "classBusCoupler.h"
#include "classModbusBusCoupler.h"
#include "classOutputChannel.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
* Export the function that is periodically called by the DSP/BIOS.
*/
void 		PRD_OutputDispatcher_funct( void );

#ifdef __cplusplus
}
#endif


/**
* @brief 
*/
class COutputDispatcher
{
public:
	/** Constructor. */
								COutputDispatcher();

	/** Returns the single instance of the jet control object. */
	static COutputDispatcher *	Instance();
	
	void 						Init();
	
	/**
	* This function is periodically called and checks whether there is a new command
	* to execute.
	*/
	void						DispatchCommands();
	
	COutputChannel &			Channel( Int nChannel );
	
protected:

	enum OutputDispatcherConsts {
		BK_UART_CHANNEL			= 1,
		MAX_OUTPUTCHANNELS		= 32,
		
		/** The maximum time between two sendings of the output image. Some bus couplers tend
		*   to trigger a watchdog if they don't receive an image once in a while. */
		MAX_TIME_BETWEENSEND_MS	= 500,
		
		/** 
		* The minimum time the dispatcher has to wait between two sendings of the output image.
		* So, this time defines the maximum frequency that may be routed to the bus coupler.
		* Beckhoff: minimum at around 15 ms (@34800 baud)
		* Wago:     minimum at 6 (@115200 baud)
		*/
		MIN_TIME_BETWEENSEND_MS	= 6
	};

	/**
	* Allocate the static and unique output dispatcher object.
	*/
	static COutputDispatcher	g_OutputDispatcher;
	
	/** A handle to the serial driver needed to talk to the bus coupler. */	
	PPUSER_DevHandle			m_hSerial;
	
	/** The bus coupler object. */	
	CModbusBusCoupler			m_BusCoupler;
	
	/** The array of output channel objects. */
	COutputChannel				m_aryChannels[ MAX_OUTPUTCHANNELS ];
	
	/** A flag to indicate whether we're already initialized or not. */
	Bool						m_bInitialized;
	
	Uint32						m_unLastSend;
	Uint32						m_unMaxTimeBetweenSend;
	Uint32						m_unMinTimeBetweenSend;

};


#endif
