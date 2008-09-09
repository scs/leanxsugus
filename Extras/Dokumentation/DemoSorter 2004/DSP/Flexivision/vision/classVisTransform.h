/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISTRANSFORM_H_
#define _CLASSVISTRANSFORM_H_

#include "classVisObject.h"
#include "classVisComponent.h"
#include "classVisVector.h"


/**
* @brief An abstract transformation class.
*
* An abstract transformation class that allows the transformation of world
* point to pixel coordinates and vice-versa.
*/
//class CVisTransform : public CVisObject
class CVisTransform : public CVisComponent
{
public:
							CVisTransform( const char * strName );
	virtual 				~CVisTransform();

	/**
	* Transforms world points to pixel coordinates.
	* Must be overriden by the sub-class to add functionality.
	*/
	virtual CVisVector		TransformToPixel( const CVisVector & v ) = 0;	

	/**
	* Transforms pixel coordinates to world points.
	* Must be overriden by the sub-class to add functionality.
	*/
	virtual CVisVector		TransformToWorld( const CVisVector & v ) = 0;	
	
};


#endif

