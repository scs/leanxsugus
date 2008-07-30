/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "config.h"

#define CONFIG_FILENAME "/tmp/leanxsugus-config"

struct {
	bool sort_color1;
	bool sort_color2;
	bool sort_color3;
	bool sort_color4;
} configuration;

sig_atomic_t flag_readConfig;

void scheduleRead(int dummy) {
	flag_readConfig = true;
}

void config_read() {
	if (flag_readConfig)
	{
		FILE * pFile = fopen(CONFIG_FILENAME, "r");
		
		if (pFile == NULL)
		{
	//		printf("No config file found!\n");
			return;
		}
	//	printf("Reading configuration...\n");
		
		while (! feof(pFile)) {
			char buf0[100], buf1[100], buf2[100];
			char * pos;
			
			/* Get one line and remove the trailing newline. */
			fgets(buf0, sizeof buf0, pFile);
			buf0[strlen(buf0) - 1] = 0;
			
			/* Find the equals sign. */
			pos = strstr(buf0, "=");
			
			/* Invalid line. */
			if (pos == NULL)
				continue;
			*pos = 0;
			
			/* Split the string. */
			strcpy(buf1, buf0);
			strcpy(buf2, pos + 1);
			
		//	printf("%s\n%s\n", buf1, buf2);
			
			if (strcmp(buf1, "sort_color1") == 0)
				configuration.sort_color1 = strcmp(buf2, "true") == 0;
			else if (strcmp(buf1, "sort_color2") == 0)
				configuration.sort_color2 = strcmp(buf2, "true") == 0;
			else if (strcmp(buf1, "sort_color3") == 0)
				configuration.sort_color3 = strcmp(buf2, "true") == 0;
			else if (strcmp(buf1, "sort_color4") == 0)
				configuration.sort_color4 = strcmp(buf2, "true") == 0;
			
		}
		
	//	printf("Configuration read.\n");
		fclose(pFile);
		flag_readConfig = false;
	}
}

void config_init() {
	flag_readConfig = true;
	signal (SIGHUP, &scheduleRead);
}
