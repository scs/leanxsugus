/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSJETCONTROL_H_
#define _CLASSJETCONTROL_H_


#include "FlexiVisioncfg.h"

#include "drvPPUSerial.h"

#include "classBusCoupler.h"

#ifdef __cplusplus
extern "C"
{
#endif

void 		PRD_JetControl_funct( void );

#ifdef __cplusplus
}
#endif

/**
* This is the command structure that is used to pass commands to the jet control
* object.
*/
typedef struct _JetCommand
{
	enum JetCommandConsts{
		MAX_JETS		= 10		
	};
	
	Uint32		unCmdTime;
		
	Uint8		aryJetState[ MAX_JETS ];	
} JetCommand;


/**
* @brief The jet controller task object.
*
* The jet control task is represented by this singleton object.
* The jet control is under real-time requirements and thus runs with the highest priority
* of all the tasks in the system. It is responsible for creating the ejection commands at
* the apropriate time and send them to the BK8000 via the RS-485 interface. It provides an
* interface to add commands to a list, which are then processed one at a time.
*/
class CJetControl
{
public:
	/** Constructor. */
							CJetControl();

	/** Returns the single instance of the jet control object. */
	static CJetControl *	Instance();
	
	void 					Init();
	
	/**
	* This function is periodically called and checks whether there is a new command
	* to execute.
	*/
	void					CheckCommands();
	
	/**
	* Adds a new jet command to the command list for execution. 
	*/
	void					AddCommand( JetCommand * cmd );
	
protected:

	enum JetControlConsts {
		MAX_COMMANDS 		= 64,
		BK_UART_CHANNEL		= 1
	};

	static CJetControl		g_JetControl;
	
	/** The jet command list, which is actually a ringbuffer of jet command objects. */
	JetCommand				m_aryJetCommands[ MAX_COMMANDS ];
	
	Int						m_nNextFreeCommand;
	Int						m_nNextCommand;
	
	Uint32					m_unLastCmdTime;
	
	PPUSER_DevHandle		m_hSerial;
	
	CBusCoupler				m_BusCoupler;
	
	Bool					m_bInitialized;

};



#endif
