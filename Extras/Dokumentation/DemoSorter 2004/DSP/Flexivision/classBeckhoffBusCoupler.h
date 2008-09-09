/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSBECKHOFFBUSCOUPLER_H_
#define _CLASSBECKHOFFBUSCOUPLER_H_

#include "classBusCoupler.h"

#include "drvPPUSerial.h"

/**
* @brief The bus coupler establishes the interface to the external BK8000 via RS-485.
*/
class CBeckhoffBusCoupler : public CBusCoupler
{
public:
						CBeckhoffBusCoupler( );

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
	
	enum BusCouplerConsts {
		REQUESTHEADER_SIZE	= 4,
		ANSWERHEADER_SIZE	= 5,
		CHECKSUM_SIZE		= 1
	};
	
	/**
	* Overwrite the PrepareImages() function. 
	*/
	void				PrepareImages();
	
	PPUSER_DevHandle	m_hSerial;
	
	Int					m_nBCAddr;
	Int					m_nNumInputs;
	Int					m_nNumOutputs;
	
	Bool				m_bChecksumReady;
	
	Uint8				m_aryRequest[REQUESTHEADER_SIZE + MAXIMAGE_SIZE + CHECKSUM_SIZE];
	/** The request image size. This is the size of the above array. This value is rounded up (to a multiple of
	 *  2). */
	Int					m_nRequestImgSize;

	
	Uint8				m_aryAnswer[ANSWERHEADER_SIZE + MAXIMAGE_SIZE + CHECKSUM_SIZE];
	Int					m_nAnswerImgSize;
};

#endif
