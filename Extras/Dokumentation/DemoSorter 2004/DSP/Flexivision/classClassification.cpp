
#include "classClassification.h"

#include "vision/classVisVision.h"
#include "vision/classVisObjectManager.h"

#ifndef _WINDOWS // ------- Don't compile on windows -----

#include "classStatus.h"
#include "classWatchdog.h"
#include "classJetControl.h"
#include "libHelpers.h"

#include "drvPPUSerial.h"
#include "drvConveyor.h"
#include "drvHighResTimer.h"

#include "libLED.h"
#include "libDebug.h"

#include <clk.h>

#else
#define dbgLog LogMsg
#endif // ------------------------------------------------

/**
* Only keep a reference to a classification object. The object must be created
* from outside this class so that it can be constructed at a reasonable time.
*/
CClassification * CClassification::g_pClassification = NULL;

// *************************************************************************

#ifndef _WINDOWS // ------- Don't compile on windows -----

void TSK_Classification_funct( void )
{
	// Wait until everything has settled.
	TSK_sleep( hlpMsToTicks( 1000 ) );

	// wait until the object has been constructed
	while ( CClassification::Instance() == NULL )
		TSK_sleep(100);
			
	CClassification::Instance()->EnterTask();	
}

#endif // ------------------------------------------------

// *************************************************************************



CClassification::CClassification()
	:	CVisComponent( "Classifier", "Classifier" ),	
		
		m_propMergeSearchDistance("MergeSearchDistance"),
		
		m_propSplitStrictness("SplitStrictness"),
		m_propShapeStrictness("ShapeStrictness"),
		m_propGreenStrictness("GreenStrictness"),
		m_propColorStrictness("ColorStrictness"),		
		
		m_propNumTriggerPulsesDelay("TriggersDelay"),
		m_propSpatialDelay("SpatialDelay"),
		m_propTemporalDelay("TempDelay"),
		m_propSmallActivationLength("SmallActiveLength"),
		m_propMediumActivationLength("MediumActiveLength"),
		m_propLargeActivationLength("LargeActiveLength"),	
		m_propMediumOffsetTime("MediumOffsetTime"),
		m_propLargeOffsetTime("LargeOffsetTime"),
		m_propMaxSmallVolume("MaxSmallVolume"),
		m_propMinLargeVolume("MinLargeVolume")
		
{
	m_pPotatoTable = NULL;
	m_csmServiceMode = CSM_NORMAL;
	
	// Clear the ejection tables array
	InitEjectionTables();
		
	m_unCurrentTriggerPulse = 0;
	
	// Init properties
	m_propMergeSearchDistance.Init( this, CVisProperty::PT_INTEGER, &m_nMergeSearchDistance );
	m_nMergeSearchDistance = 60;
	
	// Init strictnesses
	m_propSplitStrictness.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpSplitStrictness, PotatoObject::FP_FRACTIONAL_BITS );
	m_propShapeStrictness.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpShapeStrictness, PotatoObject::FP_FRACTIONAL_BITS );
	m_propGreenStrictness.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpGreenStrictness, PotatoObject::FP_FRACTIONAL_BITS );
	m_propColorStrictness.Init( this, CVisProperty::PT_FIXEDPOINT, &m_fpColorStrictness, PotatoObject::FP_FRACTIONAL_BITS );
	m_fpSplitStrictness = F2FP( 5, PotatoObject::FP_FRACTIONAL_BITS );
	m_fpShapeStrictness = F2FP( 5, PotatoObject::FP_FRACTIONAL_BITS );	
	m_fpGreenStrictness = F2FP( 5, PotatoObject::FP_FRACTIONAL_BITS );
	m_fpColorStrictness = F2FP( 5, PotatoObject::FP_FRACTIONAL_BITS );
		
	// The following parameters are ratios of the total seen pictures.
	m_nBadColorPicsAllowed 		= (Int32)( 0.10 * (1<<PotatoObject::FP_FRACTIONAL_BITS) );
	m_nBadGreenColorPicsAllowed = (Int32)( 0.10 * (1<<PotatoObject::FP_FRACTIONAL_BITS) );
	m_nBadFormPicsAllowed 		= (Int32)( 0.10 * (1<<PotatoObject::FP_FRACTIONAL_BITS) );
	m_nSplitPicsAllowed 		= (Int32)( 0.0 * (1<<PotatoObject::FP_FRACTIONAL_BITS) );
	
	m_nNumTriggerPulsesDelay 	= 7;
	m_propNumTriggerPulsesDelay.Init( this, CVisProperty::PT_INTEGER, &m_nNumTriggerPulsesDelay );
	
	m_nSpatialDelay 			= 80;//509;
	m_propSpatialDelay.Init( this, CVisProperty::PT_INTEGER, &m_nSpatialDelay);
	
	m_nTemporalDelay			= 0;
	m_propTemporalDelay.Init( this, CVisProperty::PT_INTEGER, &m_nTemporalDelay);
	
	m_nSmallActivationLength	= 450;//40;
	m_propSmallActivationLength.Init( this, CVisProperty::PT_INTEGER, &m_nSmallActivationLength);
	
	m_nMediumActivationLength	= 500;//120;
	m_propMediumActivationLength.Init( this, CVisProperty::PT_INTEGER, &m_nMediumActivationLength);
	
	m_nLargeActivationLength	= 500;//150;
	m_propLargeActivationLength.Init( this, CVisProperty::PT_INTEGER, &m_nLargeActivationLength);
	
	m_nMediumOffsetTime = 0;//30;
	m_propMediumOffsetTime.Init( this, CVisProperty::PT_INTEGER, &m_nMediumOffsetTime );
	
	m_nLargeOffsetTime = 0;//45;
	m_propLargeOffsetTime.Init( this, CVisProperty::PT_INTEGER, &m_nLargeOffsetTime );		
	
	m_nMaxSmallVolume = 99999999;//60000;
	m_propMaxSmallVolume.Init( this, CVisProperty::PT_INTEGER, &m_nMaxSmallVolume);
	
	m_nMinLargeVolume = 100000000;//140000;
	m_propMinLargeVolume.Init( this, CVisProperty::PT_INTEGER, &m_nMinLargeVolume ); 
	
	m_nNumLanes = 10;
	m_nConveyorWidth = 1260;
	
	// Set the reference to this instance so that others may access this singleton
	g_pClassification = this;
	
	// Apply the strictness so that we have valid data on the threshold variables.
	ApplyStrictness();
}	

