/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSCLASSIFICATION_H_
#define _CLASSCLASSIFICATION_H_

#ifndef _WINDOWS // ------- Don't compile on windows -----

#include "FlexiVisioncfg.h"

#include "drvPPUSerial.h"

#include "libBufferQueue.h"
#include "classProfiler.h"

#else // ------- Do some hacks for windows --------------

// Hack the ser handle
#define PPUSER_DevHandle	Int
#define SEM_Obj				Int

#endif // ------------------------------------------------

#include "vision/classVisComponent.h"
#include "vision/classVisTracker.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* This is the entry point for the DSP/BIOS' TSK module. The function will only call
* the CClassification's EnterTaks() function, which is possible since the CClassification
* object only exists once (i.e. is a singleton).
*/
void 		TSK_Classification_funct( void );

/**
* The statistics that may be accessed by the GUI to get the number of potatoes processed.
*/
typedef struct ClassificationStats
{
	Uint32			unElapsedTime;
	Uint32			unNumProcessed;
	Uint32			unNumPossible;
	Uint32			fp16ConveyorSpeed;
	Uint32			unNumRejectedTotal;
	Uint32			unNumRejectedSplit;
	Uint32			unNumRejectedColor;
	Uint32			unNumRejectedGreen;
	Uint32			unNumRejectedShape;
} ClassificationStats;

/**
* The service mode available for the jets. 
*/
enum ClassifierServiceMode
{
	CSM_NORMAL,
	CSM_CHECK_JETS,
	CSM_ADJUST_SMALL_EJECTION_PARAMS,
	CSM_ADJUST_MEDIUM_EJECTION_PARAMS,
	CSM_ADJUST_LARGE_EJECTION_PARAMS
};

#ifdef __cplusplus
}
#endif

/**
* @brief The classification task class.
*
* The classification task class, which is implemented as a singleton object. The classification
* task is one of the most critical tasks in the system and thus runs with almost highest priority
* (only surpassed by the jet control stuff, which is implemented as SWI). It is driven by the
* external hardware trigger, that fires each time a conveyor roll has passed the sensor. For each
* trigger, the task has to decide which potatoes to eject; that includes determining from their position
* which potatoes have to be considered, and deceiding whether a single potato is good or bad.
* The result of this decision process is stored in tables that lead to the sending of ejection commands
* to the jet controller.
*
* The classification task is also very important for the image processing task, since it is the only
* instance that is allowed to remove potatoes from the global potato object list. So, the tracker on the
* one hand, which is adding objects to this list, and the classifier on the other hand, which is removing
* them, must play closely together.
*
* A final task of this object is to receive strictness settings from the GUI and propagate them through
* the image processing network, if needed (e.g. the split strictness affects several properties in the network.
*/
class CClassification : public CVisComponent
{
	
public:
	/**
	* Constructor. Note: the constructor will be called before the DSP/BIOS is up
	* and running. Keep in mind that no call to MEM_* functions may be done there.
	*/
								CClassification();	
	
	/**
	* Enters the classification task loop and never returns. This serves as an entry
	* point for the TSK_Classification_funct() function and thus for the DSP/BIOS.
	*/
	void						EnterTask();
	
	/**
	* Signals the classifier task that there has been a hardware trigger. This information
	* is used by the classifier to synchronize to the real hardware and to create the
	* ejection commands at the right time.
	*
	* This function is supposed to be called by an SWI or from any other context and is thus
	* thread-safe.
	*/
	void						SignalTrigger();
	
	/**
	* Sets a reference to the potato table and thereby enables the classification process.
	*/
	void						SetPotatoTable( PotatoList * pTable );
	
	/**
	* Gets the statistics of the classifier task. A classificationStats object must be provided
	* to which the current statistics are copied.
	*
	* The statistics may also be reset by setting the bReset flag.
	*
	*/
	void						GetStats( ClassificationStats * stats, Bool bReset );
	
	void						SetServiceMode( ClassifierServiceMode csmMode, Int nParam );
	
	void						ServiceGenJetCheckCommands( );
	void						ServiceGenParamAdjustCommands( Uint32 unTriggerTime );

	/**
	* Applies all strictnesses to the vision's properties.
	*/
	void 						ApplyStrictness();
	
	/**
	* Returns the single classification object in the system.
	*/
	static CClassification *	Instance();
	
	
	enum ClassificationConsts{
		MAX_TABLE_ENTRIES = 16, //16
		MAX_LANES = 16,
		MAX_TRIGGERS_DELAY = 10,
		MAXWAIT_SECONDS = 2,
		EXCHANGE_WAIT_MS = 30,
		MAGIC_NUMBER1 = 0xDEADBEEF,
		MAGIC_NUMBER2 = 0xAFFE1234,
		
				
		INTERLINK_UART_CHAN = 0
	};
	
