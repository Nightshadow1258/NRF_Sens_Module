#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ERR_NONE           0x00
#define ERR_SENSOR_FAIL    0x01  // Bit 0
#define ERR_STORAGE_FULL   0x02  // Bit 1
#define ERR_COMM_TIMEOUT   0x04  // Bit 2
#define ERR_OVERHEAT       0x08  // Bit 3
#define ERR_LOW_BATTERY    0x10  // Bit 4
// Add more as needed

void error_flags_set(uint8_t err_mask);
void error_flags_clear(uint8_t err_mask);
bool error_flags_is_set(uint8_t err_mask);
uint8_t error_flags_get(void);
void error_flags_reset(void);
