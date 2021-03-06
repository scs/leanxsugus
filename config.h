#ifndef CONFIG_H_
#define CONFIG_H_

#include "support.h"

struct {
	/*! @brief Whether to sort each color of sugus respectively. */
	bool sort_color[6];
	/*! @brief Wether we are in calibration mode. */
	bool calibrating;
	/*! @brief Valves which should be activated no matter what. */
	bool valve_override[16];
	/*! @brief Number of sugus of each color counted so far. */
	uint32 count_color_sorted[6];
	uint32 count_color_ignored[6];
} configuration;

void config_write();
void config_read();
void config_init();

#endif /* CONFIG_H_ */