// *************************************************************************
	
void CClassification::EnterTask()
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	Uint32 unIntervalTicks;
	
	Bool bLed2 = FALSE;
	ledLight( 2, FALSE );
	
	// Initialize the conveyor device driver and register the trigger semaphore.
	convInit( );
	SEM_new( &m_semTrigger, 0 );
	
	convRegisterSemTrigger( &m_semTrigger, &m_bTrigger );	
	
	// Reset statstics
	GetStats( NULL, TRUE );

	// Initialize the high res timer
	timeInit();
	
	// Pre-calculate the maximum wait time for the semaphore.
	unIntervalTicks = hlpMsToTicks( 1000 * MAXWAIT_SECONDS );
	
	// Open the interlink UART channel and configure it.
	m_hPPUSerial = serOpen( INTERLINK_UART_CHAN, sizeof( ClassificationTable ) *4 );
	assertLog( m_hPPUSerial != NULL );
	serConfigChannel( m_hPPUSerial, 115000, FALSE, FALSE, FALSE );	
	
	// Initialize the jet controller
	TSK_sleep( 2000 );
	CJetControl::Instance()->Init();
	
	// Start watchdog and set it to twice the time of our maximum interval.
	m_unWatchId = CWatchdog::Instance()->AddToWatch( "Classification", MAXWAIT_SECONDS * 2 );
	CWatchdog::Instance()->EnableWatch( m_unWatchId, TRUE );
	
	while( 1 )
	{
			
		// Wait for a trigger signal, timeout at some time to reset watchdog
		if ( SEM_pend( &m_semTrigger, unIntervalTicks ) )
		{
			// Succesfully received a trigger
			ledLight( 2, bLed2 );
			bLed2 = !bLed2;			
						
			// Get the trigger time as exact as possible.
			Uint32 unCurTime = convGetLastTriggerTime();
						
			// Increment our trigger counter.
			m_unCurrentTriggerPulse++;
			// dbgLog( "Entering Trigger %d", m_unCurrentTriggerPulse );
			
			// Store that trigger in the stats. This increments the number of possible potatoes by the number
			// of lanes.
			m_sClassificationStats.unNumPossible += m_nNumLanes;
			
			// See if we're in service mode and handle it.
			if ( m_csmServiceMode != CSM_NORMAL )
			{
				if (   (m_csmServiceMode == CSM_ADJUST_SMALL_EJECTION_PARAMS )
					|| (m_csmServiceMode == CSM_ADJUST_MEDIUM_EJECTION_PARAMS )
					|| (m_csmServiceMode == CSM_ADJUST_LARGE_EJECTION_PARAMS ) )
					ServiceGenParamAdjustCommands( unCurTime );
			}
			else
			{		
				// If not in service mode, see if we've got any due ejections to make and generate
				// the commands for it.
				EjectionTable * pTable;
				
				// See if any ejections are due and generate the jetcontrol commands, which are then
				// sent to the jet control.
				pTable = GetDueEjectionTable();
				if ( pTable != NULL )
				{
					// Generate ejection commands, but only if we're in classification mode.
					if ( m_eOperationMode == OP_CLASSIFICATION )
					{
						GenerateEjectionCommands( pTable, unCurTime );
					}
					ReleaseEjectionTable( pTable );
				}
							
				// Now we have to classify the potatoes and create the ejection table for the current
				// line. Only build the table if we've got a reference to the global potato table
				if ( m_pPotatoTable != NULL )
				{												
					// Build the local classification table (i.e. the per-frame classification)
					BuildTable( &m_LocalClassificationTable,  m_pPotatoTable );
					
					// Only exchange tables if we're in classification mode.
					if ( m_eOperationMode == OP_CLASSIFICATION )
					{					
						// Exchange the table with the other DSP
						if ( ExchangeTables( &m_LocalClassificationTable, &m_ForeignClassificationTable ) == TRUE )
						{
							MergeTables( &m_LocalClassificationTable, &m_ForeignClassificationTable );
						}
					}
						
					// Classify the potatoes using a new ejection table. This, we'll
					// have to do even in calibration mode, because of the statistics
					// for the GUI.
					pTable = GetNewEjectionTable( m_nNumTriggerPulsesDelay );
					if ( pTable != NULL )
					{
						ClassifyPotatoes( & m_LocalClassificationTable, &m_ForeignClassificationTable, pTable );
					}	
					
				} // if potatoobject table accessible
				
			} // if not in servicemode
			
		} // if trigger occured
		
		else
		{
			// Clean the objects list from time to time, if we don't receive trigger signals for
			// a long time. This prevents the number of objects from growing to big.
			CleanObjects( m_pPotatoTable );	
			
			// Check for service operation
			if ( m_csmServiceMode == CSM_CHECK_JETS )
				ServiceGenJetCheckCommands();	
						
		}
		
		// See if any of the strictness values changed and apply it to the properties
		if (   m_propSplitStrictness.HasChanged()
			|| m_propShapeStrictness.HasChanged()
			|| m_propGreenStrictness.HasChanged()
			|| m_propColorStrictness.HasChanged() )
		{
			ApplyStrictness();		
		}
		
		// Signal the watchdog.
		CWatchdog::Instance()->SignalAlive( m_unWatchId );	
		
	} // while(1)

#endif // ------------------------------------------------

}

// *************************************************************************

void CClassification::SignalTrigger()
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	SEM_post( &m_semTrigger );

#endif // ------------------------------------------------
}

// *************************************************************************

CClassification * CClassification::Instance()
{
	return g_pClassification;
}

// *************************************************************************

void CClassification::SetPotatoTable( PotatoList * pTable )
{
	m_pPotatoTable = pTable;
}

