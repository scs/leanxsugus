/*!
 * @file main.c
 * @brief Main file of the template application. Mainly contains initialization code.
 */

/*!
 * \mainpage
 * \image html sugus-orange.png
 * \section Introduction
 * This is the documentation of the LeanXsugus source code. LeanXsugus is a Project to port the Sugus sorting algorithm of the SCS Demo-Sorter to the LeanXcam.
 *
 */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

//#define BENCHMARK_ENABLE

#include "process.h"
#include "config.h"
#include "modbus.h"
#include "valves.h"
#include "main.h"

void *hFramework;

/*! @brief The framework module dependencies of this application. */
struct OSC_DEPENDENCY deps[] = {
	{ "log", OscLogCreate, OscLogDestroy },
	{ "sup", OscSupCreate, OscSupDestroy },
	{ "bmp", OscBmpCreate, OscBmpDestroy },
	{ "cam", OscCamCreate, OscCamDestroy },
	{ "vis", OscVisCreate, OscVisDestroy }
};

/*!
 * @brief Initialize everything so the application is fully operable after a call to this function.
 *
 * @return SUCCESS or an appropriate error code.
 */
static OSC_ERR init(const int argc, const char * * argv)
{
	OSC_ERR err = SUCCESS;
	
	/* Create the framework */
	err = OscCreate(&hFramework);
	if (err != SUCCESS)
	{
		fprintf(stderr, "%s: error: Unable to create framework.\n", __func__);
		return err;
	}
	
	/* Load the framework module dependencies. */
	err = OscLoadDependencies (hFramework, deps, length (deps));
	if (err != SUCCESS)
	{
		fprintf(stderr, "%s: error: Unable to load dependencies! (%d)\n", __func__, err);
		goto dep_err;
	}
	
	/* Set log levels. */
	OscLogSetConsoleLogLevel(ERROR);
	OscLogSetFileLogLevel(ERROR);
	
	/* Seed the random generator */
	srand(OscSupCycGet());
	
	/* Set the camera registers to sane default values. */
	err = OscCamPresetRegs();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to preset camera registers! (%d)\n", __func__, err);
		goto fb_err;
	}
	
	OscCamSetupPerspective(OSC_CAM_PERSPECTIVE_DEFAULT);
	
	/* Initializes all parts if the application */
	process_init();
	config_init();
	valves_init();
	modbus_init();
	
	return SUCCESS;
	
fb_err:
	OscUnloadDependencies(hFramework, deps, sizeof(deps)/sizeof(struct OSC_DEPENDENCY));
	
dep_err:
	OscDestroy(&hFramework);
	
	return err;
}

OSC_ERR Unload()
{
	/* Unload the framework module dependencies */
	OscUnloadDependencies(hFramework,
			deps,
			sizeof(deps)/sizeof(struct OSC_DEPENDENCY));
	
	OscDestroy(hFramework);
	
	return SUCCESS;
}

/*!
 * @brief This is the main loop which is called after the initialisation of the application.
 *
 * This loop alternately takes pictures and calls parts of the aplication to process the image, handle the valves and read and write configuration data.
 */
OSC_ERR mainLoop() {
	OSC_ERR err = SUCCESS;
	static uint8 frameBuffer[OSC_CAM_MAX_IMAGE_WIDTH * OSC_CAM_MAX_IMAGE_HEIGHT];
	
	/* This sets the sensor area to capture the picture from. */
	err = OscCamSetAreaOfInterest((OSC_CAM_MAX_IMAGE_WIDTH - WIDTH_CAPTURE) / 2, (OSC_CAM_MAX_IMAGE_HEIGHT - HEIGHT_CAPTURE) / 2 + 50, WIDTH_CAPTURE, HEIGHT_CAPTURE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set the area of interest!\n", __func__);
		return err;
	}
	
	/* This set the exposure time to a reasonable value for the linghting in the sorter. */
	err = OscCamSetShutterWidth(5000);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set the exposure time!\n", __func__);
		return err;
	}
	
	/* This sets the frame buffer to store the captured picture in. */
	err = OscCamSetFrameBuffer(0, sizeof frameBuffer, frameBuffer, TRUE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up the frame buffer!\n", __func__);
		return err;
	}
	
	/* This handles the Valves for the first time, so we will be in sync from here on. */
	valves_handleValves();
	
	loop {
		uint8 * pFrameBuffer;
		time_t capture_time;
		
	benchmark_init;
		
	retry:
		/* This starts the capture and records the time. */
		err = OscCamSetupCapture(0, OSC_CAM_TRIGGER_MODE_MANUAL);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to trigger the capture (%d)!\n", __func__, err);
			goto retry;
		}
		capture_time = OscSupCycGet();
		
		valves_handleValves();
		
		config_read();
		config_write();
		
	benchmark_delta;
		
		/* Here we wait for the picture be availible in the frame buffer. */
		err = OscCamReadPicture(0, &pFrameBuffer, 0, 0);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to read the picture (%d)!\n", __func__, err);
			return err;
		}
		
	benchmark_delta;
		
		valves_handleValves();
		
		/* This processes the image and fills the valve buffer. */
		process(pFrameBuffer, capture_time);
	}
	
	return SUCCESS;
}

/*!
 * @brief Program entry
 *
 * @param argc Command line argument count.
 * @param argv Command line argument strings.
 * @return 0 on success
 */
int main(const int argc, const char ** argv)
{
	OSC_ERR err = SUCCESS;
	
	/* This initializes various parts of the framework and the application. */
	err = init(argc, argv);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Initialization failed!(%d)\n", __func__, err);
		return err;
	}
	OscLog(INFO, "Initialization successful!\n");
	
	/* Calls the main loop. This only returns on an error. */
	mainLoop();
	
	/* On an error, we unload the framework, before we exit. */
	Unload();
	return 0;
}
