/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISBUFFERMANAGER_H_
#define _CLASSVISBUFFERMANAGER_H_

class CVisBufferManager;

#include "classVisObject.h"
#include "classVisPort.h"
#include "classVisComponent.h"

/**
* @brief The buffer manager singleton object.
*
* The buffer manager singleton object, which is responsible for allocation and
* management of the image processing components' memory buffers. 
*/
class CVisBufferManager : public CVisObject
{
private:
	/**
	* Default constructor is made private.
	*/
								CVisBufferManager();
	
public:
								~CVisBufferManager();
	
																								
	enum BufferManagerConsts {
		BUF_FAST = 1,			///< The buffer should be a fast buffer, either cached SDRAM or SRAM
		BUF_NON_CACHED = 2,		///< The buffer must not be in an L2 cached RAM area.
		BUF_LARGE = 4,			///< This is a large buffer that should be placed in external RAM.
		
		BUF_MAX_BUFFERS = 64,	///< The maximum number of buffers the manager can handle

		BUF_GUARD = 0xDEADBEEF	///< The guard word that is used to identify the end of the buffer.
	};
									
	
	/**
	* Returns a reference to the single instance of the buffer manager.
	*/
	static CVisBufferManager *	Instance();
	
	/**
	* Requests a buffer immedeately.
	*/
	bool						RequestBuffer( CVisObject * pObject, Uint32 unSize, Ptr * ppBuffer, Uint32 unFlags, Int nOverlapId = -1 );
	
	/**
	* Returns a statstics of how much memory has been allocated by the buffer manager.
	*/
	bool						GetStatistic( Uint32 & unBytesSRAM, Uint32 & unBytesSDRAM, Uint32 & unBytesSDRAMCached );

	/**
	* Checks all allocated memory buffers for integrity. The begins and ends of all buffers are checked and an error
	* is produced if any of the guard words has been overwritten.
	*/
	bool						CheckMemoryBoundaries();
	
protected:

	/**
	* Stores a memory request. This structure is needed to collect all memory requests of all times and
	* thus enables the buffer manager to do boundary checking during application execution.
	*/		
	struct RequestInfo
	{
		/** The object that requested the buffer. */
		CVisObject * 	pObject;
		
		/** The component (if any) to which the requesting objected belongs. */
		CVisComponent * 	pComponent;
		
		/** The request size. */
		Uint32			unSize;
		
		/** The segment id. */
		Uint32			unSegId;
		
		/** And a pointer to the location of the allocated memory. */
		Ptr				pBuffer;
	};
	
	
	/** The buffer manager object instance. */
	static CVisBufferManager	TheManager;
	
	/** The buffer information list. */
	RequestInfo					m_aryRequests[BUF_MAX_BUFFERS];

};


#endif
