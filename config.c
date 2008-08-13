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
	{
		fprintf(file, "sort_color_%d=%s\n", i, configuration.sort_color[i] ? "true" : "false");
		fprintf(file, "count_color_%d=%d\n", i, configuration.count_color[i]);
	}
	
	fprintf(file, "count_sorted=%d\n", configuration.count_sorted);
	
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
			printf("No commands availible...\n");
			
			fclose(file);
			close(fd);
			fd = -1;
			
			return;
		}
		
		if (errno == EAGAIN)
		{ /* There is data, but not a complete line. */
			printf("Waiting for a complete line...\n");
			return;
		}
	}
	
	/* Find the equals sign. */
	pos = strstr(buf, "=");
	
	/* Test if there was an equals sign in the line */
	if (pos != NULL)
	{
		buf[strlen(buf) - 1] = 0; /* Remove the trailing newline. */
		*pos = 0; /* End the first part of the line. */
		pos += 1; /* Move into the second part of the string. */
		
		printf("%s = %s\n", buf, pos);
		
		if (strcmp(buf, "sort_color_0") == 0)
			configuration.sort_color[0] = strcmp(pos, "true") == 0;
		else if (strcmp(buf, "sort_color_1") == 0)
			configuration.sort_color[1] = strcmp(pos, "true") == 0;
		else if (strcmp(buf, "sort_color_2") == 0)
			configuration.sort_color[2] = strcmp(pos, "true") == 0;
		else if (strcmp(buf, "sort_color_3") == 0)
			configuration.sort_color[3] = strcmp(pos, "true") == 0;
	}
	else
		if (strcmp(buf, "reset_counter") == 0)
		{
			configuration.count_color[0] = 0;
			configuration.count_color[1] = 0;
			configuration.count_color[2] = 0;
			configuration.count_color[3] = 0;
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
