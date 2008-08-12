/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "config.h"

#define CONFIG_FILENAME "/tmp/leanxsugus-config"

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
			char buf[80]; /* Damned be the ones who need more than 80 characters. */
			char * pos;
			
			/* Get one line and remove the trailing newline. */
			fgets(buf, sizeof buf, pFile);
			buf[strlen(buf) - 1] = 0;
			
			/* Find the equals sign. */
			pos = strstr(buf, "=");
			
			if (pos == NULL) /* Invalid line. */
				continue;
			
			*pos = 0;
			pos += 1; /* Move into the second part of the string. */
			
			printf("%s = %s\n", buf, pos);
			
			if (strcmp(buf, "sort_color1") == 0)
				configuration.sort_color1 = strcmp(pos, "true") == 0;
			else if (strcmp(buf, "sort_color2") == 0)
				configuration.sort_color2 = strcmp(pos, "true") == 0;
			else if (strcmp(buf, "sort_color3") == 0)
				configuration.sort_color3 = strcmp(pos, "true") == 0;
			else if (strcmp(buf, "sort_color4") == 0)
				configuration.sort_color4 = strcmp(pos, "true") == 0;
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
