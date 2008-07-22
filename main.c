/*!
 * @file main.c
 * @brief Main file of the template application. Mainly contains initialization code.
 */
#include "leanxsugus.h"
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define loop while (TRUE)

/*! @brief This stores all variables needed by the algorithm. */
struct TEMPLATE data;

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
//	uint8 multiBufferIds[2] = {0, 1};
	
	memset(&data, 0, sizeof(struct TEMPLATE));
	
	/* Create the framework */
	err = OscCreate(&data.hFramework);
	if (err != SUCCESS)
	{
		fprintf(stderr, "%s: error: Unable to create framework.\n", __func__);
		return err;
	}
	
	/* Load the framework module dependencies. */
	err = OscLoadDependencies (data.hFramework, deps, length (deps));
	if (err != SUCCESS)
	{
		fprintf(stderr, "%s: error: Unable to load dependencies! (%d)\n", __func__, err);
		goto dep_err;
	}
	
	/* Seed the random generator */
	srand(OscSupCycGet());
	
#if defined(OSC_HOST) || defined(OSC_SIM)
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
#endif /* defined(OSC_HOST) || defined(OSC_SIM) */

#if defined(OSC_HOST)
	/* Set the camera registers to sane default values. */
	err = OscCamPresetRegs();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to preset camera registers! (%d)\n", __func__, err);
		goto fb_err;
	}
#endif /* defined(OSC_HOST) */
	  
	/* Set up two frame buffers with enough space for the maximum camera resolution in cached memory. */
	err = OscCamSetFrameBuffer(0, OSC_CAM_MAX_IMAGE_WIDTH * OSC_CAM_MAX_IMAGE_HEIGHT, data.frameBuffer, TRUE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up the frame buffer!\n",
				__func__);
		goto fb_err;
	}
	
	/* Create a double-buffer from the frame buffers initilalized above. */
/*	err = OscCamCreateMultiBuffer(2, multiBufferIds);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up multi buffer!\n",
				__func__);
		goto mb_err;
	} */
	
	/* Register an IPC channel to the CGI for the user interface. */
	err = OscIpcRegisterChannel(&data.ipc.ipcChan,
			USER_INTERFACE_SOCKET_PATH,
			F_IPC_SERVER | F_IPC_NONBLOCKING);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to initialize IPC channel to web"
				"interface! (%d)\n", __func__, err);
		goto ipc_err;
	}
		
	err |= PerspectiveCfgStr2Enum("DEFAULT", &data.perspective);
	if ( err != SUCCESS)
	{
		OscLog(WARN,
				 "%s: No (valid) camera-scene perspective defined "
				 "in EEPROM or no EEPROM found, use default.\n",
				 __func__);
	}
	OscCamSetupPerspective( data.perspective);
	
	/* Initialieses the object recognition data . */
	
	return SUCCESS;
	   
ipc_err:
mb_err:
fb_err:
#if defined(OSC_HOST) || defined(OSC_SIM)
frd_err:
#endif
	OscUnloadDependencies(data.hFramework, deps, sizeof(deps)/sizeof(struct OSC_DEPENDENCY));
dep_err:
	OscDestroy(&data.hFramework);
	
	return err;
}

OSC_ERR Unload()
{
	/* Unload the framework module dependencies */
	OscUnloadDependencies(data.hFramework,
			deps,
			sizeof(deps)/sizeof(struct OSC_DEPENDENCY));
	
	OscDestroy(data.hFramework);
   
	return SUCCESS;
}

#define m printf("%s: Line %d\n", __func__, __LINE__);

void mainLoop () {
	static uint8 frameBuffer[captureWidth * captureHeight];
	t_index width = OSC_CAM_MAX_IMAGE_WIDTH, height = OSC_CAM_MAX_IMAGE_HEIGHT;
	OSC_ERR err = SUCCESS;
	
	err = OscCamPresetRegs();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to preset camera registers! (%d)\n", __func__, err);
		return;
	}
	
	err = OscCamSetFrameBuffer(0, sizeof frameBuffer, frameBuffer, TRUE);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to set up the frame buffer!\n",
				__func__);
		return;
	}
	
	loop {
		static uint8 * frameBuffer;
		
		err = OscCamSetupCapture(0, OSC_CAM_TRIGGER_MODE_MANUAL);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to trigger initial capture (%d)!\n", __func__, err);
			return;
		}
		
	//	err = OscCamReadPicture(&frameBuffer);
		err = OscCamReadPicture(0, &frameBuffer, 0, 0);
		if (err != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable read the picture (%d)!\n", __func__, err);
			return;
		}
		
		processFrame(frameBuffer, width, height);
	}
}

/*!
 * @brief Program entry
 *
 * @param argc Command line argument count.
 * @param argv Command line argument strings.
 * @return 0 on success
 */
int main(const int argc, const char * * argv)
{
	OSC_ERR err = SUCCESS;
	
	err = init(argc, argv);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Initialization failed!(%d)\n", __func__, err);
		return err;
	}
	OscLog(INFO, "Initialization successful!\n");
	
	OscLogSetConsoleLogLevel(INFO);
	OscLogSetFileLogLevel(WARN);
	
//	StateControl();
	mainLoop();
		
	Unload();
	return 0;
}
