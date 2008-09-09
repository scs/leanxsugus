
#include "classWrapper.h"
#include "classImageProcessing.h"
#include "classControl.h"

/** THE global vision object. Allocated statically. */
CImageProcessing g_ImageProcessing;

/** THE global control object. Allocated statically. */
CControl g_Control( & g_ImageProcessing );

void StartVisionTask()
{
	g_ImageProcessing.SetControl( & g_Control );
	g_ImageProcessing.StartTask();
}

void StartControlTask()
{
	g_Control.StartTask();
}

void SendDebugMsg( const char * str )
{
	g_Control.PostDebugMsg( str );
}