// *************************************************************************

void CClassification::GetStats( ClassificationStats * stats, Bool bReset )
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	// First, disable all SWI, so that we're not be disturbed here. There shouldn't
	// be any critical task running at the moment, since this function will mainly be
	// called from the control task.
	
//	SWI_disable();
	
	// copy all fields
	if ( stats != NULL )
	{
		stats->unElapsedTime = timeToMs( timeGetHighResTime() - m_sClassificationStats.unElapsedTime );
		stats->unNumProcessed = m_sClassificationStats.unNumProcessed;
		stats->unNumPossible = m_sClassificationStats.unNumPossible;
		stats->fp16ConveyorSpeed = convGetMeasuredSpeed();
		stats->unNumRejectedTotal = m_sClassificationStats.unNumRejectedTotal;
		stats->unNumRejectedSplit = m_sClassificationStats.unNumRejectedSplit;
		stats->unNumRejectedColor = m_sClassificationStats.unNumRejectedColor;
		stats->unNumRejectedGreen = m_sClassificationStats.unNumRejectedGreen;
		stats->unNumRejectedShape = m_sClassificationStats.unNumRejectedShape;
	}
	
	
	if ( bReset )
	{
		m_sClassificationStats.unElapsedTime = timeGetHighResTime();
		m_sClassificationStats.unNumProcessed = 0;
		m_sClassificationStats.unNumPossible = 0;
		m_sClassificationStats.unNumRejectedTotal = 0;
		m_sClassificationStats.unNumRejectedSplit = 0;
		m_sClassificationStats.unNumRejectedColor = 0;
		m_sClassificationStats.unNumRejectedGreen = 0;
		m_sClassificationStats.unNumRejectedShape = 0;
	}

	//	SWI_enable();

#endif // ------------------------------------------------

}


// *************************************************************************

void CClassification::SetServiceMode( ClassifierServiceMode csmMode, Int32 nParam )
{
	m_csmServiceMode = csmMode;
	m_nServiceParam = nParam;
}

// *************************************************************************

void CClassification::ServiceGenJetCheckCommands( )
{
	
#ifndef _WINDOWS // ------- Don't compile on windows -----

	JetCommand 	cmd;
	Uint32		unTime;
	
	unTime = timeGetHighResTime() + timeFromMs( 50 );
	
	for ( Int i=0; i<MAX_LANES; i++ )
	{
		// clear the command
		for ( Int i=0; i<JetCommand::MAX_JETS; i++)
			cmd.aryJetState[i] = FALSE;
			
		// activate only one jet
		cmd.aryJetState[i] = TRUE;
		cmd.unCmdTime = unTime;
		
		// and add the command
		CJetControl::Instance()->AddCommand( &cmd );
		
		// increase time
		unTime += timeFromMs( 200 );		
	}
	
	// send a clear all command
	for ( Int i=0; i<JetCommand::MAX_JETS; i++)
		cmd.aryJetState[i] = FALSE;
	cmd.unCmdTime = unTime;
	CJetControl::Instance()->AddCommand( &cmd );

#endif // ------------------------------------------------

}

// *************************************************************************

void  CClassification::ServiceGenParamAdjustCommands( Uint32 unTriggerTime )
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	EjectionForce ef;
	EjectionTable table;
	
	// DEBUG
	//dbgLog( "servicemode: %d", m_csmServiceMode );
	
	// Extract the ejection force of the current service mode.
	switch ( m_csmServiceMode )
	{
	case CSM_ADJUST_SMALL_EJECTION_PARAMS:
		ef = EF_SMALL;
		break;
		
	case CSM_ADJUST_MEDIUM_EJECTION_PARAMS:
		ef = EF_MEDIUM;
		break;
		
	case CSM_ADJUST_LARGE_EJECTION_PARAMS:
		ef = EF_LARGE;
		break;
		
	default:
		return;
	}
	
	// Construct an ejection table that always ejects potatoes on the specified lane with the desired
	// ejection force.
	for ( Int i=0; i<MAX_LANES; i++ )
		table.efEject[i] = EF_NONE;
		
	if (( m_nServiceParam >= 0) && (m_nServiceParam < MAX_LANES ) )
		table.efEject[m_nServiceParam] = ef;
	
	// ... and let the standard ejection commands generating function handle the rest.
	GenerateEjectionCommands( &table, unTriggerTime );
	
#endif // ------------------------------------------------
	
}

// *************************************************************************

void CClassification::CleanObjects( PotatoList * restrict pPotatoTable )
{
		
	for ( Int i=0; i<PotatoList::MAX_OBJECTS; i++)
	{
		PotatoObject * obj = &( pPotatoTable->pObjects[i] );
		
		// Valid objects that were either marked for ejection or for discarding are deleted here.
		if ( ( obj->bValid) && (obj->bDropIt || obj->bDiscard) )
		{
			// Delete the object from the list
			obj->bValid = FALSE;
		}		
	}
}

// *************************************************************************

