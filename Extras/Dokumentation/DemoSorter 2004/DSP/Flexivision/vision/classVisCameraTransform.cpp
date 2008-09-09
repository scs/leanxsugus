
#include "classVisCameraTransform.h"
#include "classVisFixpoint.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// *************************************************************************

CVisCameraTransform::CVisCameraTransform( const char * strName )
	:	CVisTransform( strName ),

		// Initialize all fixpoint numbers to the same amount of fractional bits
		// that we use in the matrices and vectors.
		m_fpOpticalCenter_x( MTRX_FRACTBITS ),
		m_fpOpticalCenter_y( MTRX_FRACTBITS ),
		m_fpScale_x( MTRX_FRACTBITS ),
		m_fpScale_y( MTRX_FRACTBITS ),
		m_fpScale_inv_x( MTRX_FRACTBITS ),
		m_fpScale_inv_y( MTRX_FRACTBITS ),

		// Construct all properties
		m_propFocalLength_x( "FocalLength_x" ),
		m_propFocalLength_y( "FocalLength_y" ),
		m_propCCDSize_x( "CCDSize_x" ),
		m_propCCDSize_y( "CCDSize_y" ),
		m_propPixels_x( "Pixels_x" ),
		m_propPixels_y( "Pixels_y" ),
		m_propOpticalCenter_x( "OpticalCenter_x" ),
		m_propOpticalCenter_y( "OpticalCenter_y" ),
		m_propTranslation_x( "Translation_x" ),
		m_propTranslation_y( "Translation_y" ),
		m_propTranslation_z( "Translation_z" ),
		m_propRotation_x( "Rotation_x" ),
		m_propRotation_y( "Rotation_y" ),
		m_propRotation_z( "Rotation_z" )
{
	// Pre-set some camera values (Don't set optical center, assume it is in the middle).
	SetFocalLength( 0.008f, 0.008f );
	SetCCDSize( 0.0089f, 0.0066f, 1380.0f, 1030.0f, false);

	// Set the camera's position to 1m above the conveyor
	SetTranslation( 0.0f, 0.0f, 1.0f );
	SetRotation( 0.0f, 0.0f, 0.0f );

	// Initialize the properties
	m_propFocalLength_x.Init( this, CVisProperty::PT_FLOAT, &m_fFocalLength_x );
	m_propFocalLength_y.Init( this, CVisProperty::PT_FLOAT, &m_fFocalLength_y );
	m_propCCDSize_x.Init( this, CVisProperty::PT_FLOAT, &m_fSize_x );
	m_propCCDSize_y.Init( this, CVisProperty::PT_FLOAT, &m_fSize_y );
	m_propPixels_x.Init( this, CVisProperty::PT_FLOAT, &m_fPixels_x );
	m_propPixels_y.Init( this, CVisProperty::PT_FLOAT, &m_fPixels_y );
	m_propOpticalCenter_x.Init( this, CVisProperty::PT_FLOAT, &m_fOpticalCenter_x );
	m_propOpticalCenter_y.Init( this, CVisProperty::PT_FLOAT, &m_fOpticalCenter_y );
	m_propTranslation_x.Init( this, CVisProperty::PT_FLOAT, &m_fTransl_x );
	m_propTranslation_y.Init( this, CVisProperty::PT_FLOAT, &m_fTransl_y );
	m_propTranslation_z.Init( this, CVisProperty::PT_FLOAT, &m_fTransl_z );
	m_propRotation_x.Init( this, CVisProperty::PT_FLOAT, &m_fAlpha_x );
	m_propRotation_y.Init( this, CVisProperty::PT_FLOAT, &m_fAlpha_y );
	m_propRotation_z.Init( this, CVisProperty::PT_FLOAT, &m_fAlpha_z );
}

// *************************************************************************

