#ifndef MODBUS_CONTROL_H
#define MODBUS_CONTROL_H

#include <cstring>
#include <Arduino.h>

float solar_flaten(float input_mppt);
int demand_response(int load_power);
void reset();

#endif