void CClassification::BuildTable( ClassificationTable * restrict pTable, PotatoList * restrict pPotatoTable)
{
	static Bool 	bLed1 = TRUE;
	char			strLogText[512];
	int				nLogTextChars;
	
	// Clear local table
	MemSet( pTable, 0, sizeof( ClassificationTable ) );

#ifndef _WINDOWS // ------- Don't compile on windows -----
	
	ledLight( 1, bLed1 );
	bLed1 = ! bLed1;
	
#endif // ------------------------------------------------

		
	for ( Int i=0; i<PotatoList::MAX_OBJECTS; i++)
	{
		PotatoObject * obj = &( pPotatoTable->pObjects[i] );
		
		// First see if the object is valid at all.
		if (obj->bValid)
		{			
			// Do we need to discard the object?
			if (obj->bDiscard)
			{
				obj->bValid = false;
			}
			
			// Only go further with the inspection, if the object has been predicted 
			// to be in the drop zone.
			else if (obj->bDropIt)
			{
				// Either the potato is valid for ejection, so we generete the ejection commands,
				// or it's not. The object is deleted from the list in both cases.
				if ( obj->bValidEjection )
				{		
					Int i;

					// ----------------------------------------------
					//  Build the table
					// ----------------------------------------------
					
					Int index = pTable->unNumEntries;
					Int numframes = obj->unCurrentImageNum;
					
					pTable->aryEntry[index].nPosition_mm_X = obj->nLastSeenPos_mm_X;
					pTable->aryEntry[index].unNumTotalFrames = numframes;
					
					pTable->aryEntry[index].unNumFramesBadColor = 0;
					pTable->aryEntry[index].unNumFramesGreenColor = 0;
					pTable->aryEntry[index].unNumFramesBadForm = 0;
					pTable->aryEntry[index].unNumFramesSplit = 0;
					
					for ( i=0; i<numframes; i++ )
					{								
						// Apply threshold for bad color
						if ( obj->unpClassificationColor[i] > m_fpBadColorThreshold )
							pTable->aryEntry[index].unNumFramesBadColor++;
							
						// Apply threshold for green color
						if ( obj->unpClassificationGreen[i] > m_fpGreenColorThreshold )
							pTable->aryEntry[index].unNumFramesGreenColor++;
							
						// Apply threshold for bad form
						if ( obj->unpClassificationForm[i] > m_fpBadShapeThreshold )
							pTable->aryEntry[index].unNumFramesBadForm++;
						
						// Apply threshold for splits
						if ( obj->unpClassificationSplit[i] > m_nSplitThreshold )
							pTable->aryEntry[index].unNumFramesSplit++;
					}
										
					// Transfer the potato size
					pTable->aryEntry[index].unPotatoLength = (Uint16)(obj->unPotatoLength);
					pTable->aryEntry[index].unPotatoWidth = (Uint16)(obj->unPotatoWidth);
					
					// Clear the link to the foreign table.
					pTable->aryEntry[index].nOtherTableIndex = -1;
					
					// Increment the total entries counter so that the transmission part knows
					// how many entries are in the table. Do a saturated addition so we get no
					// memory errors.
					if ( pTable->unNumEntries < MAX_TABLE_ENTRIES-1 )
						pTable->unNumEntries++;
					
					
					// -------------------------------------------
					//  Create the Log output
					// -------------------------------------------
					// The form of the log output is organized in a way that allows the file to be taken directly
					// into excel and evaluated by the LogAnalyzer
					
					// Start to build the log text
					dbgLog("log<< \tpotato\t%d\tlane\t%d\ttrigger\t%d", i, this->MapPosToJet( obj->nLastSeenPos_mm_X ), m_unCurrentTriggerPulse );
					
					// Add the color information to the log.
					nLogTextChars = sprintf( strLogText, "log<< \tcolor");
					for ( i=0; i<numframes; i++ )
						nLogTextChars += sprintf( strLogText + nLogTextChars, "\t.%03d", (obj->unpClassificationColor[i]*1000) >> PotatoObject::FP_FRACTIONAL_BITS );
					dbgLog( strLogText );
						
					// Add the green information to the log.
					nLogTextChars = sprintf( strLogText, "log<< \tgreen");
					for ( i=0; i<numframes; i++ )
						nLogTextChars += sprintf( strLogText + nLogTextChars, "\t.%05d", ( obj->unpClassificationGreen[i] * 10000) >> PotatoObject::FP_FRACTIONAL_BITS);
					dbgLog( strLogText );
										
					// Add the green information to the log.
					nLogTextChars = sprintf( strLogText, "log<< \tshape");
					for ( i=0; i<numframes; i++ )
						nLogTextChars += sprintf( strLogText + nLogTextChars, "\t%3d", obj->unpClassificationForm[i] >> PotatoObject::FP_FRACTIONAL_BITS );
					dbgLog( strLogText );
					
					// Add the split information to the log.
					nLogTextChars = sprintf( strLogText, "log<< \tsplit");
					for ( i=0; i<numframes; i++ )
						nLogTextChars += sprintf( strLogText + nLogTextChars, "\t%01d", obj->unpClassificationSplit[i]);
					dbgLog( strLogText );
					
					// add two blank lines
					dbgLog( "log<<" );
					dbgLog( "log<<" );
				}
				
				// Now delete the object from the list
				obj->bValid = FALSE;					
			} // if dropit
			
		} // if valid
		
	} // for all objects
	
}

// *************************************************************************

Bool CClassification::ExchangeTables( ClassificationTable * restrict pLocalTable, ClassificationTable * restrict pForeignTable )
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	Uint32 unSize;
	Uint32 unRecvd;
	
		
	// Calculate buffer size. We deliberately send the whole table to
	// get a steady transfer rate. This is a real-time critical task
	// anyway, so that must be possible.
	unSize = sizeof(ClassificationTable);	
		
	// Set the magic number fields of both tables. We deliberately use a wrong number for
	// the foreign table so we can see whether the value has really been overwritten.
	pLocalTable->unMagicNumber1 = MAGIC_NUMBER1;
	pLocalTable->unMagicNumber2 = MAGIC_NUMBER2;
	pForeignTable->unMagicNumber1 = ~MAGIC_NUMBER1;
	pForeignTable->unMagicNumber2 = ~MAGIC_NUMBER2;
		
	// Write
	serWrite( m_hPPUSerial, (char*)pLocalTable, unSize );
	
	// Try to receive as many bytes as we've just sent. Abort if that's not the case.
	// Since serRead is non-blocking, we have to sleep a little before calling it.
	TSK_sleep( hlpMsToTicks( EXCHANGE_WAIT_MS ) );
	unRecvd = serRead( m_hPPUSerial, (char*)pForeignTable, unSize );
	
	/*
	dbgLog("Sent %d, Received %d (%d Bytes), Tx:%d, Rx:%d",
		 	pLocalTable->unNumEntries, 
			pForeignTable->unNumEntries, 
			unRecvd,
			serLevelTx( m_hPPUSerial), serLevelRx( m_hPPUSerial) );			
	*/		
	
	// Max the number of entries to an allowed value, just for safety.
	pLocalTable->unNumEntries = min( pLocalTable->unNumEntries, MAX_TABLE_ENTRIES );
	pForeignTable->unNumEntries = min( pForeignTable->unNumEntries, MAX_TABLE_ENTRIES );
	
	// Update statistics.
	gStats.unClassNumExchanges++;
	
	// If either the number of bytes received or the magic number are wrong, handle the error.
	if ( (unSize != unRecvd) 
			|| (pForeignTable->unMagicNumber1 != MAGIC_NUMBER1)
			|| (pForeignTable->unMagicNumber2 != MAGIC_NUMBER2) )
	{
		gStats.unClassNumExchangeFaults++;
		
		if ( unSize != unRecvd )
			gStats.unClassNumExchangeFaultsLength++;
		
		if (pForeignTable->unMagicNumber1 != MAGIC_NUMBER1)
		{
		
		//	dbgLog("Magic1: read %08X instead of %08X", pForeignTable->unMagicNumber1, MAGIC_NUMBER1 );
			gStats.unClassNumExchangeFaultsMagic1++;
		}
		
			
		if (pForeignTable->unMagicNumber2 != MAGIC_NUMBER2)	
		{
		//	dbgLog("Magic2: read %08X instead of %08X", pForeignTable->unMagicNumber2, MAGIC_NUMBER2 );
			gStats.unClassNumExchangeFaultsMagic2++;
		}

		// Make the table empty, so that we just won't use it for this time.
		pForeignTable->unNumEntries = 0;
		
		// Flush the UART
		serFlushRx( m_hPPUSerial );
		serFlushTx( m_hPPUSerial );
		
		return FALSE;
	}

	// Flush the UART
	serFlushRx( m_hPPUSerial );
	serFlushTx( m_hPPUSerial );

