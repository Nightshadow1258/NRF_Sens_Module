#include "pmic.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pmic, LOG_LEVEL_DBG);

void pmic_test(void)
{	
	int ret;

    printk("Starting PMIC\n");
	// if (!device_is_ready(swtest.port)) {
	// 	return -1;
	// }
	// ret = gpio_pin_configure_dt(&swtest, GPIO_OUTPUT_ACTIVE);
	// if (ret < 0) {
	// 	return -1;
	// }

	if (!device_is_ready(lsw1.port)) {
		return -1;
	}
	ret = gpio_pin_configure_dt(&lsw1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

	

	while(1) {
		printk("Toogle Pin %i \n", lsw1.pin);

		// ret = gpio_pin_toggle_dt(&swtest);
		// if (ret < 0) {
		// 	return;
		// }
		ret = gpio_pin_toggle_dt(&lsw1);
		if (ret < 0) {
			return;
		}
		k_sleep(K_MSEC(5000));

	
		// Sensor
		// if (sensor_sample_fetch(sht)) {


		// sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		// sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);
		
		// printk("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]",
		// 	sensor_value_to_double(&temp),
		// 	sensor_value_to_double(&hum));

		// PMIC
		//read_sensors();
		//}
	}

}

void read_sensors(void)
{
	LOG_DBG("READ PMIC Sensors:\n");
	struct sensor_value volt;
	struct sensor_value current;
	struct sensor_value temp;
	struct sensor_value error;
	struct sensor_value status;
	struct sensor_value vbus_present;

	sensor_sample_fetch(charger);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &volt);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &current);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_TEMP, &temp);
	sensor_channel_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_STATUS,
			   &status);
	sensor_channel_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_ERROR, &error);
	sensor_attr_get(charger, (enum sensor_channel)SENSOR_CHAN_NPM1300_CHARGER_VBUS_STATUS,
			(enum sensor_attribute)SENSOR_ATTR_NPM1300_CHARGER_VBUS_PRESENT,
			&vbus_present);

	LOG_DBG("V: %d.%03d ", volt.val1, volt.val2 / 1000);

	LOG_DBG("I: %s%d.%04d ", ((current.val1 < 0) || (current.val2 < 0)) ? "-" : "",
	       abs(current.val1), abs(current.val2) / 100);

	LOG_DBG("T: %s%d.%02d\n", ((temp.val1 < 0) || (temp.val2 < 0)) ? "-" : "", abs(temp.val1),
	       abs(temp.val2) / 10000);

	LOG_DBG("Charger Status: %d, Error: %d, VBUS: %s\n", status.val1, error.val1,
	       vbus_present.val1 ? "connected" : "disconnected");
}