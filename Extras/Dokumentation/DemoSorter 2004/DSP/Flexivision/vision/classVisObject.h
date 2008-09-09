/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISOBJECT_H_
#define _CLASSVISOBJECT_H_

// **************************************
//  Following are a few platform abstrac-
//  tion thingies, mostly for memory
//  copying and processor intrinsics.
// **************************************

#ifdef _WINDOWS

// This is only compiled on windows environments
//#include <string.h>
#include <afxtempl.h>
#include "../VisionTest/stdafx.h"			// mfc stuff
#include "../VisionTest/Types.h"			// Fake DSP/Bios types
#include "../VisionTest/C64Intrinsics.h"	// Fake DSP Intrinsics

// Define those otherwise in libEDMAManager.h defined values.
#define EDMACOPY_DONE	0xFFFFFFFF
#define EDMACOPY_1D1D	0x1          
#define EDMACOPY_2D1D	0x2          
#define EDMACOPY_2D2D	0x3          
#define EDMACOPY_1D2D   0x4

#define _nassert(x) ASSERT(x)

#else

// This is only compiled on DSP environments.
#include "../FlexiVisioncfg.h"
#include "../libEDMAManager.h"
#include "../libDebug.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(a) ( ((a)<0) ? (-(a)) : (a) )
#endif

#define ASSERT(x)

#define TRACE dbgLog

#endif


// **************************************
//  Done.
// **************************************

#define F2FP(x, base) ((Int32)( (float)(x) * (float)(1<<base) ) )
#define FP2F(x, base) ((float)(x) / (float)(1<<base))

/**
* @brief The base class for all objects in the image processing chain.
*
* This is the base class for all image processing related objects. It implements
* a few basic functions that should be available to all those objects. Furthermore
* it implements a name identifier by which all derived objects can be located.
*/
class CVisObject
{
protected:
	enum ImageProcessingObjectConsts {	
		OBJ_NAME_MAX_CHARS = 20,
		OBJ_TYPE_MAX_CHARS = 20,

		COPY_DONE = EDMACOPY_DONE,
		COPY_1D1D = EDMACOPY_1D1D,
		COPY_2D1D = EDMACOPY_2D1D,
		COPY_2D2D = EDMACOPY_2D2D,
		COPY_1D2D = EDMACOPY_1D2D
		};
public:	
	/** 
	* Defines the known direct subclass types of the vision object.
	* This is used by the object manager to identify different object
	* types.
	*/
	enum ClassType {
		CT_VISION,
		CT_BUFFER_MANAGER,
		CT_OBJECT_MANAGER,
		CT_COMPONENT,
		CT_PORT,
		CT_PROPERTY,
		
		CT_MATH,
		
		CT_NUM_TYPES
	};
	
	/** Translates the class types to human readable strings. */
	static const char * csClassTypesString[CT_NUM_TYPES];
		
protected:
	/**
	* Constructor. A name and a class type must be specified. 
	*/
						CVisObject( const char * strName, const ClassType type );

	/**
	* A considerably faster constructor that should only be used for the math object types.
	*/
	//inline				CVisObject( const ClassType type )								{ m_strName[0] = 0;m_ctClassType = type; }
	inline				CVisObject( const ClassType type )								{ m_strName = 0; m_ctClassType = type; }
	
public:
	/**
	* Sets the object's name.
	*/			
	void				SetName( const Char * strName );
	
	/**
	* Gets the object's name.
	*/
	const Char *		GetName() const;
	
	/**
	* Compare this object's name to a given string.
	*/
	bool				HasName( const Char * strName ) const;

	/**
	* Gets this object's class type.
	*/	
	ClassType			GetClassType() const;
	
	/**
	* Gets this object's class type as a string.
	*/
	const Char *		GetStrClassType() const;
					
protected:	
	/**
	* A helper function that copies a string, but stopps when a given number of characters
	* is transferred. This function must be used with the image processing framework objects,
	* since all of those strings are strictly limited in length.
	*/
	void 				StringCopy( const Char * src, Char * dest, Uint32 maxChars) const;
	
	/**
	* Compares two strings for equality. Like with StringCopy(), there may also be specified 
	* the maximum number of characters in order to not harm any string boundaries.
	*/
	bool				StringCompare( const Char * src, const Char * dest, Uint32 maxChars) const;
	
	/**
	* Clears an array list.
	*/
	void				ClearList( CVisObject ** ppList, Uint32 unNumEntries ) const;
	
