/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISOBJECTMANAGER_H_
#define _CLASSVISOBJECTMANAGER_H_

class CVisObjectManager;

#include "classVisObject.h"
#include "classVisComponent.h"
#include "classVisVision.h"

/**
* @brief The object manager, which registers all vision objects in the system.
*
* The object manager, which registers all vision objects in the system.
*/
class CVisObjectManager : public CVisObject
{
private:
	/**
	* The constructor is made private so that this class can only be allocated
	* by itself.
	*/
							CVisObjectManager();
					
public:					
	/**
	* Destructor. It may be thinkable that the destructor is deleting all objects
	* and thus acts as a kind of garbage collector. As of yet, this is not implemented.
	*/
							~CVisObjectManager();
							
	/**				
	* Returns a reference to the single instance of this class.
	*/
	static CVisObjectManager *	Instance();
	
	bool					AddObject( CVisObject * pObject );
	
	/**
	* Gets the next object of a specific class type. May be used to enumerate through multiple
	* objects. Use an id of -1 to start over.
	*/
	bool					GetNextObject( CVisObject ** ppObject, ClassType type, Int32 & id );

	/**
	* Searches through the list of objects and tries to find an object of a certain type.
	*/
	bool					GetObject( const Char * strName, CVisObject ** ppObject, ClassType type);
	
	bool					PreparePorts();
	bool					PrepareComponents();

	void					SetComponentsOperationMode( CVisComponent::OperationMode mode );
	
	CVisVision *			GetMainVisionObject();
	
protected:
	enum ObjectManagerConsts {
		OBJ_MAX_OBJECTS = 168
	};
	
	/** A single instance of this class. */
	static CVisObjectManager	TheManager;
	
	/** A flag that indicates whether construction of the manager is completed. */
	static bool				bConstructed;
	
	CVisObject * m_aryObjects[OBJ_MAX_OBJECTS];
	
	Int						m_nNumObjects;
};

#endif
