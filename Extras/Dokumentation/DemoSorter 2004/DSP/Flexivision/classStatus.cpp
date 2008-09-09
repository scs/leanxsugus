
#include "classStatus.h"

volatile sStats	gStats;

extern Int SDRAM;
extern Int SDRAM_cached;

/** Allocate single instance. */
CStatus CStatus::m_Instance;

// *************************************************************************

CStatus::CStatus()
{
	gStats.unPPULastErrorWordCountShould = 0;
	gStats.unPPULastErrorWordCountIs = 0;
	
	gStats.unPPUNumPicsGood = 0;
	gStats.unPPUNumPicsBad = 0;
	gStats.unPPUNumPicsDropped = 0;
	
	gStats.unConvNumTriggers = 0;
	
	gStats.unClassNumExchanges = 0;
	gStats.unClassNumExchangeFaults = 0;
	gStats.unClassNumExchangeFaultsLength = 0;
	gStats.unClassNumExchangeFaultsMagic1 = 0;
	gStats.unClassNumExchangeFaultsMagic2 = 0;
	gStats.unClassNumPotTotal = 0;
	gStats.unClassNumPotMerges = 0;
	gStats.unClassNumPotAdds = 0;
}

// *************************************************************************
	
void CStatus::Log( CControl * ctrl )
{
	ctrl->PostReplyMsg(" ----------- Memory Stats ----------- ");
	MEM_Stat stat;
	MEM_stat( 0, &stat );
	ctrl->PostReplyMsg("SRAM: size: %dkb, used: %dkb, max len: %dkb", stat.length / 1024, stat.used / 1024, stat.length / 1024);
	MEM_stat( SDRAM, &stat );
	ctrl->PostReplyMsg("SDRAM: size: %dkb, used: %dkb, max len: %dkb", stat.length / 1024, stat.used / 1024, stat.length / 1024);
	MEM_stat( SDRAM_cached, &stat );
	ctrl->PostReplyMsg("Cached SDRAM: size: %d,kb used: %d, max len: %dkb", stat.length / 1024, stat.used / 1024, stat.length / 1024);
	
	ctrl->PostReplyMsg(" ------------ PPU Stats ------------ ");
	ctrl->PostReplyMsg("Pics good: %d, Pics bad: %d, Pics dropped: %d", gStats.unPPUNumPicsGood, gStats.unPPUNumPicsBad, gStats.unPPUNumPicsDropped);
	ctrl->PostReplyMsg("LastErrorWordount: %d, should: %d", gStats.unPPULastErrorWordCountIs, gStats.unPPULastErrorWordCountShould);	
	
	ctrl->PostReplyMsg(" ----------- Conveyor Stats ----------- ");
	ctrl->PostReplyMsg("Number of triggers: %d", gStats.unConvNumTriggers );
	
	ctrl->PostReplyMsg(" ----------- Classifier Stats ----------- ");
	ctrl->PostReplyMsg("Number of exchanges: %d, %d of which were erroneous (len: %d, w1: %d, w2: %d", 
						gStats.unClassNumExchanges, gStats.unClassNumExchangeFaults,
						gStats.unClassNumExchangeFaultsLength, gStats.unClassNumExchangeFaultsMagic1, gStats.unClassNumExchangeFaultsMagic2 );
	ctrl->PostReplyMsg("Number of potatoes: %d, exchange matches: %d, exchange new: %d",
						gStats.unClassNumPotTotal, gStats.unClassNumPotMerges, gStats.unClassNumPotAdds );
}

// *************************************************************************

