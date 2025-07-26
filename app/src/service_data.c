#include "service_data.h"
#include "sensors.h"
#include "pmic.h"
#include "sleep.h"

void service_data_reset(service_data_t *data) {
    if (data) {
        *data = (service_data_t){0};
    }
}

void service_data_compose(service_data_t *data) {
    if (!data) return;
    service_data_reset(data);

    data->temperature_c = sensor_get_temperature();
    data->humidity_pct = sensor_get_humidity();
    data->battery_mv = sensor_get_battery_mv();
    data->door_open = sensor_get_door_state();
}
