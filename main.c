/*!
 * @file main.c
 * @brief Main file of the template application. Mainly contains initialization code.
 */
#include "main.h"
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define loop while (TRUE)

void *hFramework;

/*! @brief The framework module dependencies of this application. */
struct OSC_DEPENDENCY deps[] = {
	{ "log", OscLogCreate, OscLogDestroy },
	{ "sup", OscSupCreate, OscSupDestroy },
	{ "bmp", OscBmpCreate, OscBmpDestroy },
	{ "cam", OscCamCreate, OscCamDestroy },
	{ "hsm", OscHsmCreate, OscHsmDestroy },
	{ "ipc", OscIpcCreate, OscIpcDestroy },
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
	
	/* Seed the random generator */
	srand(OscSupCycGet());
	
#if defined(OSC_HOST) || defined(OSC_SIM)
	{
		void *hFileNameReader;
		
	/*	err = OscFrdCreateConstantReader(&data.hFileNameReader, TEST_IMAGE_FN);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to create constant file name reader for %s! (%d)\n", __func__, TEST_IMAGE_FN, err);
			goto frd_err;
		}
		
		err = OscCamSetFileNameReader(data.hFileNameReader);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to set file name reader for camera! (%d)\n", __func__, err);
			goto frd_err;
		} */
		
		{
			void * hTestImageReader;
			
			OscFrdCreateFileListReader(&hTestImageReader, "cam-file-list.txt");
			OscCamSetFileNameReader(hTestImageReader);
		}
	}
#endif /* defined(OSC_HOST) || defined(OSC_SIM) */

	/* Set the camera registers to sane default values. */
	err = OscCamPresetRegs();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to preset camera registers! (%d)\n", __func__, err);
		goto fb_err;
	}
	
	{
		enum EnOscCamPerspective perspective;
		err |= PerspectiveCfgStr2Enum("DEFAULT", &perspective);
		if ( err != SUCCESS)
		{
			OscLog(WARN, "%s: No (valid) camera-scene perspective defined in EEPROM or no EEPROM found, use default.\n", __func__);
		}
		OscCamSetupPerspective(perspective);
	}
	
	/* Initialieses the object recognition data . */
	
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

#define m printf("%s: Line %d\n", __func__, __LINE__);

uint8 frameBuffers[2][OSC_CAM_MAX_IMAGE_WIDTH * OSC_CAM_MAX_IMAGE_HEIGHT];

OSC_ERR mainLoop () {
//	static uint8 frameBuffers[2][widthCapture * heightCapture];
	
	uint8 multiBufferIds[] = { 0, 1 };
	OSC_ERR err = SUCCESS;
	
	err = OscCamSetAreaOfInterest((OSC_CAM_MAX_IMAGE_WIDTH - widthCapture) / 2, (OSC_CAM_MAX_IMAGE_HEIGHT - heightCapture) / 2, widthCapture, heightCapture);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set the area of interest!\n", __func__);
		return err;
	}
	
	err = OscCamSetFrameBuffer(0, sizeof frameBuffers[0], frameBuffers[0], TRUE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up the frame buffer!\n", __func__);
		return err;
	}
	
	err = OscCamSetFrameBuffer(1, sizeof frameBuffers[1], frameBuffers[1], TRUE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up the frame buffer!\n", __func__);
		return err;
	}
	
	/* Create a double-buffer from the frame buffers initilalized above.*/
    err = OscCamCreateMultiBuffer(2, multiBufferIds);
    if(err != SUCCESS)
    {
        OscLog(ERROR, "%s: Unable to set up multi buffer!\n",
                __func__);
       return err;
    }
		
	err = OscCamSetupCapture(OSC_CAM_MULTI_BUFFER, OSC_CAM_TRIGGER_MODE_MANUAL);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to trigger initial capture (%d)!\n", __func__, err);
		return err;
	}
	
	/* Wait for the camera module to realize that it should initiate a picture. */
	usleep(100 * 1000);
	
	loop {
		static uint8 * pFrameBuffer;
		
	//	readConfig();
		
		err = OscCamSetupCapture(OSC_CAM_MULTI_BUFFER, OSC_CAM_TRIGGER_MODE_MANUAL);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to trigger initial capture (%d)!\n", __func__, err);
			return err;
		}
		
		err = OscCamReadLatestPicture(&pFrameBuffer);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to read the picture (%d)!\n", __func__, err);
			return err;
		}
		
		processFrame(pFrameBuffer);
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
	
//	OscLogSetConsoleLogLevel(INFO);
//	OscLogSetFileLogLevel(WARN);
	
	OscLogSetConsoleLogLevel(DEBUG);
	OscLogSetFileLogLevel(DEBUG);
	
	err = init(argc, argv);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Initialization failed!(%d)\n", __func__, err);
		return err;
	}
	OscLog(INFO, "Initialization successful!\n");
	
	processFrame_init();
	config_init();
	
	mainLoop();
		
	Unload();
	return 0;
}
