
#include "classBeckhoffBusCoupler.h"
#include "libDebug.h"

#include <string.h>


CBeckhoffBusCoupler::CBeckhoffBusCoupler( )
{
	m_bChecksumReady = FALSE;
}

// *************************************************************************

void CBeckhoffBusCoupler::SetDigitalOutput( Int nOutput, Bool bValue )
{
	// Invalidate checksum flag so that it will be re-computed before sending
	// the image.
	m_bChecksumReady = FALSE;
	
	Int nBit = nOutput & 0x07;
	Int nByte = nOutput >> 3;
	
	if ( bValue )
		m_aryRequest[ REQUESTHEADER_SIZE+nByte ] |= (Uint8)( 1 << nBit );
	else
		m_aryRequest[ REQUESTHEADER_SIZE+nByte ] &= (Uint8)(~( 1 << nBit ));
}
	
// *************************************************************************

Bool CBeckhoffBusCoupler::GetDigitalInput( Int nInput )
{
	Int nBit = nInput & 0x07;
	Int nByte = nInput >> 3;
	
	if ( (m_aryAnswer[ ANSWERHEADER_SIZE+nByte ] & (Uint8)( 1 << nBit )) != 0 )
		return TRUE;
	else
		return FALSE;
}


// *************************************************************************

void CBeckhoffBusCoupler::SendImage()
{
	// Calculate the checksum 
	Uint8 sum = 0;
	
	if ( ! m_bChecksumReady )
	{
		// Sum up all data words.
		for ( Int i=0; i<(m_nRequestImgSize + REQUESTHEADER_SIZE); i++)		
			sum += m_aryRequest[ i ];
			
		// And set the checksum
		m_aryRequest[ REQUESTHEADER_SIZE + m_nRequestImgSize ] = sum;
		
		// Set the flag so that the sum doesn't have to be re-calculated each time the
		// packet is sent.
		m_bChecksumReady = TRUE;	
	}
	// DEBUG::
	/*
	static int delme=0;
	static int set = 1;
	delme++;
	if ( delme == 5 )
	{
		Uint8 aryRequest[10];
		
		delme = 0;
		aryRequest[0] = 'P';
		aryRequest[1] = 2;
		aryRequest[2] = 1;
		aryRequest[3] = 1;
		aryRequest[4] = 255;
		aryRequest[5] = 255;
		aryRequest[6] = 255;
		aryRequest[7] = 255;
		aryRequest[8] = 'P' + 2 + 1 + 1 + 4*255 ;//set;
		serWrite( m_hSerial, (char*)aryRequest, 9 );
		
		aryRequest[7] = 0;
		aryRequest[8] = 'P' + 2 + 1 + 1 + 3*255 ;//set;
		serWrite( m_hSerial, (char*)aryRequest, 9 );
		
		aryRequest[7] = 255;
		aryRequest[8] = 'P' + 2 + 1 + 1 + 4*255 ;//set;
		serWrite( m_hSerial, (char*)aryRequest, 9 );
		
		aryRequest[7] = 0;
		aryRequest[8] = 'P' + 2 + 1 + 1 + 3*255 ;//set;
		serWrite( m_hSerial, (char*)aryRequest, 9 );
		
//		set = (set==0) ? 1 : 0;
		// m_aryRequest[4] = 0x01;
		// m_aryRequest[8] = 'P' + 2 + 1 + 1;
		// serWrite( m_hSerial, (char*)m_aryRequest, REQUESTHEADER_SIZE + m_nRequestImgSize + CHECKSUM_SIZE);
		
		
		
	}
	return;
	// /DEBUG	
	*/

	// Write the request to the BK
	Int nWritten;
	nWritten = serWrite( m_hSerial, (char*)m_aryRequest, REQUESTHEADER_SIZE + m_nRequestImgSize + CHECKSUM_SIZE);
	if ( nWritten != REQUESTHEADER_SIZE + m_nRequestImgSize + CHECKSUM_SIZE )
		dbgLog( "BusCoupler: send queue full!" );

}

// *************************************************************************

void CBeckhoffBusCoupler::ReceiveImage()
{
	Int count;
	Int read;
	Uint8 answ[ANSWERHEADER_SIZE + MAXIMAGE_SIZE + CHECKSUM_SIZE];
	
	count = ANSWERHEADER_SIZE + m_nAnswerImgSize + CHECKSUM_SIZE;
	
	
	// Try to read
	read = serRead( m_hSerial, (char*)answ, count );
	
	
	// Debug:
	if ( read>0 )
	{
		answ[read]=0;
		dbgLog( "something received: %d bytes: '%s'", read, answ );
	}
	
	// See if we've got enough chars
	if ( (read != count) && (read != 0) )
	{
		serFlushRx( m_hSerial );
		return;
	}	
	
	// See if the identifiers are right
	if ( (answ[0] != 'p') || ( answ[3] != m_nBCAddr) )
	{
		serFlushRx( m_hSerial );
		return;
	}
	
	// The packet seems valid, copy it to our answer image.
	memcpy( m_aryAnswer, answ, count );
	
}

// *************************************************************************

void CBeckhoffBusCoupler::PrepareImages()
{
	
	// Calculate image sizes
	m_nRequestImgSize = ( ((m_nNumOutputs+7)/8) +  1) & ~0x01;
		
	m_nAnswerImgSize = ( ((m_nNumInputs+7) / 8) + 1) & ~0x01;
		
	// Only need to pre-fill the request image.
	m_aryRequest[0] = 'P';						// type
	m_aryRequest[1] = m_nRequestImgSize/2;		// size
	m_aryRequest[2] = m_nBCAddr;				// ident
	m_aryRequest[3] = m_nBCAddr;				// addr
	
	// Set all outputs to 0.
	for ( Int i=0; i<m_nRequestImgSize; i++)
		m_aryRequest[ REQUESTHEADER_SIZE + i ] = 0;
	
}

// *************************************************************************
