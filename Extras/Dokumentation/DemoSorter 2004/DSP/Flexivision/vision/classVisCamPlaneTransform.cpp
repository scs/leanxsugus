
#include "classVisCamPlaneTransform.h"


CVisCamPlaneTransform::CVisCamPlaneTransform( const char * strName )
	:	CVisCameraTransform( strName )
{
}

// *************************************************************************

CVisVector CVisCamPlaneTransform::TransformToWorld( const CVisVector & v )
{
	return IntersectPixelRayWithPlane( v );
}

// *************************************************************************


CVisVector CVisCamPlaneTransform::IntersectPixelRayWithPlane( const CVisVector & v )
{
	CVisEqualFixpoint	s(MTRX_FRACTBITS);
	CVisVector			vecPixel(v);
	CVisVector			vecPoint;			// The point on the ray
	CVisVector			vecRayDir;			// The ray's direction
	CVisVector			vecPlaneBaseCam;	// The plane's base point minus the camera's position.
	CVisVector			vecRes;				// The result


	// First, transform the pixel to a point on the ray; don't care for the depth value, since
	// all points of that pixel lie on the same ray. So set the depth value to 1.0, in order to 
	// get a reasonable scale. The pixel coords don't have to be homogenous.
	vecPixel(3) = 1;
	vecPoint = CVisCameraTransform::TransformToWorld( vecPixel );

	// Determine the ray's direction, which is from the camera to the point we just found.
	vecRayDir.Sub( vecPoint, m_vecCamTranslation );

	// Test if the ray is perpendicular to the plane.
	if ( DotProduct( m_vecPlaneNormal, vecRayDir ) == 0 )
		return vecRes;

	// Our point lies on the ray CamTranslation + s*(Point-CamTranslation) = CamTranslation + s*RayDir. 
	// We now have to calculate the factor s.
	vecPlaneBaseCam.Sub( m_vecPlaneBase, m_vecCamTranslation );
	s.Div(	DotProduct( m_vecPlaneNormal, vecPlaneBaseCam ),
			DotProduct( m_vecPlaneNormal, vecRayDir) );

	// Now calculate intersection point
	vecRes.Mult( vecRayDir, s );
	vecRes.Add( m_vecCamTranslation );

	// Set element 4 to one, so the vector is homogenous
	vecRes(4) = 1;

	return vecRes;

}

// *************************************************************************

void CVisCamPlaneTransform::SetPlane(	float fBase_x, float fBase_y, float fBase_z,
										float fNormal_x, float fNormal_y, float fNormal_z )
{
	// Just convert the floating values to the vectors' fixedpoint values.
	m_vecPlaneBase(1) = fBase_x;
	m_vecPlaneBase(2) = fBase_y;
	m_vecPlaneBase(3) = fBase_z;

	m_vecPlaneNormal(1) = fNormal_x;
	m_vecPlaneNormal(2) = fNormal_y;
	m_vecPlaneNormal(3) = fNormal_z;
}
// *************************************************************************
