#ifndef CONFIG_H_
#define CONFIG_H_

#include "support.h"

struct {
	bool sort_color1;
	bool sort_color2;
	bool sort_color3;
	bool sort_color4;
} configuration;

void config_init();
void config_read();

#endif /* CONFIG_H_ */
