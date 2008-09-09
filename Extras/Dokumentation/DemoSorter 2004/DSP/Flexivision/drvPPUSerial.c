

#include "drvPPUSerial.h"
#include "drvPPUSerial_HAL.h"
#include "classWrapper.h"
#include "libDebug.h"


/** Allocate a certain number of ringbuffer handles for the serial driver. */
//RingBuffer_Handle 	ppuSerialRingbuffer[ PPU_NUMSERIAL ];

const Uint8			serSerialMsg_Open = 0x01;			///< A message to tell the serial driver task to start operation.
const Uint8			serSerialMsg_Close = 0x02;			///< A message to tell the serial driver task to cease operation.

extern Int			SDRAM;								///< The segment ID of the SDRAM (defined by the config utility).

/**
* Flag to indicate whether the driver has already been opened or not.
*/
Bool				serSerialIsOpened = FALSE;

/**
* THE ppu serial device driver object.
*/
static PPUSER_Object		serObject = {
								{ FALSE, FALSE, FALSE }
								};

/**
* This is the serial port driver's main task function.
*/
void serSerial_tsk_funct();



// *************************************************************************

PPUSER_DevHandle		serOpen( Int nChannel, Int bufsize )
{
	PPUSER_DevHandle			dev;	
	const PPUSER_ChannelHAL * 	hal;
	
	if ( (nChannel < 0) || (nChannel >= PPUSER_NUMCHANNELS))
		return NULL;
	
	if ( serObject.aryChannelOpened[nChannel] )
		return NULL;
		
	// Get the right channel and hal
	dev = &(serObject.aryChannels[nChannel]);
	hal = &(serUartHal.aryChannelHal[nChannel]);
			
	// Setup serial ringbuffers and semaphores.
	// Also pre-configure the UART register.
	rbfCreate_d( &(dev->SerialRx), 0, bufsize );
	rbfCreate_d( &(dev->SerialTx), 0, bufsize );
	
	// Enable Rx and Tx interrupts
	ppuMemory[hal->RegInterruptEnable_addr] = REG_IER_ENABLETXRX;
	
	// Configure the fifo control register
	ppuMemory[hal->RegFifoControl_addr] |= REG_FCR_FIFORESET_MSK;		// reset fifos.
	ppuMemory[hal->RegFifoControl_addr] = (ppuMemory[hal->RegFifoControl_addr] & ~REG_FCR_TRIGGERLEVEL_MSK ) | (REG_FCR_TRIGGERLEVEL_1 << REG_FCR_TRIGGERLEVEL_SHIFT); // set the trigger level
	
	// Configure the line control register.
	ppuMemory[hal->RegLineControl_addr] = (ppuMemory[hal->RegLineControl_addr] & ~REG_LCR_WORDLEN_MSK ) | (REG_LCR_WORDLEN_8 << REG_LCR_WORDLEN_SHIFT);
	ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_STOPBITS_MSK;
	ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_PARITYENABLE_MSK;
	ppuMemory[hal->RegLineControl_addr] |= REG_LCR_EVENPARITY_MSK;
	ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_STICKYPARITY_MSK;
	
	// Configure modem control register
	ppuMemory[hal->RegModemControl_addr] &= ~REG_MCR_RTS_MSK;
	ppuMemory[hal->RegModemControl_addr] &= ~REG_MCR_LOOPBACK_MSK;	
	
	// Set the (PPU-)global interrupt enable register.
	ppuMemory[ serUartHal.RegInterruptEnable_addr ] |= serUartHal.RegInterruptEnable_mask;
	
	dev->unChannelNum = nChannel;
		
	// Clear and enable the interrupt
	IRQ_clear(serUartHal.unInterruptSource);
	IRQ_enable(serUartHal.unInterruptSource);
	
	serObject.aryChannelOpened[nChannel] = TRUE;
	
	return dev;
}


// *************************************************************************

