/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISCAMPLANETRANSFORM_H_
#define _CLASSVISCAMPLANETRANSFORM_H_

#include "classVisCameraTransform.h"

/**
* @brief This kind of camera transform operates on a single plane.
*
* This kind of camera transform operates on a single plane. Points that are transformed
* from pixel to world are solved so that they lie on the given plane. The transformation
* from world to pixel stays the same.
*/
class CVisCamPlaneTransform : public CVisCameraTransform
{
public:
							CVisCamPlaneTransform( const char * strName );
	
	/**
	* Overwrite this transform function so that we can add the additional functionality.
	*/
	virtual CVisVector		TransformToWorld( const CVisVector & v );	

	CVisVector				IntersectPixelRayWithPlane( const CVisVector & v );

	void					SetPlane(	float fBase_x, float fBase_y, float fBase_z,
										float fNormal_x, float fNormal_y, float fNormal_z );


protected:
	CVisVector				m_vecPlaneBase;
	CVisVector				m_vecPlaneNormal;

};

#endif

