/**
* @file
* Defines the private and hardware dependent parameters of the PPU, which only the device driver has
* access to. These are the memory map location, some helper macros and the channel HAL table.
*/

#ifndef _DRVPPU_HAL_H_
#define _DRVPPU_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <csl_irq.h>
#include <csl_edma.h>

#define				PPU_MEMORY_BASE				(0x60000000)				///< The base memory location of the ppu

#define				PPU_REG2ABS(x)				(Ptr)(PPU_MEMORY_BASE + 2*x)///< Converts a register number to an absolute memory address

static volatile Uint16 		* ppuMemory = (Uint16*)PPU_MEMORY_BASE;					///< A pointer to the memory mapped ppu interface.

/*
#define PPUREG_GAINRED_ADDR		0xD0
#define PPUREG_GAINGREEN_ADDR	0xD1
#define PPUREG_GAINBLUE_ADDR	0xD2

#define PPUREG_INTMASK			0xF2
#define PPUREG_CTRL				0xF3

#define REG_CTRL_TRIGGERIMG_MSK		0x0001
#define REG_CTRL_READYLINE_MSK		0x0008
*/

#define PPUREG_GAINRED_ADDR		0xE0
#define PPUREG_GAINGREEN_ADDR	0xE1
#define PPUREG_GAINBLUE_ADDR	0xE2

#define PPUREG_INTMASK			0xF2
#define PPUREG_CTRL				0xF3

#define REG_CTRL_TRIGGERIMG_MSK		0x0001
#define REG_CTRL_READYLINE_MSK		0x0008


/**
* A table which collects all the HAL information for a channel.
*/
typedef struct PPU_PictureChannelHAL
{	
	Int				EdmaTrigger;			///< Defines which external EDMA event corresponds to which channel. */
	SWI_Handle		Ready_swi;				///< The SWI handler that are used when an EDMA transfer completes. These SWI objects are created by the config tool.
	
	Ptr				pFifo;					///< The fifo's address on the ppu. Must be an absolute address in the DSP's main memory space.
	
	Uint8			RegIntEnable_addr;		///< The interrupt enable register's address.
	Uint16			RegIntEnable_mask;		///< The bitmask for this channel in the interrupt enable register.
		
	Uint8			RegChannelEnable_addr;	///< The channel enable register for this channel.
	Uint16			RegChannelEnable_mask;	///< The bitmask for this channel in the channel enable register.
		
	Uint8			RegChannelFlush_addr;	///< The flush regiser's address.
	Uint16			RegChannelFlush_mask;	///< The bitmask for this channel in the flush register.
	
	Uint8			RegSourceSelect_addr;
	Uint16			RegSourceSelect_mask;
	
	Uint8			RegFramesize_addr;		///< The framesize register's address.
	Uint8			RegWordcountLo_addr;	///< The wordcount_low register's address.
	Uint8			RegWordcountHi_addr;	///< The wordcount_high register's address.
	Uint8			RegPiccount_addr;		///< The piccount register's address.
} PPU_PictureChannelHAL;

/**
* The collection of HAL information regarding the PPU.
*/
typedef struct PPU_HAL
{
	// SRAM interface:
	Uint8						RegSramWriteAddrLo_addr;	
	Uint8						RegSramWriteAddrHi_addr;
	Uint8						RegSramReadAddrLo_addr;
	Uint8						RegSramReadAddrHi_addr;
	
	Uint8						RegSramFifo_addr;
	
	Uint8						RegSramFifoStatus_addr;
	Uint16						RegSramFifoStatusWriteDone_mask;
	Uint16						RegSramFifoStatusReadReady_mask;
	Uint16						unSramFifoSize;
	
	// Picture acquisition
	Uint8						RegRTPFStatus_addr;
	
	Uint8						RegControl_addr;

	// Picture Fifos interface:
	PPU_PictureChannelHAL		PictureChannel[PPU_MAXCHANNELS];
} PPU_HAL;

