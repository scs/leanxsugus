
#include "classVision.h"

// *************************************************************************

#define GRABBER_WIDTH	1380
#define GRABBER_HEIGHT_CCD 1030
#define GRABBER_DOWNSIZE_Y	((float)GRABBER_HEIGHT_CCD/(float)GRABBER_HEIGHT)
#define GRABBER_HEIGHT	256
#define CLIPPED_WIDTH	500
#define CLIPPED_HEIGHT	GRABBER_HEIGHT
#define CLIPP_OFFS_X	( ((GRABBER_WIDTH)-(CLIPPED_WIDTH))/2 )
#define CLIPP_OFFS_Y	( ((GRABBER_HEIGHT)-(CLIPPED_HEIGHT))/2 )


#define VISUALIZE_SCALE	1


CVision::CVision()
	: 	CVisVision("Vision")
	
		// *************************************
		//  Initialize all components
		// *************************************	

		// Framegrabber and clippers
		,m_compFrameGrabber( "Grabber", GRABBER_WIDTH, GRABBER_HEIGHT, CVisPort::PDT_32BPP_RGB)
		,m_compClip( "Clipper", CVisPort::PDT_32BPP_RGB, CLIPP_OFFS_X, CLIPP_OFFS_Y, CLIPPED_WIDTH, CLIPPED_HEIGHT )
		,m_compRGBToGray( "RGBToGray" )
		,m_compThreshold( "Threshold", CVisThreshold::TT_CLAMP_TO_ZERO )
		,m_compMorphology( "Morphology", CVisPort::PDT_8BPP_GRAY, CVisMorphology::MORPH_CLOSE )
		,m_compKeypointsCut( "KeypointsCut", 1024, CVisKeypointsCut::OUTPUT_ON_INPUTPORT )
		,m_compLabel( "Label", 64, 0, 1024, CVisFastLabel::LABEL_NEIGHBORHOOD_8 | CVisFastLabel::LABEL_TRANSFORM_COORDS )
		,m_compColorPick ( "ColorPick", 1024 )
		,m_compDemoSorterClassifier( "Classifier" )
		,m_compLabelVisualizer( "LabelVisualizer", CLIPPED_WIDTH / VISUALIZE_SCALE, CLIPPED_HEIGHT / VISUALIZE_SCALE, VISUALIZE_SCALE )
		,m_compColorVisualizer( "ColorVisualizer", VISUALIZE_SCALE, CVisColorPickVisualizer::OUTPUT_ON_INPUTPORT )
		,m_compModelVisualizer( "ModelVisualizer", VISUALIZE_SCALE, CVisModelVisualizer::OUTPUT_ON_INPUTPORT )
		,m_compModel( "Model" )
		,m_compCoordTransform( "Transform" )
{

	m_vmMode = VM_IDLE;

	// *************************************
	//  Connect the network
	// *************************************
	// The network is configured in idle mode and must be reconfigured when
	// service, processing or calibration mode is required.
	m_compClip.Connect( "input", &m_compFrameGrabber, "output" );
	m_compRGBToGray.Connect( "input", &m_compClip, "output" );
	m_compThreshold.Connect( "input", &m_compRGBToGray, "output" );
	m_compMorphology.Connect( "input", &m_compThreshold, "output" );
	m_compKeypointsCut.Connect( "input", &m_compMorphology, "output" );
	//m_compLabel.Connect( "input", &m_compKeypointsCut, "output" );
	m_compLabel.Connect( "input", &m_compThreshold, "output" );
	m_compColorPick.Connect( "rgbInput", &m_compClip, "output" );
	m_compColorPick.Connect( "objLabels", &m_compLabel, "objLabels" );
	m_compDemoSorterClassifier.Connect( "objLabels", &m_compLabel, "objLabels" );
	m_compDemoSorterClassifier.Connect( "objColors", &m_compColorPick, "objColors" );
	
	m_compLabelVisualizer.Connect( "rgbInput", &m_compClip, "output" );
	m_compLabelVisualizer.Connect( "objLabels", &m_compLabel, "objLabels" );		
	m_compColorVisualizer.Connect( "rgbInput", &m_compLabelVisualizer, "output" );
	m_compColorVisualizer.Connect( "objLabels", &m_compLabel, "objLabels" );
	m_compColorVisualizer.Connect( "objColors", &m_compColorPick, "objColors" );

	m_compModelVisualizer.Connect( "rgbInput", &m_compClip, "output" );

	// *************************************
	// Prepare the network
	// *************************************
	if ( ! Prepare() )
		LogMsg("Preparation failed!");
		
	m_compRGBToGray.Prepare();
	m_compMorphology.Prepare();
	m_compLabel.Prepare();
	
	// *************************************
	//  Pre-set the properties.
	// *************************************
	SetProperty( "Threshold", "Threshold", 25.0f );
	SetProperty( "Morphology", "numIterations", 2.0f );
	SetProperty( "Label", "MinObjectArea", 900.0f );
	SetProperty( "Label", "MinDistToBorder", -1.0f );
	SetProperty( "ColorPick", "WindowSize", 31.0f );

	SetProperty( "Classifier", "TempDelay_ms", 0.0f );
	SetProperty( "Classifier", "Margin_ms", 15.0f );
	SetProperty( "Classifier", "SideMargin", 0.002f );
	SetProperty( "Classifier", "CriticalDistance_2", 0.017f*0.017f );
	
	// *************************************
	//  Setup the colors
	// *************************************
	// These are for sugus
	SetProperty( "Classifier", "Color0.Hue", 45.0f );		// Yellow
	SetProperty( "Classifier", "Color1.Hue", 15.0f );		// Orange
	SetProperty( "Classifier", "Color2.Hue", 9.0f );		// Red
	SetProperty( "Classifier", "Color3.Hue", 86.0f );		// Green
	SetProperty( "Classifier", "Color4.Hue", 170.0f );		// None

/*
	// These are for Feuersteine
	SetProperty( "Classifier", "Color0.Hue", 33.0f );		// Yellow
	SetProperty( "Classifier", "Color1.Hue", 8.0f );		// Orange
	SetProperty( "Classifier", "Color2.Hue", 2.0f );		// Red
	SetProperty( "Classifier", "Color3.Hue", 99.0f );		// Green
	SetProperty( "Classifier", "Color4.Hue", 152.0f );		// Blue
*/
	
	SetProperty( "Classifier", "Color0.Eject", 0.0f );
	SetProperty( "Classifier", "Color1.Eject", 0.0f );
	SetProperty( "Classifier", "Color2.Eject", 0.0f );
	SetProperty( "Classifier", "Color3.Eject", 0.0f );
	SetProperty( "Classifier", "Color4.Eject", 0.0f );

	// *************************************
	//  Setup the model
	// *************************************
	SetProperty( "Model", "Width", 0.24f );
	SetProperty( "Model", "Height", 0.13f );
	SetProperty( "Model", "JetsSpacing", 0.014f );
	SetProperty( "Model", "ObjectsLength", 0.055f );	
	SetProperty( "Model", "ObjectsWidth", 0.01f );	
	SetProperty( "Model", "ConveyorPosition", 0.16f );	
	SetProperty( "Model", "EjectionPosition_Left", -0.135f );	
	SetProperty( "Model", "EjectionPosition_Right", -0.148f );
	SetProperty( "Model", "Margin_ms", 10.0f );
	SetProperty( "Model", "SideMargin", 0.005f );
	
	// *************************************
	//  Setup the transform
	// *************************************
	m_compCoordTransform.SetClipping( CLIPP_OFFS_X, CLIPP_OFFS_Y );
	m_compCoordTransform.SetFocalLength( 0.006f, 0.006f );
	m_compCoordTransform.SetCCDSize( 0.0089f, 0.0066f / GRABBER_DOWNSIZE_Y, (float)GRABBER_WIDTH, (float)GRABBER_HEIGHT, false );
	m_compCoordTransform.SetTranslation( 0.0f, 0.0f, 0.225f );
	m_compCoordTransform.SetRotation( 0.0f, 0.0f, 0.0f );
	m_compCoordTransform.SetPlane( 0.0f, 0.0f, 0.0f,
						  			0.0f, 0.0f, 1.0f );
	

	
	// *************************************
	//  Set models and transforms in the network
	// *************************************
	m_compLabel.SetTransform( &m_compCoordTransform );
	m_compDemoSorterClassifier.SetModel( &m_compModel );
	m_compDemoSorterClassifier.SetTransform( &m_compCoordTransform );
	m_compModelVisualizer.SetTransform( &m_compCoordTransform );
	m_compModelVisualizer.SetModel( &m_compModel );	
		
}

