
#include "tstThroughput.h"

#include <assert.h>
#include <stdio.h>

#include <csl_timer.h>

#include "libEDMAManager.h"

static TIMER_Handle 	hTimer;

void		tstStartTimer()
{
	// Open and start timer	
	hTimer = TIMER_open(TIMER_DEVANY, 0);
	assert( hTimer != INV);
	
	TIMER_configArgs(hTimer,
		0x000002C0, /* ctl */
		0xFFFFFFFF, /* prd */
		0x00000000 	/* cnt */ );
		
	TIMER_start( hTimer );
}			

int			tstStopTimer()
{
	Uint32 counts;
	
	TIMER_pause( hTimer );
	counts = TIMER_getCount( hTimer );
	printf("Cycle count: %d\n", counts*8 );
	printf("in ms: %f\n", (float)counts * 8.0 / 450000000.0 * 1000.0 );
	
	TIMER_close( hTimer );	
	
	return counts;
}


void		tstThroughput()
{
/*
	int				i;
	Uint32 *		pSDRAM = (Uint32*)0x84000000;
	
	#define			TESTSIZE 0x1000000
	
	// Prime memory
	for (i=0; i<TESTSIZE; i++)
		pSDRAM[i] = i;
		
	printf("\nEMIFA test 1\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x80000000, (Ptr)(0x80000000 + TESTSIZE), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_32, 
				TESTSIZE/4, 1, 1);	
	tstStopTimer();
	
	printf("\nEMIFA test 2\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x80000000, (Ptr)(0x80000000 + TESTSIZE), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_32, 
				TESTSIZE/4, 0, 1);	
	tstStopTimer();
	
	printf("\nEMIFA test 3\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x80000000, (Ptr)(0x80000000 + TESTSIZE), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_16, 
				TESTSIZE/4, 0, 1);	
	tstStopTimer();
	
	printf("\nEMIFB test 1\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x60000000, (Ptr)(0x80000000), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_32, 
				TESTSIZE/4, 0, 1);
	tstStopTimer();
	
	printf("\nEMIFB test 2\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x60000000, (Ptr)(0x80000000), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_16, 
				TESTSIZE/4, 0, 1);
	tstStopTimer();
	
	printf("\nEMIFB test 2 nochmals\n");
	tstStartTimer();	
	edmaCopy( (Ptr)0x60000000, (Ptr)(0x80000000), 
				EDMA_PRI_MED, EDMA_ELEMSIZE_16, 
				TESTSIZE/4, 0, 1);
	tstStopTimer();
	
	printf("done\n");
	*/
}

