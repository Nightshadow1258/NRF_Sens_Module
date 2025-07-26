#pragma once

#include <stdbool.h>
#include <stdint.h>

// Unified snapshot of system state
typedef struct {
    float temperature_c;
    float humidity_pct;
    uint16_t battery_mv;
    bool door_open;
} service_data_t;

// Clears values to default
void service_data_reset(service_data_t *data);

// Gathers data from subsystems
void service_data_compose(service_data_t *data);
