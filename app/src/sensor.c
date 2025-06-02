#include "sensor.h"




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
