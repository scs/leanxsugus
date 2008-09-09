/**
* @file
* @author Bernhard Mäder
*/


#ifndef _CLASSVISCAMERATRANSFORM_H_
#define _CLASSVISCAMERATRANSFORM_H_


#include "classVisTransform.h"
#include "classVisVector.h"
#include "classVisMatrix.h"

/**
* @brief This class provides a transformation from world to pixel coordinates and vice-versa.
*
* This class provides a transformation from world to pixel coordinates and vice-versa.
* For this purpose, all camera parameters (like focal length, CCD size etc.) must
* be specified along with the camera's position in space (translation and rotation).
*
* Note that a pixel depth has to be specified when transforming from pixel to
* world coordinates. If no depth information is available (which usually is the case),
* use one of the more sophisticated subclasses (e.g. to get the ray's intersection 
* point with a plane).
*/
class CVisCameraTransform : public CVisTransform
{
public:
							CVisCameraTransform( const char * strName );

	/**
	* Transforms world points to pixel coordinates.
	* Implements the abstract base class' transformation function.
	*/
	virtual CVisVector		TransformToPixel( const CVisVector & v );	

	/**
	* Transforms pixel coordinates to world points.
	* Implements the abstract base class' transformation function.
	*/
	virtual CVisVector		TransformToWorld( const CVisVector & v );	


	/** Sets the lens' focal length in meters(!).*/
	void					SetFocalLength( float f_x, float f_y );

	/** Sets the CCD size, also in meters. The BottomUp parameter specifies whether the CCD's data is 
	*	stored bottom up or top-down; in the first case, the positive y-axis is pointing towards larger
	*	memory indices, in the latter case, it is pointing towards smaller indices. The latter case leads
	*	to an inversion of all pixel y values before (or after) processing.
	*/
	void					SetCCDSize( float size_x, float size_y, float pixels_x, float pixels_y, bool bBottomUp  );

	/** Sets, if required, the optical center of the camera, in pixels. The optical center is
	*   assumed to be in the center of the CCD if SetOpticalCenter() isn't called. */
	void					SetOpticalCenter( float opticalCenter_x, float opticalCenter_y);

	/** Sets the camera's translation from the coordinat system's center (in meters). */
	void					SetTranslation( float transl_x, float transl_y, float transl_z );

	/** Sets the camera's rotation in degrees. */
	void					SetRotation( float alpha_x, float alpha_y, float alpha_z );

	/** Sets the clipping of the image. This must be used if the camera's image is not
	*   directly processed but reduced in size by clipping. It is a simple offset that
	*	is added to all pixel coordinates prior to processing. */
	void					SetClipping( int offs_x, int offs_y );

protected:
	void					CalcMatrices();

	float					m_fFocalLength_x;
	float					m_fFocalLength_y;

	float					m_fSize_x;
	float					m_fSize_y;

	float					m_fPixels_x;
	float					m_fPixels_y;

	bool					m_bBottomUp;

	float					m_fPixelScale_x;
	float					m_fPixelScale_y;

	float					m_fOpticalCenter_x;
	float					m_fOpticalCenter_y;

	float					m_fAlpha_x;
	float					m_fAlpha_y;
	float					m_fAlpha_z;

	float					m_fTransl_x;
	float					m_fTransl_y;
	float					m_fTransl_z;

	int						m_nOffs_x;
	int						m_nOffs_y;

	/** The optical center offset */
	CVisFixpoint			m_fpOpticalCenter_x;	
	CVisFixpoint			m_fpOpticalCenter_y;
	
	/** Pre-calculated scale multipliers for each axis. Scale = FocalLength * PixelScale */
	CVisFixpoint			m_fpScale_x;
	CVisFixpoint			m_fpScale_y;

	/** Pre-calculated inverted scale multipliers. */
	CVisFixpoint			m_fpScale_inv_x;
	CVisFixpoint			m_fpScale_inv_y;

	/** The resulting view matrices that are generated of the translation and rotation. */	
	CVisMatrix				m_matView;
	CVisMatrix				m_matView_inv;

	CVisVector				m_vecCamTranslation;

	//  The properties...
	// -------------------
	// Camera
	CVisProperty			m_propFocalLength_x;
	CVisProperty			m_propFocalLength_y;
	CVisProperty			m_propCCDSize_x;
	CVisProperty			m_propCCDSize_y;
	CVisProperty			m_propPixels_x;
	CVisProperty			m_propPixels_y;	
	CVisProperty			m_propOpticalCenter_x;
	CVisProperty			m_propOpticalCenter_y;

	// Orientation in space
	CVisProperty			m_propTranslation_x;
	CVisProperty			m_propTranslation_y;
	CVisProperty			m_propTranslation_z;
	CVisProperty			m_propRotation_x;
	CVisProperty			m_propRotation_y;
	CVisProperty			m_propRotation_z;
	
};

#endif

