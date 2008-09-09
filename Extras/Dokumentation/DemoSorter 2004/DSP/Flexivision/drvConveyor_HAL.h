/**
* @file
* @author Bernhard Mäder
*/

#define				PPU_MEMORY_BASE				(0x60000000)				///< The base memory location of the ppu
static Uint16 		* ppuMemory = (Uint16*)PPU_MEMORY_BASE;					///< A pointer to the memory mapped ppu interface.

/**
* A structure that stores the hardware information for the conveyor driver.
*/
typedef struct Conveyor_HAL
{
	/**
	* This is the external interrupt ID to which conveyor triggers are signaled
	* by the FPGA. Note: this must also be specified in the DSP/BIOS config utility,
	* so that the conveyor HWI is on the correct external interrupt.
	*/
	Uint32		unExtInterrupt;
	
	/**
	* Defines the time after the last trigger signal after which the conveyor is declared
	* to be standing. In milliseconds.
	*/
	Uint32		unTimeout;
	
} Conveyor_HAL;

#define CONV_REG_INTENABLE_ADDR	0xF2
#define CONV_REG_INTENABLE_MASK	0x08


/**
* Defines how many millimeters the conveyor advances until a new trigger is generated.
*/
//#define CONV_MILLIMETERS_PER_TRIGGER 73
#define CONV_MILLIMETERS_PER_TRIGGER 80

Conveyor_HAL convHal = {
							IRQ_EVT_GPINT7,
							10000
						};
			
