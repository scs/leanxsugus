/*! @file valves.c
 * @brief Contains function to handle the valves with support of the modbus component.
 *
 * This file defines functions to manipulate the valve ring-buffer and send updates to the modbus interface at the right time.
 */

#include <unistd.h>

#define ASSERTS_ENABLE

#include "modbus.h"
#include "config.h"
#include "valves.h"

#include <stdbool.h>

/* This value may be used to adjust the timing of the valves. Larger values delay the activation of the valves. */
#define TUNE_VALVES_ON ((uint32) CPU_FREQ / 1000 * -20)
#define TUNE_VALVES_OFF ((uint32) CPU_FREQ / 1000 * 10)

/* This sets to handle the valves a hundred times per second. */
#define INTERVAL ((uint32) CPU_FREQ / 100)
/* This defines how many time steps ahead we can set a valve state */
#define VALUES_AHEAD 100

struct {
	/*! @brief Time the values values[nex_values] should be written to the modbus interface. */
	t_time next_time;
	/*! @brief values[next_values] contains the valve values that will be written to the modbus interface next. */
	t_index next_values;
	/*! @brief This is the ring-buffer that contains the valve values for the future. */
	bool values[VALUES_AHEAD][16];
} valves;

/*!
 * @brief This inserts an event into the valve ring-buffer.
 *
 * A valve event will be inserted into the ring-buffer so the specified valves will activate at the specified time.
 *
 * @param begin_time The time at wich the valves should open.
 * @param end_time The time at wich the valves should close.
 * @param first_valve Number of the first valve to affect.
 * @param last_valve Number of the last valve to affect.
 */
void valves_insertEvent(t_time const begin_time, t_time const end_time, t_index const first_valve, t_index const last_valve)
{
	int32 const time_begin = begin_time - valves.next_time + TUNE_VALVES_ON;
	int32 const time_end = end_time - valves.next_time + TUNE_VALVES_OFF;
	t_index i, j, ahead_begin, ahead_end;
	
	assert (begin_time >= valves.next_time);
	assert (first_valve >= 0);
	assert (last_valve < 16);
	
	if (time_begin < 0)
		ahead_begin = 0;
	else
		ahead_begin = time_begin / INTERVAL;
	
	if (time_end < time_begin)
		ahead_end = ahead_begin + 1;
	else
		ahead_end = time_end / INTERVAL + 1;
	
	printf("Ahead: (%d, %d), Valves: (%d, %d)\n", ahead_begin, ahead_end, first_valve, last_valve);
	
	/* The valves are addressed from the right to the left relative to the picture of the camera. */
	for (i = ahead_begin; i < ahead_end; i += 1)
		for (j = first_valve; j <= last_valve; j += 1)
			valves.values[(valves.next_values + i) % VALUES_AHEAD][15 - j] = true;
}

/*!
 * @brief Handles the valves for the next time step.
 *
 * This blocks until the time to handle the valves has arrived and then sends the next packet over the modbus interface.
 */
void valves_handleValves() {
	int32 const sleep_time = valves.next_time - OscSupCycGet();
	t_index i;
	uint16 valves_out = 0;
	
	/* If we're in calibration mode, we're not handling the valves */
	if (configuration.calibrating)
		return;
	
	if (sleep_time > 0)
		usleep(OscSupCycToMicroSecs(sleep_time));
	
	if (sleep_time < 0)
	{
		uint32 const behind = OscSupCycToMicroSecs(-sleep_time);
		if (behind > 1000)
			printf("Behind by %d ms!\n", behind / 1000);
	}
	
	/* Here we put together the two bytes we send over Modbus. */
	for (i = 0; i < 16; i += 1)
	{
		valves_out <<= 1;
		if (valves.values[valves.next_values][i] || configuration.valve_override[i])
			valves_out |= 0x0001;
		valves.values[valves.next_values][i] = false;
	}
	
	/* This sends the message. */
	modbus_sendMessage(valves_out);
	
	/* Here we set up the pointer to the ring buffer an the time we will handle the valves next. */
	valves.next_values = (valves.next_values + 1) % VALUES_AHEAD;
	valves.next_time += INTERVAL;
}

/*!
 * @brief Initializes the valve handling subsystem.
 */
void valves_init()
{
	t_index i, j;
	
	/* This clears the ring buffer. */
	for (i = 0; i < length(valves.values); i += 1)
		for (j = 0; i < 16; i += 1)
			valves.values[i][j] = false;
	
	/* Here we set up the pointer to the ring buffer an the time we will handle the valves next. */
	valves.next_values = 0;
	valves.next_time = OscSupCycGet();
}