void	serClose( PPUSER_DevHandle device )
{
	Int 	i;
	Bool 	bAllClosed = TRUE;
	
	serObject.aryChannelOpened[ device->unChannelNum ] = FALSE;
	
	// Disable interrupt if all UART channels are closed now
	for ( i=0; i<PPUSER_NUMCHANNELS; i++)
		if ( serObject.aryChannelOpened[ device->unChannelNum ] )
			bAllClosed = FALSE;
			
	if ( bAllClosed )
		IRQ_disable(serUartHal.unInterruptSource);
	
	// Close the serial channel
	rbfDelete( &(device->SerialTx));
	rbfDelete( &(device->SerialRx));
}

// *************************************************************************

Int		serWrite(PPUSER_DevHandle device, const char * buffer, Int count)
{
	Uint32 write;
	
	SWI_disable();
	
	write = rbfWrite( &device->SerialTx, buffer, count );
	
	// Trigger the task so that the new characters are sent
	//SEM_post( &serSerial_sem );
	SWI_post( &serPPUSerial_swi );
	
	SWI_enable();
	
	return write;
}

// *************************************************************************

Int		serRead(PPUSER_DevHandle device, char * buffer, Int count )
{
	Uint32 read;
	
	SWI_disable();
	
	read = rbfRead( &device->SerialRx, buffer, count);
	
	SWI_enable();
	
	return read;
}


// *************************************************************************

void	serFlushRx( PPUSER_DevHandle device )
{
	SWI_disable();
	
	rbfFlush( &(device->SerialRx) );
	
	SWI_enable();
}

// *************************************************************************

void	serFlushTx( PPUSER_DevHandle device )
{
	SWI_disable();
	
	rbfFlush( &(device->SerialTx) );
	
	SWI_enable();
}

// *************************************************************************

int	serLevelTx( PPUSER_DevHandle device )
{
	int n;
	
	SWI_disable();
	
	n = rbfNumFull( &(device->SerialTx) );
	
	SWI_enable();
	
	return n;
}

// *************************************************************************

int	serLevelRx( PPUSER_DevHandle device )
{
	int n;
	
	SWI_disable();
	
	n = rbfNumFull( &(device->SerialRx) );
	
	SWI_enable();
	
	return n;
}

// *************************************************************************

Bool	serConfigChannel(PPUSER_DevHandle device, const Int speed, const Bool bTwoStopBits, const Bool bParity, const Bool bEvenParity )
{
	Uint16 divisor;
	
	const PPUSER_ChannelHAL * 	hal;
	
	hal = &(serUartHal.aryChannelHal[device->unChannelNum]);
	
	// Calculate the divisor. The divisor divides the fpga clock and must be
	// 16 times faster than the actual UART rate. Also, one must be subtracted,
	// since the divisor defines the number of clock cycles to wait between two
	// samples.
	//
	// 9600 => 0x028a
	divisor = serUartHal.unClockFrequency / 16 / speed - 1;
		
	// Write the two registers.
	ppuMemory[hal->RegDivisorLSB_addr] = (divisor & 0x00FF);
	ppuMemory[hal->RegDivisorMSB_addr] = (divisor & 0xFF00) >> 8 ;
	
	// Configure the stop bits. A value of 0 on this bit lead to the use of 1 stopbit, a value of 1
	// will provoce the use of two stop bits.
	if (bTwoStopBits)
		ppuMemory[hal->RegLineControl_addr] |= REG_LCR_STOPBITS_MSK;
	else
		ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_STOPBITS_MSK;
	
	// Configure the parity stuff.
	if ( bParity )
		ppuMemory[hal->RegLineControl_addr] |= REG_LCR_PARITYENABLE_MSK;
	else
		ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_PARITYENABLE_MSK;
		
	if ( bEvenParity )
		ppuMemory[hal->RegLineControl_addr] |= REG_LCR_EVENPARITY_MSK;
	else
		ppuMemory[hal->RegLineControl_addr] &= ~REG_LCR_EVENPARITY_MSK;
	
		
	return TRUE;
}

// ********************************************************************

