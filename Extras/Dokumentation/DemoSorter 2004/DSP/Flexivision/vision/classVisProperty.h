/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISPROPERTY_H_
#define _CLASSVISPROPERTY_H_

class CVisProperty;

#include "classVisObject.h"
#include "classVisComponent.h"
#include "classVisFixpoint.h"
#include "../libFixedPoint.h"

/**
* @brief An image processing component's property.
*
* An image processing component's property. This property may either be of floating point
* or integer value.
*
* Properties all have a name (like all image processing objects), a type and a reference
* to the variable they're representing. That means that the variable must be allocated
* regardless of a CVisProperty object. This mechanism is chosen so that the processing algorithm
* is still able to access object members directly and doesn't have to address a property object.
*
* The property object is thus provided for access to the processing parameters from a controller's
* point of view, not from the process itself.
*/
class CVisProperty : public CVisObject
{
public:
	enum PropertyType {
		PT_INTEGER,
		PT_FLOAT,
		PT_FIXEDPOINT,
		PT_FIXEDPOINTOBJ		
	};
	
	/**
	* Constructor. 
	*/
						CVisProperty( const Char * strName );
				
	/**
	* Initializes the property and links it with the corresponding variable and component.
	*/		
	void 				Init( CVisComponent * pComp, PropertyType type, Ptr pVariable, Int nFPFactorBits = 0);
	void 				Init( CVisComponent * pComp, CVisFixpoint * fp );
						
	
	CVisComponent *		GetComponent();

	/**
	* Sets the the property's value as an integer. If this is a float property, the value is
	* just converted to an integer.
	*/
	bool				SetIntegerValue( Int32 v);
	
	/**
	* Gets the property's integer value. Again, if this is a float property, the value is
	* converted.
	*/
	bool				GetIntegerValue( Int32 & v );
	
	/**
	* Sets the property's floating point value. If this is an integer property, precision is
	* lost.
	*/
	bool				SetFloatValue( float v);
	
	/**
	* Gets the property's floating point value.
	*/
	bool				GetFloatValue( float & v );
	
	Bool				HasChanged();
	
protected:
	/** A reference to the variable. */
	Ptr					m_pVariable;
	
	/** The property's type. */
	PropertyType		m_ptType;
	
	/** The component this property is linked to. */
	CVisComponent *		m_pComponent;
	
	/** The number of factor bits when this is a fixed point number. */
	Int					m_nFactorBits;
	
	Bool				m_bHasChanged;
	
};

#endif