#else // ------------------------------------------------

	return FALSE;
	
#endif // ------------------------------------------------
	
	return TRUE;

}

// *************************************************************************

void CClassification::MergeTables( ClassificationTable * restrict pLocalTable, const ClassificationTable * restrict pForeignTable)
{

#ifndef _WINDOWS // ------- Don't compile on windows -----

	// For all objects in the foreign list...
	for ( int foreign=0; foreign < pForeignTable->unNumEntries; foreign++)
	{
		Int nMinDist = 0x0FFFFFFF;
		Int nMinIndex = 0;
		
		//DEBUG
		//dbgLog("log<< foreign %d: pos: %d, searching...", foreign, pForeignTable->aryEntry[foreign].nPosition_mm_X);
		
		// ...find the potato in the local list with the smallest distance to it.
		for ( int local=0; local<pLocalTable->unNumEntries; local++)
		{
			Int nDist;
			bool bNearer;
			bool bFound;
			bool bAlreadyUsed;
			
			nDist = abs( pLocalTable->aryEntry[local].nPosition_mm_X
						- pForeignTable->aryEntry[foreign].nPosition_mm_X );
						
			//dbgLog("log<<    local %d: pos: %d -> dist %d", local, pLocalTable->aryEntry[local].nPosition_mm_X, nDist);
						
			// Pre-calculate some flags, so we get an optimized loop here with no jumps and calls.
			bAlreadyUsed = (pLocalTable->aryEntry[local].nOtherTableIndex != -1);
			bNearer = (nDist<nMinDist);	
			bFound = (!bAlreadyUsed && bNearer);

			// Only store the index and the distance, if the distance is smaller as well as
			// the local object isn't already matched.
			if ( bFound )
			{
				nMinDist = nDist;
				nMinIndex = local;
			}
		}
		
		// Now see if the object is in our search radius.
		if ( nMinDist < m_nMergeSearchDistance )
		{
			//dbgLog("log<<    choosing %d", nMinIndex );
			
			// Mark the local classification table's object. That's all we do for the moment,
			// the values are merged below.
			pLocalTable->aryEntry[nMinIndex].nOtherTableIndex = foreign;
			
			gStats.unClassNumPotMerges++;
		}
		// Otherwise, we have to handle an object that was only seen by the other
		// camera and thus must be added to the local table.
		else
		{
			// See if there's space left
			if ( pLocalTable->unNumEntries < MAX_TABLE_ENTRIES-1 )
			{
				// And simply set the reference to the foreign table. The entry
				// is zeroed anyway and the values will be copied during the actual
				// merger process below.
				pLocalTable->aryEntry[ pLocalTable->unNumEntries ].nOtherTableIndex = foreign;
				pLocalTable->unNumEntries++;			
				
				gStats.unClassNumPotAdds++;
			}
		} // add new
	
	} // for all foreign objects.
	
	gStats.unClassNumPotTotal += pLocalTable->unNumEntries;	
	
	// Now, merge all values of the foreign table to the local table.
	// For each of the (partly new) local objects, see if a foreign
	// object was found and add its values.
	for ( int local=0; local<pLocalTable->unNumEntries; local++)
	{
		// Determine reference to foreign table.
		int foreign = pLocalTable->aryEntry[local].nOtherTableIndex;
		
		// If the reference is valid....
		if ( foreign != -1 )
		{
			ClassificationTableEntry * pLocal;
			const ClassificationTableEntry * pForeign;
			
			pLocal = &( pLocalTable->aryEntry[local] );
			pForeign = &( pForeignTable->aryEntry[foreign] );
			
			/*
			DEBUG:
			dbgLog("log<< numTotalFrames local: %d, foreign: %d", pLocal->unNumTotalFrames, pForeign->unNumTotalFrames);
			dbgLog("log<< numGreenFrames local: %d, foreign: %d", pLocal->unNumFramesGreenColor, pForeign->unNumFramesGreenColor);
			*/
						
			// Just sum up the number of frames for each classification feature.
			pLocal->unNumTotalFrames 		+= pForeign->unNumTotalFrames;
			
			pLocal->unNumFramesBadColor 	+= pForeign->unNumFramesBadColor;
			pLocal->unNumFramesGreenColor 	+= pForeign->unNumFramesGreenColor;
			pLocal->unNumFramesBadForm 		+= pForeign->unNumFramesBadForm;
			pLocal->unNumFramesSplit 		+= pForeign->unNumFramesSplit;		
			
			// Calculate the mean of the sizes
			pLocal->unPotatoLength = ( pLocal->unPotatoLength + pForeign->unPotatoLength ) / 2;
			pLocal->unPotatoWidth = ( pLocal->unPotatoWidth + pForeign->unPotatoWidth ) / 2;
		}
	}	

	
#endif // ------------------------------------------------
	
}

