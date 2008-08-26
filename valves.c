/*! @file valves.c
 * @brief Contains function to handle the valves with support of the modbus component.
 */

#include <unistd.h>

#include "modbus.h"
#include "config.h"
#include "valves.h"

/* This value may be used to adjust the timing of the valves. Larger values delay the activation of the valves. */
#define TUNE_VALVES_ON ((uint32) CPU_FREQ / 1000 * -30)
#define TUNE_VALVES_OFF ((uint32) CPU_FREQ / 1000 * 0)

/* This sets to handle the valves a hundred times per second. */
#define INTERVAL ((uint32) CPU_FREQ / 100)
/* This defines how many time steps ahead we can set a valve state */
#define VALUES_AHEAD 100

struct {
	/* Time the values values[nex_values] should be written to the modbus interface. */
	t_time next_time;
	t_index next_values;
	bool values[VALUES_AHEAD][16];
} valves;

void valves_insertEvent(t_time const begin_time, t_time const end_time, t_index const first_valve, t_index const last_valve)
{
	int32 const time_begin = begin_time - valves.next_time + TUNE_VALVES_ON;
	int32 const time_end = end_time - valves.next_time + TUNE_VALVES_OFF;
	t_index i, j, ahead_begin, ahead_end = time_end / INTERVAL + 1;
	
	if (time_begin < 0)
		ahead_begin = 0;
	else
		ahead_begin = time_begin / INTERVAL;
	
	printf("%d, %d, %lu, %lu\n", first_valve, last_valve, begin_time, end_time);
	printf("%d, %d, %d, %d\n", first_valve, last_valve, ahead_begin, ahead_end);
	printf("%d, %d, %d, %d\n", begin_time, valves.next_time, TUNE_VALVES_ON, INTERVAL);
	
	assert (begin_time >= valves.next_time);
	assert (end_time >= begin_time);
	assert (ahead_begin >= 0);
	assert (first_valve >= 0);
	assert (last_valve < 16);
	
	/* The valves are adressed from the right to the left relative to the picture of the camera. */
	for (i = ahead_begin; i < ahead_end; i += 1)
		for (j = first_valve; j <= last_valve; j += 1)
			valves.values[(valves.next_values + i) % VALUES_AHEAD][15 - j] = true;
}

/* This blocks until the time to handle the valves has arrived and then sends the next packet over the modbus interface. */
void valves_handleValves() {
	int32 const sleep_time = valves.next_time - OscSupCycGet();
	t_index i;
	uint16 valves_out = 0;
	
	if (configuration.calibrating)
		return;
	
	if (sleep_time > 0)
		usleep(OscSupCycToMicroSecs(sleep_time));
	else
	{
		uint32 const behind = OscSupCycToMicroSecs(-sleep_time);
		if (behind > 1000)
			printf("Behind by %d ms!\n", behind / 1000);
	}
	
//	printf("-> %d\n", valves.next_values);
	
	for (i = 0; i < 16; i += 1)
	{
		valves_out <<= 1;
		if (valves.values[valves.next_values][i] || configuration.valve_override[i])
			valves_out |= 0x0001;
		valves.values[valves.next_values][i] = false;
	}
	
	modbus_sendMessage(valves_out);
	
	valves.next_values = (valves.next_values + 1) % VALUES_AHEAD;
	valves.next_time += INTERVAL;
}

void valves_init()
{
	t_index i, j;
	
	/* Clears the ring buffer. */
	for (i = 0; i < length(valves.values); i += 1)
		for (j = 0; i < 16; i += 1)
			valves.values[i][j] = false;
	
	valves.next_values = 0;
	valves.next_time = OscSupCycGet();
}
