#ifndef CONFIG_H_
#define CONFIG_H_

#include "support.h"


struct {
	bool sort_color[4];
	int count_color[4];
	int count_sorted;
} configuration;

void config_write();
void config_read();
void config_init();

#endif /* CONFIG_H_ */