// *************************************************************************


void CClassification::InitEjectionTables()
{
	for ( Int i=0; i<MAX_TRIGGERS_DELAY; i++ )
	{
		m_aryEjectionTables[i].bValid = FALSE;
	}
}


// *************************************************************************

CClassification::EjectionTable * CClassification::GetNewEjectionTable( Uint32 unTriggersDelay )
{
	for ( Int i=0; i<MAX_TRIGGERS_DELAY; i++ )
	{
		// We have to find an unused entry
		if ( ! (m_aryEjectionTables[i].bValid) )
		{
			// First mark this entry as valid
			m_aryEjectionTables[i].bValid 			= TRUE;
			
			// Then store the pulse number at which this entry is triggered.
			m_aryEjectionTables[i].unNumTriggers 	= m_unCurrentTriggerPulse + unTriggersDelay;
			
			// And return the reference
			return &(m_aryEjectionTables[i]);
		}
	}

	return NULL;
}

// *************************************************************************

CClassification::EjectionTable * CClassification::GetDueEjectionTable()
{
	// First drop all entries that are too old. This shouldn't be necessary and is only a precaution
	for ( Int j=0; j<MAX_TRIGGERS_DELAY; j++ )
	{
		// Only look at the valid entries
		if ( (m_aryEjectionTables[j].bValid) )
		{		
			if ( m_aryEjectionTables[j].unNumTriggers < m_unCurrentTriggerPulse )
			{

				m_aryEjectionTables[j].bValid = FALSE;
			}
		}
	}
	
	// Now search for the current entry
	for ( Int i=0; i<MAX_TRIGGERS_DELAY; i++ )
	{
		// Only look at the valid entries
		if ( (m_aryEjectionTables[i].bValid) )
		{
			// If this is it, return it
			if ( m_aryEjectionTables[i].unNumTriggers == m_unCurrentTriggerPulse )
			{
				// DEBUG:
				/*
				dbgLog( "Found due ejectiontable @ %d, index %d, ptr %p: %d%d%d%d%d%d%d%d%d%d", m_unCurrentTriggerPulse, i,  &(m_aryEjectionTables[i]),
							m_aryEjectionTables[i].efEject[0], m_aryEjectionTables[i].efEject[1],
							m_aryEjectionTables[i].efEject[2], m_aryEjectionTables[i].efEject[3],
							m_aryEjectionTables[i].efEject[4], m_aryEjectionTables[i].efEject[5],
							m_aryEjectionTables[i].efEject[6], m_aryEjectionTables[i].efEject[7],
							m_aryEjectionTables[i].efEject[8], m_aryEjectionTables[i].efEject[9] );
	*/
				return &(m_aryEjectionTables[i]);
			}
		}
	}
	return NULL;	
}

// *************************************************************************

void CClassification::ReleaseEjectionTable( CClassification::EjectionTable * pTable )
{
	pTable->bValid = FALSE;	
}

// *************************************************************************

void CClassification::ClassifyPotatoes( ClassificationTable * restrict pLocalTable, const ClassificationTable * restrict pForeignTable, EjectionTable * restrict pEjectionTable )
{	

	// First clear the ejection table
	for ( Int i=0; i<MAX_LANES; i++)
		pEjectionTable->efEject[i] = EF_NONE;
		

	// Now classify each potato
	// TODO: handle potatoes that were only visible by the other DSP
	for ( Int nLocal = 0; nLocal < pLocalTable->unNumEntries; nLocal++ )
	{
		ClassificationTableEntry * pLocal;
		pLocal = &( pLocalTable->aryEntry[nLocal] );
			
		// Now judge the potato. First generate the absolute number of picture thresholds for all
		// features, out of the percentage.
		Uint32 unNumBadColorPics 		= ( m_nBadColorPicsAllowed * pLocal->unNumTotalFrames ) >> PotatoObject::FP_FRACTIONAL_BITS;
		Uint32 unNumBadGreenColorPics 	= ( m_nBadGreenColorPicsAllowed * pLocal->unNumTotalFrames ) >> PotatoObject::FP_FRACTIONAL_BITS;
		Uint32 unNumBadFormPics 		= ( m_nBadFormPicsAllowed * pLocal->unNumTotalFrames ) >> PotatoObject::FP_FRACTIONAL_BITS;
		Uint32 unNumSplitPics 			= ( m_nSplitPicsAllowed * pLocal->unNumTotalFrames ) >> PotatoObject::FP_FRACTIONAL_BITS;
		
		// Map the potato's position to a lane. If it doesn't match, don't enter it into the efect table
		Int nLane = MapPosToJet( pLocal->nPosition_mm_X );
		if ( (nLane >= 0) && (nLane < MAX_LANES) )
		{
			// Now, finally, do the classification. The potato is marked for ejection if either one of
			// the thresholds is broken.
			// Calculate the needed ejection force depending on the potato's
			// estimated volume
			Uint32 unVolume;
			unVolume = pLocal->unPotatoWidth * pLocal->unPotatoWidth * pLocal->unPotatoLength;
			// TODO: use ellipsoid aproximation for the volume.
			EjectionForce ef = EF_MEDIUM;		
			if ( unVolume > m_nMinLargeVolume )
				ef = EF_LARGE;
			else if ( unVolume < m_nMaxSmallVolume )
				ef = EF_SMALL;
			
			if ( pLocal->unNumFramesBadColor > unNumBadColorPics )
			{
				pEjectionTable->efEject[nLane] = ef;
				m_sClassificationStats.unNumRejectedColor++;
			}
						
			if ( pLocal->unNumFramesGreenColor > unNumBadGreenColorPics )
			{
				pEjectionTable->efEject[nLane] = ef;
				m_sClassificationStats.unNumRejectedGreen++;
			}
				
			if ( pLocal->unNumFramesBadForm > unNumBadFormPics )
			{
				pEjectionTable->efEject[nLane] = ef;
				m_sClassificationStats.unNumRejectedShape++;
			}
				
			if ( pLocal->unNumFramesSplit > unNumSplitPics )
			{
				pEjectionTable->efEject[nLane] = ef;	
				m_sClassificationStats.unNumRejectedSplit++;
			}
			
			// If the potato is ejected due to any cause, store that in the statistics
			if ( pEjectionTable->efEject[nLane] != EF_NONE )
				m_sClassificationStats.unNumRejectedTotal++;
				
			// Also store that we've seen this potato at all
			m_sClassificationStats.unNumProcessed++;
				
			// DEBUG: eject every potato
			//pEjectionTable->efEject[nLane] = ef;		
			
			
			//dbgLog("Ejected potato of size: %d x %d mm -> %d", pLocal->unPotatoLength, pLocal->unPotatoWidth, ef );
		}	
	}
}

