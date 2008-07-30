#ifndef MODBUS_H_
#define MODBUS_H_

#include "support.h"

void modbus_init();
void modbus_sendMessage(uint16 const valves);

#endif /* MODBUS_H_ */
