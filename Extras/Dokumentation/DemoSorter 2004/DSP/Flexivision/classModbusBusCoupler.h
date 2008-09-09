/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSMODBUSBUSCOUPLER_H_
#define _CLASSMODBUSBUSCOUPLER_H_


#include "drvPPUSerial.h"
#include "classBusCoupler.h"

/**
* @brief The modbus bus coupler establishes the interface to the external 750-315 Wago buscoupler via RS-485.
*
* The class implements the generic buscoupler interface to access a Wago 750-315 modbus buscoupler
* via the RS-485 interface.
*/
class CModbusBusCoupler : public CBusCoupler
{
public:
						CModbusBusCoupler( );

	/**
	* Overwrite the SetDigitalOutput() function.
	*/
	virtual void		SetDigitalOutput( Int nOutput, Bool bValue );
	
	/**
	* Overwrite the GetDigitalInput() function.
	*/
	virtual Bool		GetDigitalInput( Int nInput );
	
	/**
	* Overwrite the SendImage() function.
	*/
	virtual void		SendImage();
	
	/**
	* Overwrite the ReceiveImage() function.
	*/
	virtual void		ReceiveImage();
							
				
protected:
	
	enum ModbusBusCouplerConsts {
		SENDHEADER_SIZE	= 7,
		RECEIVEHEADER_SIZE	= 9,
		CRC_SIZE = 2,
		
		CMD_PRESET_MULTIPLE_REGISTERS = 0x10,
		PROCESS_IMG_ADDR	= 0x0000
	};
	
	/**
	* Overwrite the PrepareImages() function. 
	*/
	virtual void		PrepareImages();
	
	unsigned short 		CalcCRC16(unsigned char * puchMsg, unsigned short usDataLen);
	
	Uint8				m_arySend[SENDHEADER_SIZE + MAXIMAGE_SIZE + CRC_SIZE];
	Int					m_nSendImgSize;
		
	Uint8				m_aryReceive[RECEIVEHEADER_SIZE + MAXIMAGE_SIZE + CRC_SIZE];
	Int					m_nReceiveImgSize;
};

#endif