	/**
	* The structure that makes up an entry of the classification table.
	*/
	typedef struct 
	{
		/** 
		* The current (i.e. last seen) X position of the potato. This information is needed by
		* the other DSP to match both views of a single potato. The position is in millimeters of
		* the world coordinate system.
		*/
		Int16	nPosition_mm_X;
		
		/** The number of views that were used to classify this potato. */
		Uint8	unNumTotalFrames;
		
		/** These are the number of frames that were classified as bad. */
		Uint8	unNumFramesBadColor;
		Uint8	unNumFramesGreenColor;
		Uint8	unNumFramesBadForm;
		Uint8	unNumFramesSplit;
		
		/** The index of the same potato in the other table. */
		Int8	nOtherTableIndex;
		
		/** The potato's width and length. */
		Uint16	unPotatoLength;
		Uint16	unPotatoWidth;
		
	} ClassificationTableEntry;
	
	/**
	* The classification table is used to communicate with the other DSP. Two magic number
	* fields are provided to secure the start and the end of the table (at which most of
	* the transmission errors occur!). In addition, a dummy word is added to the start of
	* the table, in the hope of evading some of the transmission errors that occur in the first
	* byte or so (still, nobody's got a clue why...).
	*/
	typedef struct 
	{
		Uint32						unDummy;
		
		Uint32						unMagicNumber1;
		Uint32						unNumEntries;
		ClassificationTableEntry	aryEntry[MAX_TABLE_ENTRIES];
		Uint32						unMagicNumber2;
	} ClassificationTable;
	
	enum EjectionForce
	{
		EF_NONE,
		EF_SMALL,
		EF_MEDIUM,
		EF_LARGE
	};
	
	/**
	* This is the resulting table where the ejection forces for each lane are stored. This is the result
	* of the classifier, which operates on the merged classification lists. Also, the potatoes' position
	* is mapped to a corresponding lane when being entered in the ejection table.
	*/
	typedef struct 
	{
		Bool			bValid;
		Uint32			unNumTriggers;
		
		EjectionForce	efEject[MAX_LANES];
		
	} EjectionTable;		
	
		
protected:

	/**
	* Clears the potato objects table of objects that are marked for deletion by the image processing task.
	* This function should not be called in a steady operation state but is useful, if the conveyor is
	* standing still, to remove unnecessary objects from the view.
	*/
	void 						CleanObjects( PotatoList * restrict pPotatoTable );
	
	/**
	* Builds the classification table out of the potato objects list. This is the first function that is called
	* whenever a trigger signal is received. The resulting table must then be transmitted to the other DSP board,
	* where it is merged.
	*/
	void 						BuildTable( ClassificationTable * restrict pTable, PotatoList * restrict pObjects );
	
	/**
	* Exchanges the classification tables between the two DSP platforms. Since the trigger is synchronous to both
	* DSPs, this function will also be called at the same time. So, both DSPs will send their classification table
	* and wait for one to received at the same time.
	*/
	Bool 						ExchangeTables( ClassificationTable * restrict pLocalTable, ClassificationTable * restrict pForeignTable );
	
	void 						MergeTables( ClassificationTable * restrict pLocalTable, const ClassificationTable * restrict pForeignTable );
	
	/**
	* Initializes the ejection tables buffer.
	*/
	void 						InitEjectionTables();
	
	/**
	* Gets a new ejection table given a number of trigger pulses to delay (measured from the current 
	* time). The table must later be released by ReleaseEjectionTable.
	*
	* The function may return NULL if no table was found.
	*/
	EjectionTable * 			GetNewEjectionTable( Uint32 unTriggersDelay );
	
	/**
	* Gets the ejection table that is due currently. I.e. the table wich should be used for generating
	* the current trigger pulse's ejection commands.
	*
	* The function may return NULL if no table was found.
	*/
	EjectionTable * 			GetDueEjectionTable();
	
	/**
	* Releases an ejection table after use.
	*/
	void 						ReleaseEjectionTable( EjectionTable * pTable );

	
	/**
	* Classifies all potatoes found in either the local or the foreign table. The tables must already be
	* merged for this. The result is stored in the ejection table.
	*/
	void 						ClassifyPotatoes( ClassificationTable * restrict pLocalTable, const ClassificationTable * restrict pForeignTable, EjectionTable * restrict pEjectionTable );
	
	/**
	* Generates ejection commands out of a given ejection table.
	*/
	void 						GenerateEjectionCommands( EjectionTable * restrict pEjectionTable, Uint32 unCurTime );
	
	
	/**
	* Maps a potato's position to a certain lane.
	*/
	Int							MapPosToJet( const Int nPositionX );
	