void serLoopback( PPUSER_DevHandle device, Bool bLoop )
{
	const PPUSER_ChannelHAL * 	hal;
	
	hal = &(serUartHal.aryChannelHal[device->unChannelNum]);
	
	if ( bLoop )
		ppuMemory[hal->RegModemControl_addr] |= REG_MCR_LOOPBACK_MSK;
	else
		ppuMemory[hal->RegModemControl_addr] &= ~REG_MCR_LOOPBACK_MSK;
}

// *************************************************************************

void serPPUSerial_swi_funct()
{	
	Uint32 	i;
	Uint16 	unIIR;
	volatile Uint16	unTmp;
	Bool 	bNextTxChar;
	char 	cNextTxChar;
	
	PPUSER_DevHandle			dev;	
	const PPUSER_ChannelHAL * 	hal;
			
	// Go through all channels.
	for ( i=0; i<PPUSER_NUMCHANNELS ; i++ )
	{
		// Get the right channel and hal
		dev = &(serObject.aryChannels[i]);
		hal = &(serUartHal.aryChannelHal[i]);
		
		// Only go on if that channel is in use
		if ( serObject.aryChannelOpened[ i ] )
		{	
			// Get the next char from the tx fifo
			bNextTxChar = rbfPeekChar( &(dev->SerialTx), &cNextTxChar );
			
			// Read the interrupt ID
			unIIR = ppuMemory[ hal->RegInterruptIdentify_addr ];
			
			/*
			if ( i==0 )
			{
				if ( numChars > 400 )
					dbgLog("numChars: %d, IIR: 0x%04x, LSR: 0x%04x", numChars, unIIR, ppuMemory[ hal->RegLineStatus_addr] );
			}
			*/
			
			// Is this really an interrupt from this UART? Bit 0 of IIR has to be 0 if that's the case.
			if ( (unIIR & REG_IIR_PENDING_MSK) == 0 )
			{				
				// Now handle the specific interrupt, which is coded in bits 1 through 3.
				switch ( unIIR & REG_IIR_IRQ_MSK )
				{
					// Modem status change
					case INT_MODEMSTATUSCHANGE :
						unTmp = ppuMemory[ hal->RegModemStatus_addr ];
						break;
					
					// Tx holding register empty.
					case INT_TXHOLDINGREGEMPTY :
						// Write the char
						if ( bNextTxChar )
						{
							ppuMemory[ hal->RegData_addr ] = (Uint16)cNextTxChar;
							
							// And remove it from the fifo
							rbfReadChar( &(dev->SerialTx), &cNextTxChar );
						}
						break;
						
					// Rx data available or RX timeout.
					case INT_RXDATAAVAILABLE :
					case INT_CHARTIMEOUT :
						// Put the character to the fifo.
						rbfWriteChar( &(dev->SerialRx), (char)(ppuMemory[ hal->RegData_addr ]) );
						break;
						
					// Line status change
					case INT_LINESTATUSCHANGE :
						unTmp = ppuMemory[ hal->RegLineStatus_addr ];
						break;
				}
			}
			else
			// If there was no interrupt on this UART but we've still got chars to send,
			// see if the tx register is empty and write the char if that's the case.
			{
				if ( (bNextTxChar ) && ( (ppuMemory[ hal->RegLineStatus_addr] & REG_LSR_TXREGEMPTY_MSK) != 0 ) )
				{
					ppuMemory[ hal->RegData_addr ] = (Uint16)cNextTxChar;
					
					// And remove it from the fifo
					rbfReadChar( &(dev->SerialTx), &cNextTxChar );	
				}
			}			
			
		} // if channel is open
			
	} // for all channels
}

// *************************************************************************

void serPPUSerial_hwi_funct()
{
	// Trigger the serial task to execute.
	//SEM_post( &serSerial_sem );
	
	// DEBUG: directly call SWI
	SWI_post( &serPPUSerial_swi );
	//serPPUSerial_swi_funct();
	
	IRQ_clear( serUartHal.unInterruptSource );
}

// *************************************************************************




