/*! @file valves.c
 * @brief Contains function to handle the valves with support of the modbus component.
 */

#include <unistd.h>

#include "valves.h"
#include "modbus.h"

#define assert(a) if (!(a)) printf("%s: %s: %d: Assertion failed: %s", __FILE__, __func__, __LINE__, #a)

#define CPU_FREQ 500000000

/* This sets to handle the valves a hundred times per second. */
#define INTERVAL (CPU_FREQ / 50)
#define VALUES_AHEAD 10

struct {
	/* Time the values values[nex_values] should be written to the modbus interface. */
	t_time next_time;
	t_index next_values;
	bool values[VALUES_AHEAD][16];
} valves;

void valves_insertEvent(t_time const begin_time, t_time const end_time, t_index const first_valve, t_index const last_valve)
{
	t_index const ahead_begin = (begin_time - valves.next_time) / INTERVAL;
	t_index const ahead_end = (end_time - valves.next_time) / INTERVAL + 1;
	t_index i, j;
	
	assert (begin_time >= valves.next_time);
	assert (end_time >= begin_time);
	assert (first_valve >= 0);
	assert (last_valve < 16);
	
	for (i = ahead_begin; i < ahead_end; i += 1)
		for (j = first_valve; j <= last_valve; i += 1)
			valves.values[(valves.next_values + i) % VALUES_AHEAD][j] = true;
}

/* This blocks until the time to handle the valves has arrived and then sends the next packet over the modbus interface. */
void valves_handleValves() {
	int32 const sleep_time = valves.next_time - OscSupCycGet();
	t_index i;
	uint16 valves_out = 0;
	
	if (sleep_time > 0)
		usleep(OscSupCycToMicroSecs(sleep_time));
	else
		printf("Behind by %lu ms!\n", OscSupCycToMicroSecs(-sleep_time) / 1000);
	
	for (i = 0; i < 16; i += 1)
		if (valves.values[valves.next_values][i])
			valves_out = (valves_out << 1) & 0x0001;
	
	modbus_sendMessage(valves_out);
	
	valves.next_values = (valves.next_time + 1) % INTERVAL;
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