// *************************************************************************

void CClassification::GenerateEjectionCommands( EjectionTable * restrict pEjectionTable, Uint32 unCurTime )
{
#ifndef _WINDOWS // ------- Don't compile on windows -----

	JetCommand cmdDone;
	JetCommand cmdLarge;
	JetCommand cmdMedium;
	JetCommand cmdSmall;	
	
	Uint32 unCenterTime;
	Uint32 unCenterTimeMs;

	Uint32 unConvSpeed;
	
	// abort if not in classification mode.
	if ( m_eOperationMode != OP_CLASSIFICATION )
		return;	
	
	// DEBUG
	/*
	pEjectionTable.aryEntry[2] = EF_MEDIUM;
	pEjectionTable.aryEntry[7] = EF_MEDIUM;
	*/
	/*
		pEjectionTable.aryEntry[1] = EF_SMALL;
		pEjectionTable.aryEntry[2] = EF_MEDIUM;
		pEjectionTable.aryEntry[3] = EF_LARGE;
		pEjectionTable.aryEntry[4] = EF_MEDIUM;
		pEjectionTable.aryEntry[5] = EF_SMALL;
	*/
	// Get the conveyor speed and abort if it is zero
	unConvSpeed = convGetMeasuredSpeed();
	
	if ( unConvSpeed == 0 )
		return;
	
	// Calculate the timing
	unCenterTimeMs = (((1 << DRVCONV_FRACTIONAL_BITS) * m_nSpatialDelay) / unConvSpeed);
	unCenterTimeMs += m_nTemporalDelay;
	unCenterTime = unCurTime + timeFromMs( unCenterTimeMs );
	
	// Then, clear all command structures.
	for (Int i=0; i<JetCommand::MAX_JETS; i++)
	{
		cmdDone.aryJetState[i] = 0;
		cmdLarge.aryJetState[i] = 0;
		cmdMedium.aryJetState[i] = 0;
		cmdSmall.aryJetState[i] = 0;
	}
	
	// Transfer ejection map to the correct commands
	for ( Int i=0; i<MAX_LANES; i++)
		if ( pEjectionTable->efEject[i] >= EF_LARGE )
		{
			cmdLarge.aryJetState[i] = TRUE;
			cmdMedium.aryJetState[i] = TRUE;
			cmdSmall.aryJetState[i] = TRUE;
		}
		
	for ( Int i=0; i<MAX_LANES; i++)
		if ( pEjectionTable->efEject[i] >= EF_MEDIUM )
		{
			cmdMedium.aryJetState[i] = TRUE;
			cmdSmall.aryJetState[i] = TRUE;
		}
			
	for ( Int i=0; i<MAX_LANES; i++)
		if ( pEjectionTable->efEject[i] >= EF_SMALL )
			cmdSmall.aryJetState[i] = TRUE;
			
	// Now add all these commands at the right time
	cmdLarge.unCmdTime = unCenterTime 
							- timeFromMs( m_nLargeActivationLength / 2 ) 
							+ timeFromMs( m_nLargeOffsetTime );
	CJetControl::Instance()->AddCommand( &cmdLarge );
	
	cmdMedium.unCmdTime = unCenterTime 
							- timeFromMs( m_nMediumActivationLength / 2 )
							+ timeFromMs( m_nMediumOffsetTime );
	CJetControl::Instance()->AddCommand( &cmdMedium );
	
	cmdSmall.unCmdTime = unCenterTime - timeFromMs( m_nSmallActivationLength / 2 );
	CJetControl::Instance()->AddCommand( &cmdSmall );
	
	cmdMedium.unCmdTime = unCenterTime 	+ timeFromMs( m_nSmallActivationLength / 2 );
	CJetControl::Instance()->AddCommand( &cmdMedium );
	
	cmdLarge.unCmdTime = unCenterTime 
							+ timeFromMs( m_nMediumActivationLength / 2 )
							+ timeFromMs( m_nMediumOffsetTime );							
	CJetControl::Instance()->AddCommand( &cmdLarge );
	
	cmdDone.unCmdTime = unCenterTime 
							+ timeFromMs( m_nLargeActivationLength / 2 )
							+ timeFromMs( m_nLargeOffsetTime );
	CJetControl::Instance()->AddCommand( &cmdDone );

#endif // ------------------------------------------------

}

// *************************************************************************

Int CClassification::MapPosToJet( const Int nPositionX )
{
	int pos = nPositionX;
	int lane;

	// Offset the position so that lane 0 starts at a position of 0.
	// Lane 0 now starts at -m_nConveyorWidth/2...
	pos = pos + m_nConveyorWidth/2;
	lane = pos * m_nNumLanes / m_nConveyorWidth;
	
	// Limit the number of lanes and discard wrong results
	if ( (lane > m_nNumLanes-1) || (lane < 0) )
		return -1;

	// done. The ejection system's lanes are numbered from right 
	// to left (who knows why...), whereas we number them from left to
	// right...
	return m_nNumLanes - lane - 1;
}

// *************************************************************************

