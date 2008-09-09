
#include "tstPPUDriver.h"

#define TST_TRIGGEREVT(x)	EDMA_ESRL |= 0x01<<x

PPU_DevHandle 		ppu;
PictureHandle 		hpic;

Bool		go1 = FALSE;
Bool		go2 = FALSE;
Bool		go3 = FALSE;
Bool		go4 = FALSE;
Bool		go5 = FALSE;

extern Uint16		* ppuMemory;

void PPUDriverTest_tsk();

void PPUDriverTest()
{
	Int 				i;
	unsigned short 		* pixels;
	
	for (i=0; i<255; i++)	ppuMemory[i] = 0x55AA;
	
	LOG_printf(&trace, "PPU driver test: Started.");
	
	ppu = ppuOpen(NULL);
	if (ppu != NULL) LOG_printf(&trace, "PPU driver test: Opened.");
	assert (ppu!=NULL);
	
	
	// *********************************************************
	//  Test 1
	// *********************************************************
	// Test if the DMAs are correctly calculated and linked. This 
	// is done by opening the first two channels with three different 
	// configurations:
	// 1) Only one frame, which is not fully used.
	// 2) Multiple frames, no last frame.
	// 3) Multiple frames, with a last frame.
	//
	// The procedure is as follows:
	// Setup the channel so that a single buffer does NOT take a
	// buffer with a size of a multiple of 4, so that there are a few
	// spare bytes at the end of the buffer. Since the buffer is initialized
	// with 0xFF ( Be sure to define _BFQ_INIT_BUFFERS, so that the buffers
	// are initialized correctly), we can check if the DMA has written to the
	// correct bytes.
	//
	// In the PPU_HAL.h, there need to be specific frame sizes:
	// Channel 0: Framesize = 16
	// Channel 1: Framesize = 14
	
	
	
	// Start the writer task
	TSK_create((Fxn)PPUDriverTest_tsk, NULL, NULL);
	
	// Configuration 1): 	Buffer size is 3(width)*1(height)*2(Bytes per pixel) + 12 (pic header) = 18,
	//						which gets rounded to 20, so we have 2 spare bytes at the end of the buffer.
	//						Framesize is 16.
	assert( SYS_OK == ppuOpenChannel(ppu, 0, 16, 3, 1, 4) );
	assert( SYS_OK == ppuEnableChannel(ppu, 0) );	
	go1 = TRUE;
	assert( SYS_OK == ppuGetPicture(ppu, 0, &hpic, SYS_FOREVER));
	assert( (hpic->Type == PICT_RGB565) && (hpic->Width == 3) && (hpic->Height == 1) ); 	// check the pic's header
	pixels = (unsigned short *)hpic->Data;
	for (i=0; i<3; i++) assert( pixels[i] == 0x55AA);							// check that the pixel data was read correctly
	assert( pixels[3] == 0xFFFF);												// Check that the DMA didn't write over the picture's boundaries
	LOG_printf(&trace, "PPU driver test: received a picture on channel 0... ok.");
	
	// Configuration 2): 	Buffer size is 14(width)*5(height)*1(Bytes per pixel) + 12 (pic header) = 82,
	//						which gets rounded to 84, so we have 2 spare bytes at the end of the buffer.
	//						Framesize is 14, 5 frames need to be transmitted.
	assert( SYS_OK == ppuOpenChannel(ppu, 1, 8, 14, 5, 4) );
	assert( SYS_OK == ppuEnableChannel(ppu, 1) );	
	go2 = TRUE;
	assert( SYS_OK == ppuGetPicture(ppu, 1, &hpic, SYS_FOREVER));
	assert( (hpic->Type == PICT_GREY8) && (hpic->Width == 14) && (hpic->Height == 5) ); 	// check the pic's header
	pixels = (unsigned short *)hpic->Data;
	for (i=0; i<35; i++) assert( pixels[i] == 0x55AA);							// check that the pixel data was read correctly (note: effective pixels are 8bit, pixels[] is 16 bit
	assert( pixels[35] == 0xFFFF);												// Check that the DMA didn't write over the picture's boundaries
	LOG_printf(&trace, "PPU driver test: received a picture on channel 1... ok.");
	
	// Configuration 3): 	Buffer size is 9(width)*10(height)*3(Bytes per pixel) + 12 (pic header) = 282,
	//						Framesize is 16, 16.875 frames needed.
	assert( SYS_OK == ppuCloseChannel(ppu, 0));
	assert( SYS_OK == ppuOpenChannel(ppu, 0, 24, 9, 10, 4) );
	assert( SYS_OK == ppuEnableChannel(ppu, 0) );	
	go3 = TRUE;
	assert( SYS_OK == ppuGetPicture(ppu, 0, &hpic, SYS_FOREVER));
	assert( (hpic->Type == PICT_RGB888) && (hpic->Width == 9) && (hpic->Height == 10) );	// check the pic's header
	pixels = (unsigned short *)hpic->Data;
	for (i=0; i<135; i++) assert( pixels[i] == 0x55AA);							// check that the pixel data was read correctly
	assert( pixels[135] == 0xFFFF);												// Check that the DMA didn't write over the picture's boundaries
	LOG_printf(&trace, "PPU driver test: received a picture on channel 0... ok.");
		
	go4 = TRUE;
				
	ppuClose(ppu);	
	
	LOG_printf(&trace, "PPU driver test: finished succesfully.");
}

void PPUDriverTest_tsk()
{
	Int j;
	
	TSK_sleep(2);
	
	LOG_printf(&trace, "PPU driver test: writer task started.");
	
	
	while(go1 == FALSE) TSK_sleep(1);	
	LOG_printf(&trace, "PPU driver test: writer task: go1");
	TST_TRIGGEREVT(4);
	assert(go2==FALSE);			// check if the reader has not yet (faultly) finished
	
	TSK_sleep(2);
	
	
	while(go2 == FALSE) TSK_sleep(1);	
	LOG_printf(&trace, "PPU driver test: writer task: go2");
	TST_TRIGGEREVT(5);
	TSK_sleep(3);
	TST_TRIGGEREVT(5);
	TSK_sleep(3);
	TST_TRIGGEREVT(5);
	TSK_sleep(3);
	TST_TRIGGEREVT(5);
	TSK_sleep(3);
	TST_TRIGGEREVT(5);
	assert(go3==FALSE);			// check if the reader has not yet (faultly) finished

	TSK_sleep(2);	
	
	while(go3 == FALSE) TSK_sleep(1);	
	LOG_printf(&trace, "PPU driver test: writer task: go3");
	for (j=0; j<17; j++)
	{
		TSK_sleep(1);
		TST_TRIGGEREVT(4);	
	}
	assert(go4==FALSE);			// check if the reader has not yet (faultly) finished
	
	LOG_printf(&trace, "PPU driver test: writer task: done.");
}