CVisVector CVisCameraTransform::TransformToPixel( const CVisVector & v )
{
	CVisVector		p_world(v);
	CVisVector		p_cam;
	CVisVector		pixel;

	// Re-calculate matrices if the properties have changed.
	if ( HavePropertiesChanged() )
		CalcMatrices();

	// make vector homogenous
	p_world(4) = 1;
	
	// Transform to camera coordinates	
	p_cam = m_matView * p_world;

	/*
	char  str[1024];
	LogMsg( "Camera Matrix:");
	m_matView.FormatMatrix( str );
	LogMsg( str );
*/
	// Pre-calculate the negative inverse of p_cam(3)
	CVisFixpoint z_inv( MTRX_FRACTBITS );
	z_inv.Div( CVisFixpoint( -1.0, MTRX_FRACTBITS), p_cam(3) );
	
/*	
	p_world.FormatMatrix( str );
	LogMsg("Before:");
	LogMsg(str);
	p_cam.FormatMatrix( str );
	LogMsg("After:");
	LogMsg( str );
*/
	// Transform to pixel coordinates
	pixel(1).Mult( p_cam(1), m_fpScale_x );
	pixel(1).Mult( z_inv );
	pixel(1).Add( m_fpOpticalCenter_x );
	
	pixel(2).Mult( p_cam(2), m_fpScale_y );
	pixel(2).Mult( z_inv );
	pixel(2).Add( m_fpOpticalCenter_y );

	pixel(3) = -p_cam(3);

/*
	pixel.FormatMatrix( str );
	LogMsg( "Pixel:" );
	LogMsg( str );
*/

	return pixel;
}

// *************************************************************************

CVisVector CVisCameraTransform::TransformToWorld( const CVisVector & v )
{
	CVisVector		p_cam;
	CVisVector		p_world;

	// Re-calculate matrices if the properties have changed.
	if ( HavePropertiesChanged() )
		CalcMatrices();

/*
	char str[1024];
	v.FormatMatrix( str );
	LogMsg("Input point:\n %s\n\n", str );
*/

	// Transform pixel back to camera coordinates.
	p_cam(1).Sub( v(1), m_fpOpticalCenter_x );
	p_cam(1).Mult( v(3) );
	p_cam(1).Mult( m_fpScale_inv_x );
	
	p_cam(2).Sub( v(2), m_fpOpticalCenter_y );
	p_cam(2).Mult( v(3) );
	p_cam(2).Mult( m_fpScale_inv_y );
	
	p_cam(3) = -v(3);
	p_cam(4) = 1;

	// Transform back
	p_world = m_matView_inv * p_cam;

/*
	p_world.FormatMatrix( str );
	LogMsg("Result point:\n %s\n\n", str );
*/
	
	return p_world;	
}

// *************************************************************************

void CVisCameraTransform::SetFocalLength( float focalLength_x, float focalLength_y )
{
	if ( focalLength_y == -1)
		m_fFocalLength_y = focalLength_x;
	else
		m_fFocalLength_y = focalLength_y;

	m_fFocalLength_x = focalLength_x;
}

// ********************************************************************************

void CVisCameraTransform::SetCCDSize( float size_x, float size_y, float pixels_x, float pixels_y, bool bBottomUp )
{
	m_fSize_x = size_x;
	m_fSize_y = size_y;

	m_fPixels_x = pixels_x;
	m_fPixels_y = pixels_y;

	m_fPixelScale_x = pixels_x / size_x / 2;
	m_fPixelScale_y = pixels_y / size_y / 2;

	m_bBottomUp = bBottomUp;

	// Set the optical center to the middle of the CCD
	SetOpticalCenter( pixels_x / 2, pixels_y / 2 );
}

// ********************************************************************************

void CVisCameraTransform::SetOpticalCenter( float opticalCenter_x, float opticalCenter_y)
{
	m_fOpticalCenter_x = opticalCenter_x;
	m_fOpticalCenter_y = opticalCenter_y;
}

// ********************************************************************************

void CVisCameraTransform::SetRotation( float alpha_x, float alpha_y, float alpha_z )
{
	m_fAlpha_x = alpha_x;
	m_fAlpha_y = alpha_y;
	m_fAlpha_z = alpha_z;

	CalcMatrices();
}

// ********************************************************************************

void CVisCameraTransform::SetTranslation( float transl_x, float transl_y, float transl_z )
{
	m_fTransl_x = transl_x;
	m_fTransl_y = transl_y;
	m_fTransl_z = transl_z;

	CalcMatrices();
}