void CClassification::ApplyStrictness()
{
	// We'll need the central vision object
	CVisVision * vis;
	vis = CVisObjectManager::Instance()->GetMainVisionObject();
	if ( vis == NULL )
		return;
		
	// Color
	if ( m_fpColorStrictness != 0 )
		m_fpBadColorThreshold = ConvertStrictnessToValue( m_fpColorStrictness, F2FP( 0.5, PotatoObject::FP_FRACTIONAL_BITS), F2FP( 0.05, PotatoObject::FP_FRACTIONAL_BITS), 0 );
	else
		m_fpBadColorThreshold = float2fp(1, PotatoObject::FP_FRACTIONAL_BITS);
	
	// Green
	if ( m_fpGreenStrictness != 0 )
		m_fpGreenColorThreshold = ConvertStrictnessToValue( m_fpGreenStrictness, F2FP( 0.05, PotatoObject::FP_FRACTIONAL_BITS), F2FP( 0.001, PotatoObject::FP_FRACTIONAL_BITS), 0 );
	else
		m_fpGreenColorThreshold = float2fp(1, PotatoObject::FP_FRACTIONAL_BITS);
	
	// Shape
	if ( m_fpShapeStrictness != 0 )
		m_fpBadShapeThreshold = ConvertStrictnessToValue( m_fpShapeStrictness, F2FP( 200.0, PotatoObject::FP_FRACTIONAL_BITS), F2FP( 30, PotatoObject::FP_FRACTIONAL_BITS), 0 );
	else
		m_fpBadShapeThreshold = 0x7FFFFFF;
	
	// Splits
	if ( m_fpSplitStrictness != 0 )
	{
		// First calculate those values
		Int32 nMinLen = ConvertStrictnessToValue( m_fpSplitStrictness, 60, 15, 0 );
		Int32 fpMinRatio = ConvertStrictnessToValue( m_fpSplitStrictness, F2FP( 4.5, PotatoObject::FP_FRACTIONAL_BITS), F2FP( 3.0, PotatoObject::FP_FRACTIONAL_BITS), 0 );
		Int32 nMinArea = (((nMinLen * nMinLen) << PotatoObject::FP_FRACTIONAL_BITS) / fpMinRatio) / 4;
		
		// Every split is a killer for the moment.
		m_nSplitThreshold = 0;
		
		if ( vis != NULL )
		{
			vis->SetProperty( "SplitClassifier", "MinLength", (float)nMinLen );
			vis->SetProperty( "SplitClassifier", "MinRatio", fp2float( fpMinRatio, PotatoObject::FP_FRACTIONAL_BITS) );
			vis->SetProperty( "SplitClassifier", "MinArea", (float)nMinArea );
		}			
	}	
	else
		vis->SetProperty( "SplitClassifier", "MinArea", 10000 );
		
	// Adjust the visualizer values
	vis->SetProperty( "Visualizer", "BadColorIndThr", fp2float( m_fpBadColorThreshold, PotatoObject::FP_FRACTIONAL_BITS)  ); 
	vis->SetProperty( "Visualizer", "GreenColorIndThr", fp2float( m_fpGreenColorThreshold, PotatoObject::FP_FRACTIONAL_BITS) );
	vis->SetProperty( "Visualizer", "BadShapeIndThr", fp2float( m_fpBadShapeThreshold, PotatoObject::FP_FRACTIONAL_BITS) );
		
	// DEBUG:
	/*
	float f;
	LogMsg( "Color thresh: %f", fp2float( m_fpBadColorThreshold, PotatoObject::FP_FRACTIONAL_BITS ) );
	LogMsg( "Green thresh: %f", fp2float( m_fpGreenColorThreshold, PotatoObject::FP_FRACTIONAL_BITS ) );
	LogMsg( "Shape thresh: %f", fp2float( m_fpBadShapeThreshold, PotatoObject::FP_FRACTIONAL_BITS ) );
	vis->AccessProperty( FALSE, "SplitClassifier", "MinLength", f );	
	LogMsg( "Split MinLength thresh: %f", f );
	vis->AccessProperty( FALSE, "SplitClassifier", "MinRatio", f );
	LogMsg( "Split MinRatio thresh: %f", f );
	vis->AccessProperty( FALSE, "SplitClassifier", "MinArea", f );	
	LogMsg( "Split MinArea thresh: %f", f );
	*/
}

// *************************************************************************

Int32 CClassification::ConvertStrictnessToValue( Int32 fpStrictness, Int32 nLooseValue, Int32 nStrictValue, Int32 nExp )
{
	// This is the part that must be added to the loose value. 
	Int nP;
	
	// A strictness value of 0 must be mapped to the loose value, a value of 10 must be mapped
	// to the strict value.
	Int fpS = fpStrictness / 10;
	
	// Calculate the part that is to be added to the loose value. We have to consider the numeric range of those valuse
	// in order to be able to handle both fixedpoint and integer values.
	if ( max(nLooseValue, nStrictValue) > 0x00000FFF )
	{
		// This seems to be fixed point since the values are quite high. So, we have to shift both multipliers
		// by half the fixedpoint range in order to not loose accuracy.
		nP = (Int32)(( (float)nStrictValue - (float)nLooseValue) * (float)fpS / (float)(1<<PotatoObject::FP_FRACTIONAL_BITS));
		//nP = ( ((nStrictValue - nLooseValue) >> (PotatoObject::FP_FRACTIONAL_BITS/2)) * (fpS >> (PotatoObject::FP_FRACTIONAL_BITS/2)) );
	}
	else
	{
		// The small values allows us to shift but the result of the multiplication. Note: fpS is in [0..1] anyway.
		nP = ( (nStrictValue - nLooseValue) * fpS) >> PotatoObject::FP_FRACTIONAL_BITS;
	}
	
	// Sum up the result
	Int nResult = nLooseValue + nP;
	
	// LogMsg("loose: %d, strict: %d, part: %d, res: %d", nLooseValue, nStrictValue, nP, nResult );
	
	
	// TODO: apply exponential part.
	
	return nResult;
	
}

// *************************************************************************
