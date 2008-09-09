/**
* @file
* Defines the private and hardware dependent parameters of the PPU serial driver, which only the device 
* driver has access to. These are the memory map location, some helper macros and the channel HAL table.
*/

#ifndef _DRVPPUSERIAL_HAL_H_
#define _DRVPPUSERIAL_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <csl_irq.h>

#define				PPU_MEMORY_BASE				(0x60000000)				///< The base memory location of the ppu

static Uint16 		* ppuMemory = (Uint16*)PPU_MEMORY_BASE;					///< A pointer to the memory mapped ppu interface.

/*
#define PPUREG_INTMASK			0xF2
#define PPUREG_CTRL				0xF3
*/

#define REG_IER_ENABLETXRX				0x0003

#define REG_IIR_PENDING_MSK				0x0001
#define REG_IIR_IRQ_MSK					0x000E

#define REG_FCR_FIFORESET_MSK			0x0006
#define REG_FCR_TRIGGERLEVEL_MSK		0x00C0
#define REG_FCR_TRIGGERLEVEL_SHIFT		6
#define REG_FCR_TRIGGERLEVEL_1			0


#define REG_LCR_WORDLEN_MSK				0x0003
#define REG_LCR_WORDLEN_SHIFT			0
#define REG_LCR_WORDLEN_8				3			// wordlen of 8 bits
#define REG_LCR_STOPBITS_MSK			0x0004
#define REG_LCR_PARITYENABLE_MSK		0x0008
#define REG_LCR_EVENPARITY_MSK			0x0010
#define REG_LCR_STICKYPARITY_MSK		0x0020

#define REG_MCR_RTS_MSK					0x0002
#define REG_MCR_LOOPBACK_MSK			0x0010

#define REG_LSR_TXREGEMPTY_MSK			0x0020

#define INT_NONE						0x00
#define	INT_LINESTATUSCHANGE			0x06
#define INT_RXDATAAVAILABLE				0x04
#define INT_CHARTIMEOUT					0x0C
#define INT_TXHOLDINGREGEMPTY			0x02
#define INT_MODEMSTATUSCHANGE			0x00


/**
* The HAL structure that stores the information for a specific UART channel.
*/
typedef struct PPUSER_ChannelHAL
{
	Uint8 			RegData_addr;	
	Uint8			RegInterruptEnable_addr;		
	Uint8			RegInterruptIdentify_addr;	
	Uint8			RegFifoControl_addr;	
	Uint8			RegLineControl_addr;	
	Uint8			RegModemControl_addr;	
	Uint8			RegLineStatus_addr;
	Uint8			RegModemStatus_addr;
	Uint8			RegDivisorLSB_addr;
	Uint8			RegDivisorMSB_addr;

} PPUSER_ChannelHAL;

/**
* The HAL structure that stores the general UART specific hardware information. 
*/
typedef struct PPUSER_HAL
{
	Uint32					unInterruptSource;
	Uint16					RegInterruptEnable_addr;
	Uint16					RegInterruptEnable_mask;
	
	/** The FPGA's clock frequency, used to calculate the UART divisor. */
	Uint32					unClockFrequency;
	
	PPUSER_ChannelHAL		aryChannelHal[PPUSER_NUMCHANNELS];
} PPUSER_HAL;

const PPUSER_HAL serUartHal=
{
	IRQ_EVT_EXTINT4,
	0xF2,
	0x0001,
	
	/** The fpga is running on the 100MHZ EMIFB clock. */
	100*1000*1000,
	{
		{	0x80, 0x81, 0x82, 0x82, 0x83, 0x84, 0x85, 0x86, 0x88, 0x89 },
		{	0x90, 0x91, 0x92, 0x92, 0x93, 0x94, 0x95, 0x96, 0x98, 0x99 },
		{	0xA0, 0xa1, 0xa2, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa8, 0xa9 }
	}
};

									

#ifdef __cplusplus
}
#endif

#endif

