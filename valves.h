#ifndef VALVES_H_
#define VALVES_H_

#include "support.h"

#define TIME_TO_TOP_OF_PICTURE (CLOCKS_PER_SEC / 1000000 * 142784)
#define TIME_TO_BOTTOM_OF_PICTURE (CLOCKS_PER_SEC / 1000000 * 191565)
#define TIME_TO_VALVES (CLOCKS_PER_SEC / 1000000 * 247309)

#define PIXEL_BEGIN_FIRST_VALVE 20
#define PIXEL_END_LAST_VALVE 334

void valves_init();
void valves_insertEvent(t_time const begin_time, t_time const end_time, t_index const first_valve, t_index const last_valve);
void valves_handleValves();

#endif /* VALVES_H_ */
