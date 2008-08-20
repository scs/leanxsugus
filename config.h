#ifndef CONFIG_H_
#define CONFIG_H_

#include "support.h"

struct {
	bool sort_color[4];
	bool sort_unknown;
	int32 count_color[4];
	int32 count_sorted;
	int32 count_unknown;
	bool calibrating;
	bool valve_override[16];
} configuration;

void config_write();
void config_read();
void config_init();

#endif /* CONFIG_H_ */
