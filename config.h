#ifndef CONFIG_H_
#define CONFIG_H_

#include "support.h"

struct {
	bool sort_color[4];
	int count_color[4];
	int count_sorted;
} configuration;

void config_init();
void config_read();

#endif /* CONFIG_H_ */
