#include "pmic.h"

void pmic_test(void)
{
    printk("Starting PMIC\n");
	if (!device_is_ready(swtest.port)) {
		return -1;
	}
	ret = gpio_pin_configure_dt(&swtest, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

	if (!device_is_ready(lsw1.port)) {
		return -1;
	}
	ret = gpio_pin_configure_dt(&lsw1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}


	while(1) {
		printk("Toogle Pin %i and Pin %i\n", swtest.pin, lsw1.pin);

		ret = gpio_pin_toggle_dt(&swtest);
		if (ret < 0) {
			return;
		}
		ret = gpio_pin_toggle_dt(&lsw1);
		if (ret < 0) {
			return;
		}
		k_sleep(K_MSEC(5000));

	
		Sensor
		if (sensor_sample_fetch(sht)) {


		sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);
		
		printk("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]",
			sensor_value_to_double(&temp),
			sensor_value_to_double(&hum));

		// PMIC
		//read_sensors();
		}
	}

}