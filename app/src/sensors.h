#pragma once
#include <stdbool.h>
#include <stdint.h>

// #include "service_data.h"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/sgp40.h>


#include <hal/nrf_gpio.h>
#include <zephyr/drivers/adc.h>

int sensors_init(void);


// *******************
// SHT4x Section
// *******************
#define SHT_NODE DT_INST(0, sensirion_sht4x)
static const struct device *dev_sht4x = DEVICE_DT_GET(DT_INST(0, sensirion_sht4x));

static int sensor_sht4x_init(void);
struct sensor_value sensor_sht4x_get_temperature(void);
struct sensor_value sensor_sht4x_get_humidity(void);
uint16_t sensor_get_battery_mv(void);


// *******************
// ADC Section
// *******************
#include <zephyr/drivers/adc.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static int sensor_adc_init(void);
double get_adc_data(void);


// *******************
// Door Section
// *******************
#include <zephyr/drivers/gpio.h>
#include <hal/nrf_gpio.h>

#define DOOR_NODE DT_ALIAS(door0)
#define DEBOUNCE_TIMEOUT_MS 50 // 50 ms debounce timeout

extern struct gpio_dt_spec door;

static struct gpio_callback door_cb_data;
static struct k_work_delayable debounce_work;


static int sensor_door_init(void);
int sensor_get_door_state(void);
