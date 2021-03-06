/*   Do *not* directly modify this file.  It was    */
/*   generated by the Configuration Tool; any  */
/*   changes risk being overwritten.                */

/* INPUT flexivision.cdb */

#define CHIP_6414 1

/*  Include Header Files  */
#include <std.h>
#include <prd.h>
#include <hst.h>
#include <swi.h>
#include <tsk.h>
#include <log.h>
#include <sem.h>
#include <mbx.h>
#include <lck.h>
#include <sts.h>
#include <csl.h>
#include <csl_edma.h>
#include <csl_emifa.h>
#include <csl_emifb.h>

#ifdef __cplusplus
extern "C" {
#endif

extern far PRD_Obj PRD_Watchdog;
extern far PRD_Obj PRD_OutputDispatcher;
extern far HST_Obj RTA_fromHost;
extern far HST_Obj RTA_toHost;
extern far SWI_Obj PRD_swi;
extern far SWI_Obj KNL_swi;
extern far SWI_Obj ppuPictureReady1_swi;
extern far SWI_Obj ppuPictureReady2_swi;
extern far SWI_Obj ppuPictureReady0_swi;
extern far SWI_Obj edmaCopyDone_swi;
extern far SWI_Obj serPPUSerial_swi;
extern far TSK_Obj TSK_idle;
extern far TSK_Obj ctrlControl_tsk;
extern far TSK_Obj hpiOutPacket_tsk;
extern far TSK_Obj procImageProcessing_tsk;
extern far TSK_Obj hpiInPacket_tsk;
extern far LOG_Obj LOG_system;
extern far LOG_Obj trace;
extern far SEM_Obj ppuSerial_sem;
extern far SEM_Obj serSerial_sem;
extern far SEM_Obj hpiInPacket_sem;
extern far MBX_Obj hpiOutPacket_mbx;
extern far LCK_Obj gMemory_lck;
extern far STS_Obj IDL_busyObj;
extern far EDMA_Config edmaCfg0;
extern far EMIFA_Config emifaCfgSDRAM;
extern far EMIFB_Config emifbCfg0;
extern far void CSL_cfgInit();

#ifdef __cplusplus
}
#endif /* extern "C" */
