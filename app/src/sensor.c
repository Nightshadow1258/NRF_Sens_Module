#include "sensor.h"


void read_sensors(void)
{
	struct sensor_value volt;
	struct sensor_value current;
	struct sensor_value temp;
	struct sensor_value error;
	struct sensor_value status;

	sensor_sample_fetch(charger);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &volt);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &current);
	sensor_channel_get(charger, SENSOR_CHAN_GAUGE_TEMP, &temp);
	sensor_channel_get(charger, SENSOR_CHAN_NPM1300_CHARGER_STATUS, &status);
	sensor_channel_get(charger, SENSOR_CHAN_NPM1300_CHARGER_ERROR, &error);

	printk("V: %d.%03d ", volt.val1, volt.val2 / 1000);

	printk("I: %s%d.%04d ", ((current.val1 < 0) || (current.val2 < 0)) ? "-" : "",
	       abs(current.val1), abs(current.val2) / 100);

	printk("T: %s%d.%02d\n", ((temp.val1 < 0) || (temp.val2 < 0)) ? "-" : "", abs(temp.val1),
	       abs(temp.val2) / 10000);

	printk("Charger Status: %d, Error: %d\n", status.val1, error.val1);
}
void sensor_test(void)
{
    int err;
	int ret;
	int temp=0;
	// Sensor
	struct sensor_value temp, hum, gas;
	//	PMIC init stuff
	if (!device_is_ready(pmic)) {
		printk("Pmic device not ready.\n");
		return;
	}

	if (!device_is_ready(regulators)) {
		printk("Error: Regulator device is not ready\n");
		return 0;
	}

	if (!device_is_ready(charger)) {
		printk("Charger device not ready.\n");
		return 0;
	}
}