// *************************************************************************

void CVision::FeedImage( Ptr pBuffer )
{
	m_compFrameGrabber.SetInputBuffer( pBuffer );
}

// *************************************************************************

void CVision::DoProcessing()
{
	// Check memory boundaries.
	CVisBufferManager::Instance()->CheckMemoryBoundaries();
	
	// Clear all events
	ClearEvents();
	
	// Propagate Time
	m_compDemoSorterClassifier.SetCurrentImageTime( m_unCurrentImageTime );

	// Grab the frame
	m_compFrameGrabber.DoProcessing();
	m_compClip.DoProcessing();

	// Distinguish whether we're in idle, service, calibration or classification mode.
	switch ( m_vmMode )
	{
	case VM_IDLE:
		break;
		
	case VM_SERVICE:
		break;

	case VM_CALIBRATION:
		m_compModelVisualizer.DoProcessing();
		
		break;

	case VM_CLASSIFICATION:	
		m_compRGBToGray.DoProcessing();
		m_compThreshold.DoProcessing();
		/*
		m_compMorphology.DoProcessing();
		m_compKeypointsCut.DoProcessing();
		*/
		m_compLabel.DoProcessing();
		
		m_compColorPick.DoProcessing();		
		
		m_compDemoSorterClassifier.DoProcessing();
		
#ifdef _WINDOWS
		m_compLabelVisualizer.DoProcessing();		
		m_compColorVisualizer.DoProcessing();
#endif	
		
		// Generate events, if there were objects.
		if ( m_compDemoSorterClassifier.GetNumObjectsFound() > 0 )
			SetEvent( VE_OBJECTS_FOUND );
			
		break;
	}	
#ifdef _WINDOWS_
		m_compModelVisualizer.DoProcessing();
#endif
}

// *************************************************************************

void CVision::ChangeMode( VisionMode newMode )
{
	// If we're not in idle and the request is not the current mode or idle, we have
	// to switch back to idle first and then to the new mode.
	if ( (m_vmMode != VM_IDLE) && (newMode != VM_IDLE) && (newMode != m_vmMode))
		ChangeMode( VM_IDLE );
	
	m_vmMode = newMode;

	// Reconfigure the vision network.
	switch ( m_vmMode )
	{
	case VM_IDLE:		
		SetComponentsOperationMode( CVisComponent::OP_IDLE );
		break;
		
	case VM_CLASSIFICATION:
		SetComponentsOperationMode( CVisComponent::OP_CLASSIFICATION );		
		break;

	case VM_SERVICE:
		SetComponentsOperationMode( CVisComponent::OP_SERVICE );		
		break;
		
	case VM_CALIBRATION:
		SetEvent( 1 );
		SetComponentsOperationMode( CVisComponent::OP_CALIBRATION );
		break;
	}
}

// *************************************************************************

CVision::VisionMode CVision::GetMode( )
{
	return m_vmMode;
}

// *************************************************************************
