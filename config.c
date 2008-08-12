/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"

#define CONFIG_FILENAME "/tmp/leanxsugus-config"

sig_atomic_t flag_readConfig;

void scheduleRead(int dummy) {
	flag_readConfig = true;
}

void config_read() {
	int fd = open(CONFIG_FILENAME, O_NONBLOCK | O_RDONLY);
	FILE * file = fdopen(fd, "r");
	
	if (fd == -1)
	{ /* There is no configuration pipe. */
		printf("There is no configuration pipe.\n");
		return;
	}
	
	printf("Reading in configuration commands...\n");
	
	loop {
		char buf[80] = { 0 }; /* Damned be the ones who need more than 80 characters. */
		char * pos;
		char * ret = fgets(buf, sizeof buf, file); /* Gets one line. */
		
		if (feof(file)) /* There was no data available in the pipe. */
			break;
		
		if (ret == NULL && errno == EAGAIN) /* There is data, but not a complete line. */
			continue;
		
		/* Find the equals sign. */
		pos = strstr(buf, "=");
		
		if (pos == NULL) /* There was no equals sign in this line so we discard it at the moment. */
			continue;
		
		buf[strlen(buf) - 1] = 0; /* Remove the trailing newline. */
		*pos = 0; /* End the first part of the line. */
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
	
	close(fd);
	fclose(file);
}

void config_init() {
	flag_readConfig = true;
	signal (SIGHUP, &scheduleRead);
}
