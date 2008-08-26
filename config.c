/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "config.h"

#define CONFIG_FILENAME "/home/httpd/cgi-bin/config"
#define STATISTICS_FILENAME "/home/httpd/statistics.txt"

void config_write()
{
	int fd, i, ret;
	FILE * file;
	
	fd = open(STATISTICS_FILENAME "~", O_WRONLY | O_CREAT);
	
	if (fd == -1)
	{
		printf("The statistics journal file could not be created.\n");
		return;
	}
	
	file = fdopen(fd, "w");
	
	for (i = 0; i < 4; i += 1)
		fprintf(file, "count_color_%d=%lu\n", i, configuration.count_color[i]);
	
	fprintf(file, "count_sorted=%lu\n", configuration.count_sorted);
	fprintf(file, "count_unknown=%lu\n", configuration.count_unknown);
	
	fclose(file);
	close(fd);
	
	ret = rename(STATISTICS_FILENAME "~", STATISTICS_FILENAME);
	if (ret)
		printf("The statistics file could be moved to its final place.\n");
}

void config_read()
{
	static int fd = -1;
	static FILE * file;
	char buf[80] = { 0 }; /* Damned be the ones who need more than 80 characters. */
	char * pos, * ret;
	
	if (fd == -1)
	{
		fd = open(CONFIG_FILENAME, O_NONBLOCK | O_RDONLY);
		
		if (fd == -1)
		{
			printf("The configuration fifo could not be opened.\n");
			return;
		}
		
		file = fdopen(fd, "r");
	}
	
	/* Gets one line. */
	ret = fgets(buf, sizeof buf, file);
	
	if (ret == NULL) {
		if (feof(file))
		{ /* There was no data available in the pipe. */
		//	printf("No commands availible...\n");
			
			fclose(file);
			close(fd);
			fd = -1;
			
			return;
		}
		
		if (errno == EAGAIN)
		{ /* There is data, but not a complete line. */
		//	printf("Waiting for a complete line...\n");
			return;
		}
	}
	
	buf[strlen(buf) - 1] = 0; /* Remove the trailing newline. */
	pos = strstr(buf, "="); /* Find the equals sign. */
	
	/* Test if there was an equals sign in the line */
	if (pos != NULL)
	{
		*pos = 0; /* End the first part of the line. */
		pos += 1; /* Move into the second part of the string. */
		
		printf("%s = %s\n", buf, pos);
		
		if (strncmp(buf, "sort_color_", 11) == 0)
		{
			t_index n = atoi(buf + 11);
			
			if (0 <= n && n < 4)
				configuration.sort_color[n] = strcmp(pos, "true") == 0;
		}
		
		if (strcmp(buf, "sort_unknown") == 0)
			configuration.sort_unknown = strcmp(pos, "true") == 0;
		
		if (strncmp(buf, "valve_override_", 15) == 0)
		{
			t_index n = atoi(buf + 15);
			
			if (0 <= n && n < 16)
				configuration.valve_override[n] = strcmp(pos, "true") == 0;
		}
		
		if (strcmp(buf, "calibrating") == 0)
			configuration.calibrating = strcmp(pos, "true") == 0;
	}
	else
	{
		printf("%s\n", buf);
		
		if (strcmp(buf, "reset_counter") == 0)
		{
			configuration.count_color[0] = 0;
			configuration.count_color[1] = 0;
			configuration.count_color[2] = 0;
			configuration.count_color[3] = 0;
			
			configuration.count_sorted = 0;
		}
	}
}

void config_init() {
	configuration.sort_color[0] = false;
	configuration.sort_color[1] = false;
	configuration.sort_color[2] = false;
	configuration.sort_color[3] = false;
	
	configuration.count_color[0] = 0;
	configuration.count_color[1] = 0;
	configuration.count_color[2] = 0;
	configuration.count_color[3] = 0;
	
	configuration.count_sorted = 0;
}
