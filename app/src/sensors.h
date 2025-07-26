#pragma once
#include <stdbool.h>
#include <stdint.h>

// #include "service_data.h"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/sgp40.h>


#include <hal/nrf_gpio.h>
#include <zephyr/drivers/adc.h>

#include <zephyr/logging/log.h>


int sensors_init(void);

// SHT40 Sensor stuff

struct sensor_value sensor_sht4x_get_temperature(void);
struct sensor_value sensor_sht4x_get_humidity(void);
uint16_t sensor_get_battery_mv(void);


// ADC Sensor stuff

// bool sensor_get_door_state(void);
// void sensor_irq_handler(uint32_t pin);  // If using interrupt-based GPIO

// void sensor_export(service_data_t *data);  // Optional: feeds the shared struct



// // Magnetic Door Sensor
// #define DT_SPEC_AND_COMMA(node_id, prop, idx)
// 	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// #define DOOR_NODE DT_ALIAS(door0)
// #define DEBOUNCE_TIMEOUT_MS 50 // 50 ms debounce timeout

// uint64_t last_time = 0;
// static const struct gpio_dt_spec door = GPIO_DT_SPEC_GET(DOOR_NODE, gpios);

// static struct gpio_callback door_cb_data;
// static struct k_work_delayable debounce_work;

// void debounce_handler(struct k_work *work);
// void door_callback(const struct device *port, struct gpio_callback *cb, uint32_t pins);



// // Data of ADC io-channels specified in devicetree. //
// static const struct adc_dt_spec adc_channels[] = {
// 	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
// 						 DT_SPEC_AND_COMMA)};