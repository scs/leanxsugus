#ifndef VALVES_H_
#define VALVES_H_

#include "support.h"

#define CPU_FREQ 500000000

/*! @brief Time a sugus needs from the conveyor belt to the top of the picture. */
#define TIME_TO_TOP_OF_PICTURE ((uint32) CPU_FREQ / 1000000 * 144205)
/*! @brief Time a sugus needs from the conveyor belt to the bottom of the picture. */
#define TIME_TO_BOTTOM_OF_PICTURE ((uint32) CPU_FREQ / 1000000 * 222120)
/*! @brief Time a sugus needs from the conveyor belt to the valves row. */
#define TIME_TO_VALVES ((uint32) CPU_FREQ / 1000000 * 247309)

/*! @brief Position in the picture where the influence range of the first valve starts. This is usually the position of the center of the first valve minus half the distance between two valves. */
#define PIXEL_BEGIN_FIRST_VALVE ((int16) 0)
/*! @brief Position in the picture where the influence range of the last valve ends. */
#define PIXEL_END_LAST_VALVE ((int16) 376)

void valves_init();
void valves_insertEvent(t_time const begin_time, t_time const end_time, t_index const first_valve, t_index const last_valve);
void valves_handleValves();

#endif /* VALVES_H_ */