// *************************************************************************

void CVisCameraTransform::SetClipping( int offs_x, int offs_y )
{
	m_nOffs_x = offs_x;
	m_nOffs_y = offs_y;
}

// *************************************************************************

void CVisCameraTransform::CalcMatrices()
{
	CVisMatrix Rx, Ry, Rz;
	CVisMatrix T;

	// Convert angles from degrees to radians.
	float a_x, a_y, a_z;
	a_x = (m_fAlpha_x / 360) * 2 * (float)M_PI;
	a_y = (m_fAlpha_y / 360) * 2 * (float)M_PI;
	a_z = (m_fAlpha_z / 360) * 2 * (float)M_PI;

	// Convert the optical center values to fp. Take the clipping offset into account!
	m_fpOpticalCenter_x = m_fOpticalCenter_x - (float)m_nOffs_x;
	m_fpOpticalCenter_y = m_fOpticalCenter_y - (float)m_nOffs_y;

	// Pre-calculate the axis scales
	m_fpScale_x = (m_fPixelScale_x*m_fFocalLength_x);
	m_fpScale_y = (m_fPixelScale_y*m_fFocalLength_y);
	m_fpScale_inv_x = (1/(m_fPixelScale_x*m_fFocalLength_x));
	m_fpScale_inv_y = (1/(m_fPixelScale_y*m_fFocalLength_y));

	// If the storage is not bottom-up, invert the y-axis scales.
	if ( ! m_bBottomUp )
	{
		m_fpScale_y = -m_fpScale_y;
		m_fpScale_inv_y = -m_fpScale_inv_y;
	}

	// Fill the camera's translation vector
	m_vecCamTranslation(1) = m_fTransl_x;
	m_vecCamTranslation(2) = m_fTransl_y;
	m_vecCamTranslation(3) = m_fTransl_z;
	m_vecCamTranslation(4) = 0;

	// ---------------------------------------
	//  Forward view matrix (world->cam)
	// ---------------------------------------
	CVisFixpoint fpZero(MTRX_FRACTBITS );
	CVisFixpoint fpOne(MTRX_FRACTBITS );
	fpZero = 0;
	fpOne = 1;
	
	// Assemble the view matrix. Note that all angles and translations have to 
	// be applied negatively, since we want to move the camera, not the model.
	// We have to apply the translation first, so that the rotation is only affecting
	// the camera, not the model. 	
	Rx.MakeRotation( fpOne, fpZero, fpZero, CVisFixpoint(-a_x, MTRX_FRACTBITS) );
	Ry.MakeRotation( fpZero, fpOne, fpZero, CVisFixpoint(-a_y, MTRX_FRACTBITS) );
	Rz.MakeRotation( fpZero, fpZero, fpOne, CVisFixpoint(-a_z, MTRX_FRACTBITS) );

	// Create the translation vector, which again equals the negative of the camera translation and
	// add it to the view matrix. Note: we can't directly enter those values to the rotation matrix!
	T.MakeIdentity();
	T(1,4) = -m_fTransl_x;
	T(2,4) = -m_fTransl_y;
	T(3,4) = -m_fTransl_z;
	T(4,4) = 1;

	// Now all rotations and the translation in the correct order!
	m_matView = Rz * Ry * Rx * T;

	// ---------------------------------------
	//  Inverse view matrix (cam->world)
	// ---------------------------------------
	// Construct the inverse view matrix, to which we'll apply all values positive now and in the
	// reverse order.
	Rx.MakeRotation( fpOne, fpZero, fpZero, CVisFixpoint(a_x, MTRX_FRACTBITS) );
	Ry.MakeRotation( fpZero, fpOne, fpZero, CVisFixpoint(a_y, MTRX_FRACTBITS) );
	Rz.MakeRotation( fpZero, fpZero, fpOne, CVisFixpoint(a_z, MTRX_FRACTBITS) );
	T.MakeIdentity();
	T(1,4) = m_fTransl_x;
	T(2,4) = m_fTransl_y;
	T(3,4) = m_fTransl_z;
	T(4,4) = 1;
	m_matView_inv =  T * Rx * Ry * Rz;

}

// *************************************************************************




