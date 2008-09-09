/*! @file config.c
 * @brief Code used for interaction with the web-interface.
 * 
 * Thsi file defines functions to write the statistics file and read configuration updates from the configuration pipe.
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "config.h"

/*! @brief Name of the pipe to read configuration commands from. */
#define CONFIG_FILENAME "/home/httpd/cgi-bin/config"
/*! @brief Name to write statistical data to. */
#define STATISTICS_FILENAME "/home/httpd/statistics.txt"

/*! @brief Writes a file with all the statistics values to be read by the web interface. */
void config_write()
{
	int fd, i, ret;
	FILE * file;
	
	/* Open the statistics journal file. */
	fd = open(STATISTICS_FILENAME "~", O_WRONLY | O_CREAT);
	
	if (fd == -1)
	{
		printf("The statistics journal file could not be created.\n");
		return;
	}
	
	/* Create a stream out of the file descriptor. */
	file = fdopen(fd, "w");
	
	/* Write the counts of all the sugus by color. */
	for (i = 0; i < 4; i += 1)
		fprintf(file, "count_color_%d=%lu\n", i, configuration.count_color[i]);
	
	/* Write the counts of othe other categories. */
	fprintf(file, "count_sorted=%lu\n", configuration.count_sorted);
	fprintf(file, "count_unknown=%lu\n", configuration.count_unknown);
	
	/* Close the file stream and descripor. */
	fclose(file);
	close(fd);
	
	/* Move the journal to the final path. */
	ret = rename(STATISTICS_FILENAME "~", STATISTICS_FILENAME);
	if (ret)
		printf("The statistics file could be moved to its final place.\n");
}

/*!
 * @brief Reads command availible from the configuration pipe.
 *
 * The following commands are understood by the configuration interface:
 *	- "sort_color_" <n> "=" ( "true" | "false" ): Sort out sugus of the specified color where n may be:
 *		- "0": Green sugus.
 *		- "1": Yellow sugus.
 *		- "2": Orange sugus.
 *		- "3": Red sugus.
 *	- "sort_unknown=" ( "true" | "false" ): Whether to sort sugus whose color could not be determined.
 *	- "valve_override_" <n> "=" ( "true" | "false" ): Whether to activate valve <n> no matter what.
 *	- "calibrating=" ( "true" | "false" ): Wheter to activate the calibration mode.
 *	- "reset_counter": Reset all the counters to zero.
 */
void config_read()
{
	static int fd = -1;
	static FILE * file;
	char buf[80] = { 0 }; /* Damned be the ones who need more than 80 characters. */
	char * pos, * ret;
	
	/* Check whether we already have the pipe opened. */
	if (fd == -1)
	{
		/* Open the pipe nonblockingly. */
		fd = open(CONFIG_FILENAME, O_NONBLOCK | O_RDONLY);
		
		if (fd == -1)
		{
			printf("The configuration fifo could not be opened.\n");
			return;
		}
		
		/* Create a stream out if the file descriptor. */
		file = fdopen(fd, "r");
	}
	
	/* Gets one line. */
	ret = fgets(buf, sizeof buf, file);
	
	/* Check whether we read something. */
	if (ret == NULL) {
		if (feof(file))
		{	/* There was no data available in the pipe. */
			fclose(file);
			close(fd);
			fd = -1;
			
			return;
		}
		
		if (errno == EAGAIN)
		{	/* There is data, but not a complete line. */
			return;
		}
	}
	
	buf[strlen(buf) - 1] = 0; /* Remove the trailing newline. */
	pos = strstr(buf, "="); /* Find the equals sign. */
	
	/* Test if there was an equals sign in the line */
	if (pos != NULL)
	{	/* There was an equals sign at pos. */
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
	{	/* There was no equals sign. */
		printf("%s\n", buf);
		
		if (strcmp(buf, "reset_counters") == 0)
		{
			configuration.count_color[0] = 0;
			configuration.count_color[1] = 0;
			configuration.count_color[2] = 0;
			configuration.count_color[3] = 0;
			
			configuration.count_sorted = 0;
		}
	}
}

/*! @brief Initializes the configuration subsystem, mainly sets all the configuration variables to their default value. */
void config_init() {
	int i;
	
	for (i = 0; i < length (configuration.sort_color); i += 1)
		configuration.count_color[i] = 0;
	
	for (i = 0; i < length (configuration.valve_override); i += 1)
		configuration.valve_override[i] = false;
	
	configuration.sort_color[0] = false;
	configuration.sort_color[1] = false;
	configuration.sort_color[2] = true;
	configuration.sort_color[3] = true;
	
	configuration.calibrating = false;
	configuration.sort_unknown = false;
	configuration.count_sorted = false;
	configuration.count_sorted = 0;
}
