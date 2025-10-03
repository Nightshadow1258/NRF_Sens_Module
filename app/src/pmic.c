#include "pmic.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pmic, LOG_LEVEL_DBG);

void get_pmic_sensors(void)
{
	LOG_DBG("READ PMIC Sensors:\n");
	
	struct sensor_value volt;
	struct sensor_value current;
	struct sensor_value temp;
	struct sensor_value error;
	struct sensor_value status;
	struct sensor_value vbus_present;
	struct sensor_value dietemp;

	sensor_sample_fetch(charger);
	LOG_DBG("PMIC Sensor Sample fetched\n");
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &volt);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &current);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_TEMP, &temp);
	sensor_channel_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_STATUS,
			   &status);
	sensor_channel_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_ERROR, &error);
	sensor_attr_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_VBUS_STATUS,
			(enum sensor_attribute)SENSOR_ATTR_NPM1300_CHARGER_VBUS_PRESENT,
			&vbus_present);
	
	npm1300_charger_channel_get(charger, SENSOR_CHAN_DIE_TEMP, &dietemp);	
	LOG_DBG("PMIC Die Temp: %d C\n", dietemp.val1);

	LOG_DBG("VBAT : %d C\n", volt.val1);
	LOG_DBG("V: %d.%03d ", volt.val1, volt.val2 / 1000);

	LOG_DBG("I: %s%d.%04d ", ((current.val1 < 0) || (current.val2 < 0)) ? "-" : "",
	       abs(current.val1), abs(current.val2) / 100);

	LOG_DBG("T: %s%d.%02d\n", ((temp.val1 < 0) || (temp.val2 < 0)) ? "-" : "", abs(temp.val1),
	       abs(temp.val2) / 10000);

	LOG_DBG("Charger Status: %d, Error: %d, VBUS: %s\n", status.val1, error.val1,
	       vbus_present.val1 ? "connected" : "disconnected");

}

uint16_t get_pmic_battery_percent(void)
{
    struct sensor_value volt;
    if (sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &volt) != 0) {
        LOG_ERR("Failed to get voltage");
        return 0;
    }
    float voltage = volt.val1 + volt.val2 / 1000000.0f;
    uint16_t percentage = 0;
    if (voltage <= 3.0f) {
        percentage = 0;
    } else if (voltage >= 4.2f) {
        percentage = 100;
    } else {
        percentage = (uint16_t)(((voltage - 3.0f) / 1.2f) * 100.0f + 0.5f);
    }
    LOG_DBG("Battery Voltage: %.2fV, Estimated Charge: %d%%\n", voltage, percentage);
    return percentage;
}