	/**
	* Adds an image processing object to an array of objects. This function  does not allocate
	* any memory for the list itself but may be used by sub-classes to easier handle lists.
	*/
	bool				AddObjectToList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject * pObject ) const;
	
	/**
	* Searches through a given list and tries to find a certain object by its name. There is
	* no list allocated by default, this function only provides functionality for derived classes.
	*/
	bool 				GetObjectFromList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject ** pObject, const char * strName ) const;
	
	/**
	* Used to iterate through a given list. An Id of -1 will restart the iteration process. A return
	* value of false indicates that no more objects are available.
	*/
	bool				GetNextObjectFromList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject ** pObject, Int & nID ) const;
	
	void 				MemSet( Ptr pDest, Uint8 unValue, Uint32 unSize ) const;
	
	void				QuickCopy(  Ptr dest, Ptr source, Uint32 size );
	
	/**
	* Starts a DMA copying process, returning an ID. The ID can then be used to wait for
	* the completion of that process.
	*/
	Uint32 				StartCopy( Ptr dest, Ptr source, Uint32 size );

	/**
	* Starts a 2D DMA copying process, returning an ID. Like in Startcopy(), this Id can later be used
	* to wait for the transfer to finish.
	*
	* 2 dimensional copying allows to copy certain rectangular regions out of an image and assemble them
	* to a coherent image at the target. Copying in the other direction (to an image region) is also possible,
	* as well as copying from region to region. The type specifier defines what type of copy is used; it can
	* be COPY_1D2D, COPY_2D1D or COPY_2D2D. The number of bytes actually copied is lineCnt * lineLen in each
	* case.
	*/
	Uint32				StartCopy2D( Uint32 type, Ptr dst, Ptr src, Uint16 lineLen, Uint16 lineCnt, Uint16 linePitch );

	/**
	* Waits for the completion of a copy process.
	*/
	void 				WaitCopy( Uint32 unCopyId ) ;
	
	void				WritebackCache( const Ptr p, const Uint32 unSize ) const;
	void				InvalidateCache( const Ptr p, const Uint32 unSize ) const;	
	
				
	/**
	* Logs a message to whatever logging capabilites are available. Also, the name and
	* classtype of the object which is causing the log are written.
	*/
	void 				LogMsg(const char* strFormat, ...) const;
	
	/** 
	* Creates a new profiler task.
	*/
	Int 				NewProfileTask( const Char * strTaskName ) const;
	
	/**
	* Starts the measurement of a certain profiler task.
	*/
	void				StartProfileTask( Int tasknum ) const;
	
	/**
	* Stops the measurement of a profiler task.
	*/
	void				StopProfileTask( Int tasknum ) const;
	
	/** The object's name. */
	//Char				m_strName[OBJ_NAME_MAX_CHARS];
	const Char *		m_strName;
	
	/** The object's class type */
	ClassType			m_ctClassType;

#ifdef _WINDOWS	
	/** Defines a copy request */
	struct CopyRequest
	{
		Uint32		unType;
		Ptr			pSource;
		Ptr			pDest;
		Uint32		unSize;
		Uint32		unLineLen;
		Uint32		unLineCount;
		Uint32		unLinePitch;		
	};

	/** Does the real copying. This is only used on Windows. */
	void				DoCopy( CopyRequest * req );

	/** 
	* This implements the list for the EDMA copy emulation on windows environment.
	* Since we can't use DMAs on the PC, we try to emulate their "simultaneousness"
	* by randomly executing the copy either at the StartCopy() or WaitCopy() command.
	* The purpose of this is to test wether the Start/Wait copy commands are applied
	* at the right place. 
	*/
	CList<CopyRequest*, CopyRequest*&>	m_lCopyRequest;
#endif

#ifdef _WRITETOFILE
	void			WriteIntValuesToFile(	Int16 * mValues, 
											Uint32 numValues, 
											bool bComplex, 
											CString strFile, 
											CString strVar, 
											bool bFirst );

	void			WriteLongValuesToFile(	Int32 * mValues, 
											Uint32 numValues, 
											bool bComplex, 
											CString strFile, 
											CString strVar, 
											bool bFirst );

	void			WriteFloatValuesToFile(	float * mValues, 
											Uint32 numValues, 
											bool bComplex, 
											CString strFile, 
											CString strVar, 
											bool bFirst );
#endif

};

#endif