	/**
	* Helper function to convert a value of range [0..1] to an arbitrary range.
	*/
	Int32 						ConvertStrictnessToValue( Int32 fpStrictness, Int32 nLooseValue, Int32 nStrictValue, Int32 nExp = 0 );
	
	/** We need the ppuSerial device driver to transmit to the other DSP plattform. */
	PPUSER_DevHandle			m_hPPUSerial;
	
	/** 
	* Allocate two instance for the classification tables, one for the local classification,
	* one for the other DSP board's classification.
	*/
	ClassificationTable			m_LocalClassificationTable;
	ClassificationTable			m_ForeignClassificationTable;
	
	/** 
	* The ejection table list.
	*/
	EjectionTable 				m_aryEjectionTables[MAX_TRIGGERS_DELAY];
	
	Uint32						m_unCurrentTriggerPulse;

	
	static CClassification		* g_pClassification;
	
	PotatoList *				m_pPotatoTable;
	
	ClassificationStats			m_sClassificationStats;
	
	ClassifierServiceMode		m_csmServiceMode;
	Int32						m_nServiceParam;
		
	Uint32						m_unNumTableEntries;
	
	SEM_Obj						m_semTrigger;
	Bool						m_bTrigger;
	
	/** The watchdog ID of this task. */
	Uint32						m_unWatchId;
	
	/** 
	* The property that defines the maximum search distance when merging foreign and
	* local table objects.
	*/
	Int32						m_nMergeSearchDistance;
	CVisProperty				m_propMergeSearchDistance;

	/**
	* These are the different thresholds for judging a single frame's features.
	*/
	Int32						m_fpBadColorThreshold;	
	Int32						m_fpGreenColorThreshold;	
	Int32						m_fpBadShapeThreshold;	
	Int32						m_nSplitThreshold;
	
	/**
	* These are the externally available strictness definitions. A change in one
	* of these will affect some of the other properties throughout the vision
	* network.
	*/
	Int32						m_fpSplitStrictness;
	CVisProperty				m_propSplitStrictness;
	
	Int32						m_fpShapeStrictness;
	CVisProperty				m_propShapeStrictness;
	
	Int32						m_fpGreenStrictness;
	CVisProperty				m_propGreenStrictness;
	
	Int32						m_fpColorStrictness;
	CVisProperty				m_propColorStrictness;	
	
	/**
	* These are percentage thresholds of how many frames are allowed to be classified
	* as bad for each category.
	*/
	Int32						m_nBadColorPicsAllowed;
	Int32						m_nBadGreenColorPicsAllowed;
	Int32						m_nBadFormPicsAllowed;
	Int32						m_nSplitPicsAllowed;
	
	/**
	* The number of trigger pulses delay defines how many triggers the ejection commands should
	* be delayed. This time is measured from when the potatoes reach the drop zone until when
	* they should be ejected. This is the raw delay adjustment, use the SpatialDelay property for
	* fine tuning.
	*/
	Int32						m_nNumTriggerPulsesDelay;
	CVisProperty				m_propNumTriggerPulsesDelay;
	
	/**
	* The spatial delay adjusts the generation of ejection commands. It will create a delay
	* that depends on the conveyor's speed. The delay is given in millimeters.
	*/
	Int32						m_nSpatialDelay;
	CVisProperty				m_propSpatialDelay;
	
	/**
	* The temporal delay creates a delay on the creation of the ejection commands, that is not
	* dependant on the conveyor's speed. The delay is given in milliseconds.
	*/
	Int32						m_nTemporalDelay;
	CVisProperty				m_propTemporalDelay;
	
	/**
	* This value defines how long the jets will be activated for a single ejection. In milliseconds.
	*/
	Int32						m_nSmallActivationLength;
	CVisProperty				m_propSmallActivationLength;
	
	Int32						m_nMediumActivationLength;
	CVisProperty				m_propMediumActivationLength;
	
	Int32						m_nLargeActivationLength;
	CVisProperty				m_propLargeActivationLength;
	
	Int32						m_nMediumOffsetTime;
	CVisProperty				m_propMediumOffsetTime;
	
	Int32						m_nLargeOffsetTime;
	CVisProperty				m_propLargeOffsetTime;
		
	
	Int32						m_nMaxSmallVolume;
	CVisProperty				m_propMaxSmallVolume;
	
	Int32						m_nMinLargeVolume;
	CVisProperty				m_propMinLargeVolume;
	
	Int32						m_nNumLanes;
	Int32						m_nConveyorWidth;
	
	
};




#endif