#undef _USE_OLD_FW
#ifndef _USE_OLD_FW

const PPU_HAL				ppuHal =
							{
								0xD0,					// WriteAddr Lo
								0xD1,					// WriteAddr Hi
								0xD2,					// ReadAddr Lo
								0xD3,					// ReadAddr Hi
								
								0x40,					// Fifo addr
								
								0xD4,					// Fifo status
								0x0001,					// write done mask
								0x0002,					// read ready mask
								32,						// fifo size
								
								0xCF,					// RTPF status
								0xF3, 					// Control register
								
								{
									{	EDMA_CHA_EXTINT5, &ppuPictureReady0_swi,	
										PPU_REG2ABS(0x00), 
										0xF2, 0x0002,   	// Int enable
										0xFF, 0x0000,  		// chan enable (not needed)
										0xF3, 0x0002,   	// Flush
										0xF3, 0x0004,		// Source select
										0xC0, 				// Framesize
										0xC1, 0xC2, 		// Wordcount lo, hi
										0xC3 				// Pic count
									},
										
									{	EDMA_CHA_EXTINT6, &ppuPictureReady0_swi,	
										PPU_REG2ABS(0x10), 
										0xF2, 0x0003,   
										0xFF, 0x0000,  
										0xF3, 0x0002,   
										0xF3, 0x0004,
										0xC4, 
										0xC5, 0xC6, 
										0xC7 }				
								}
							};
	
/*
const PPU_HAL				ppuHal =
							{
								0xC8,
								0xC9,
								0xCA,
								0xCB,
								
								0x40,
								
								0xCC,
								0x0001,
								0x0002,
								32,
								
								0xC0,					// RTPF status
								0xF3, 					// Picture triggering
								
								{
									{	EDMA_CHA_EXTINT5, &ppuPictureReady0_swi,	
										PPU_REG2ABS(0x00), 
										0xF2, 0x0002,   	// Int enable
										0xFF, 0x0000,  		// chan enable (not needed)
										0xF3, 0x0002,   	// Flush
										0xF3, 0x0004,		// Source select
										0xC1, 				// Framesize
										0xC2, 0xC3, 		// Wordcount lo, hi
										0xC4 				// Pic count
									},
										
									{	EDMA_CHA_EXTINT6, &ppuPictureReady0_swi,	
										PPU_REG2ABS(0x00), 0xF2, 0x0002,   0xFF, 0x0000,  0xF3, 0x0002,   0xF3, 0x0004,
										0xC1, 0xC2, 0xC3, 0xC4 }				
								}
							};
*/							
						
#else
const PPU_HAL				ppuHal =
							{
								// SRAM is not implemented in this firmware
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0,
								
								// RTPF status and Image triggering not implemented
								0x00, 0x00, 
								
								{
									{	EDMA_CHA_EXTINT4, &ppuPictureReady0_swi,	
										PPU_REG2ABS(0x60), 
										0xFF, 0x0000,   	// Int enable (not needed)
										0x34, 0x0001,  		// chan enable
										0x35, 0x0001,   	// Flush
										0xFF, 0x0000,		// Source select (not needed)
										0x33, 				// Framesize
										0x38, 0x37, 		// Wordcount lo, hi
										0x36 				// Pic count
									},
										
									{	EDMA_CHA_EXTINT5, &ppuPictureReady1_swi,	
										PPU_REG2ABS(0x70), 
										0xFF, 0x0000,   	// Int enable (not needed)
										0x44, 0x0001,  		// chan enable
										0x45, 0x0001,   	// Flush
										0xFF, 0x0000,		// Source select (not needed)
										0x43, 				// Framesize
										0x48, 0x47, 		// Wordcount lo, hi
										0x46 				// Pic count
									}				
								}
							};
							
#endif

#ifdef __cplusplus
}
#endif

#endif

