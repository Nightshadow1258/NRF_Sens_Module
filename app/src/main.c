/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

// #include "pmic.h"
#include "sensor.h"
#include <math.h>

LOG_MODULE_REGISTER(SensorNode, LOG_LEVEL_INF);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);




#define SERVICE_DATA_LEN        7 //9
#define SERVICE_UUID            0xfcd2      /* BTHome service UUID */
#define IDX_TEMPL               4           /* Index of lo byte of temp in service data*/
#define IDX_TEMPH               5           /* Index of hi byte of temp in service data*/

#define ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				  BT_GAP_ADV_SLOW_INT_MIN, \
				  BT_GAP_ADV_SLOW_INT_MAX, NULL)

static uint8_t service_data[SERVICE_DATA_LEN] = {
	BT_UUID_16_ENCODE(SERVICE_UUID),
	0x40,
	0x02,	/* Temperature */
	0xc4,	/* Low byte */
	0x00,   /* High byte */
	// 0x03,	/* Humidity */
	// 0xbf,	/* 50.55%  low byte*/
	// 0x13,   /* 50.55%  high byte*/
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
}


int main(void)
{
	int err;

	// LOG_INF("Starting Lesson 2 - Exercise 1 \n");

	// /* STEP 5 - Enable the Bluetooth LE stack */
	// err = bt_enable(NULL);
	// if (err) {
	// 	LOG_ERR("Bluetooth init failed (err %d)\n", err);
	// 	return -1;
	// }

	// LOG_INF("Bluetooth initialized\n");

	// /* STEP 6 - Start advertising */
	// err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	// if (err) {
	// 	LOG_ERR("Advertising failed to start (err %d)\n", err);
	// 	return -1;
	// }

	// LOG_INF("Advertising successfully started\n");
	int test = 0;

	const struct device *const sht = DEVICE_DT_GET_ANY(sensirion_sht4x);
	struct sensor_value temp, hum;
	if (!device_is_ready(sht))
	{
		printf("Device %s is not ready.\n", sht->name);
		return 0;
	}

	printk("Starting BTHome sensor template\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	while (true)
	{

		if (sensor_sample_fetch(sht))
		{
			printf("Failed to fetch sample from SHT4X device\n");
			return 0;
		}

		sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);

		printf("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]\n",
			   sensor_value_to_double(&temp),
			   sensor_value_to_double(&hum));

		k_sleep(K_MSEC(2000));
		int16_t tmp = (floor(sensor_value_to_double(&temp)*100));
		
		/* Simulate temperature from 0C to 25C */
		printf("TESTING: %d \n" , tmp);
		// see https://bthome.io/format/ for details on the format
		service_data[IDX_TEMPH] = tmp >> 8;
		service_data[IDX_TEMPL] = tmp << 8;
		if (test++ == 25) {
			test = 0;
		}
		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			printk("Failed to update advertising data (err %d)\n", err);
		}
		k_sleep(K_MSEC(BT_GAP_ADV_SLOW_INT_MIN));
	}
}
