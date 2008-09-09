
#include "classModbusBusCoupler.h"
#include "libDebug.h"

#include <string.h>


CModbusBusCoupler::CModbusBusCoupler( )
{
}

// *************************************************************************

void CModbusBusCoupler::SetDigitalOutput( Int nOutput, Bool bValue )
{
	Int nBit = nOutput & 0x07;
	Int nByte = nOutput >> 3;
	
	if ( nByte >= MAXIMAGE_SIZE )
		return;
	
	if ( bValue )
		m_arySend[ SENDHEADER_SIZE+nByte ] |= (Uint8)( 1 << nBit );
	else
		m_arySend[ SENDHEADER_SIZE+nByte ] &= (Uint8)(~( 1 << nBit ));
}
	
// *************************************************************************

Bool CModbusBusCoupler::GetDigitalInput( Int nInput )
{
	Int nBit = nInput & 0x07;
	Int nByte = nInput >> 3;
	
	if ( nByte >= MAXIMAGE_SIZE )
		return FALSE;
	
	if ( (m_aryReceive[ RECEIVEHEADER_SIZE+nByte ] & (Uint8)( 1 << nBit )) != 0 )
		return TRUE;
	else
		return FALSE;
}


// *************************************************************************

void CModbusBusCoupler::SendImage()
{
	// DEBUG:
	static bool b=false;
	SetDigitalOutput( 31, b );
	b = !b;
	// /DEBUG
	
	m_arySend[0] = m_nBCAddr;
	m_arySend[1] = CMD_PRESET_MULTIPLE_REGISTERS;		
	m_arySend[2] = PROCESS_IMG_ADDR >> 8;				// start address hi
	m_arySend[3] = PROCESS_IMG_ADDR & 0x00FF;			// start address lo 
	m_arySend[4] = m_nSendImgSize >> 9;				// number of registers hi
	m_arySend[5] = (m_nSendImgSize >> 1) & 0x00FF;		// number of registers lo
	m_arySend[6] = m_nSendImgSize;
	
	// The outputs are already set accordingly...
	
	// Calculate the checksum. Note: the CRC function is copied from a modbus homepage
	// and already swaps the high and the low bytes, which neither helps nor harms us, but must
	// be taken into account.
	Uint16 crc = CalcCRC16( m_arySend, SENDHEADER_SIZE + m_nSendImgSize );
	m_arySend[SENDHEADER_SIZE + m_nSendImgSize ] = ( crc >> 8 );
	m_arySend[SENDHEADER_SIZE + m_nSendImgSize + 1 ] = ( crc & 0xFF );
	
	// Write the request to the BK
	Int nWritten;
	nWritten = serWrite( m_hSerial, (char*)m_arySend, SENDHEADER_SIZE + m_nSendImgSize + CRC_SIZE);
	if ( nWritten != SENDHEADER_SIZE + m_nSendImgSize + CRC_SIZE )
		dbgLog( "BusCoupler: send queue full!" );

/*
	dbgLog("modbus write: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
			m_arySend[0],
			m_arySend[1],
			m_arySend[2],
			m_arySend[3],
			m_arySend[4],
			m_arySend[5],
			m_arySend[6],
			m_arySend[7],
			m_arySend[8],
			m_arySend[9],
			m_arySend[10],
			m_arySend[11],
			m_arySend[12] );
			*/
			
			
}

// *************************************************************************

void CModbusBusCoupler::ReceiveImage()
{
	
}

// *************************************************************************

void CModbusBusCoupler::PrepareImages()
{
	
	// Calculate image sizes
	m_nSendImgSize = ( ((m_nNumOutputs+7)/8) +  1) & ~0x01;
		
	m_nReceiveImgSize = ( ((m_nNumInputs+7) / 8) + 1) & ~0x01;
		
	// Only need to pre-fill the request image.
		
	// Set all outputs to 0.
	for ( Int i=0; i<m_nSendImgSize; i++)
		m_arySend[ SENDHEADER_SIZE + i ] = 0;
	
}

// *************************************************************************

/* Table of CRC values for high-order byte */
static unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ; 

/* Table of CRC values for low-order byte */
static char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

unsigned short CModbusBusCoupler::CalcCRC16(unsigned char * puchMsg, unsigned short usDataLen)
{
	/* high CRC byte initialized */
	unsigned char uchCRCHi = 0xFF ;
	
	/* low CRC byte initialized  */
	unsigned char uchCRCLo = 0xFF ;
	
	/* will index into CRC lookup*/
	unsigned uIndex ;
	
	/* table */
	while (usDataLen--)/* pass through message buffer*/
	{
		/* calculate the CRC*/
		uIndex = uchCRCHi ^ *puchMsg++ ;
		
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	
	return (uchCRCHi << 8 | uchCRCLo) ;
}

