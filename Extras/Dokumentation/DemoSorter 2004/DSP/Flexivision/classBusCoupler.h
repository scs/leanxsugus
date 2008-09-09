/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSBUSCOUPLER_H_
#define _CLASSBUSCOUPLER_H_


#include "drvPPUSerial.h"

/**
* @brief The bus coupler establishes the interface to the external BK8000 via RS-485.
*/
class CBusCoupler
{
public:
						CBusCoupler( );

	/**
	* The BusCoupler object need a serial UART channel to operate on (note: multiple objects may operate
	* on the same UART channel, only diversed by the bus coupler address), a bus coupler address and
	* the number of digital input and output channels of the bus coupler.
	*/						
	virtual void		Config( PPUSER_DevHandle hSerial, Int nBCAddr, Int nNumInputs, Int nNumOutputs );
	
	/**
	* Sets a digital output to a certain value. The image is not yet transmitted to the bus coupler, for
	* this is done by the SendImage() function.
	*/
	virtual void		SetDigitalOutput( Int nOutput, Bool bValue ) = 0;
	
	/**
	* Gets the last read state of a certain input channel of the bus coupler.
	*/
	virtual Bool		GetDigitalInput( Int nInput ) = 0;
	
	/**
	* Sends the current image through the UART channel. This will provoke the Bus coupler
	* to send back the input channel states, which can be received by ReceiveImage().
	*/
	virtual void		SendImage() = 0;
	
	/**
	* This receives the input channel states from the bus coupler. The function assumes that
	* all the necessary characters have already arrived through the UART and are ready on the
	* input buffer.	
	*/
	virtual void		ReceiveImage() = 0;
							
				
protected:
	
	enum BusCouplerConsts {
		/** The maximum sie of the input and output process images in bytes. */
		MAXIMAGE_SIZE		= 16		
	};
	
	/** Prepares the images after the bus coupler has been configured. */
	virtual void		PrepareImages() = 0;
	
	/** The serial handle that is used to communicate with the buscoupler. */
	PPUSER_DevHandle	m_hSerial;
	
	/** The bus coupler's address. */
	Int					m_nBCAddr;
	
	/** The number of digital inputs. */
	Int					m_nNumInputs;
	
	/** The number of digital outputs. */
	Int					m_nNumOutputs;
	
};

#endif
