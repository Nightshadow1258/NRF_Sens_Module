/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define SW_LSW1 DT_NODELABEL(loadsw0)
#define SW_TEST DT_NODELABEL(loadsw1)
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const struct gpio_dt_spec lsw1 = GPIO_DT_SPEC_GET(SW_LSW1, gpios);
static const struct gpio_dt_spec swtest = GPIO_DT_SPEC_GET(SW_TEST, gpios);



// Bluetooth stuff
#define SERVICE_DATA_LEN        9
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
	0x03,	/* Humidity */
	0xbf,	/* 50.55%  low byte*/
	0x13,   /* 50.55%  high byte*/
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



// int main(void)
// {
// 	int ret;
// 	bool led_state = true;

// 	if (!gpio_is_ready_dt(&led)) {
// 		return 0;
// 	}

// 	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
// 	if (ret < 0) {
// 		return 0;
// 	}

// 	while (1) {
// 		ret = gpio_pin_toggle_dt(&led);
// 		if (ret < 0) {
// 			return 0;
// 		}

// 		led_state = !led_state;
// 		printf("LED state changed: %s\n", led_state ? "ON" : "OFF");
// 		k_msleep(SLEEP_TIME_MS);
// 	}
// 	return 0;
// }

int main(void)
{
	int err;
	int temp = 0;
	int ret;

	printk("Starting GPIO Testing\n");
	if (!device_is_ready(swtest.port)) {
		return -1;
	}
	ret = gpio_pin_configure_dt(&swtest, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

	printk("Starting GPIO Testing\n");
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
	}

	// printk("Starting BTHome sensor template\n");

	// /* Initialize the Bluetooth Subsystem */
	// err = bt_enable(bt_ready);
	// if (err) {
	// 	printk("Bluetooth init failed (err %d)\n", err);
	// 	return 0;
	// }

	// for (;;) {
	// 	/* Simulate temperature from 0C to 25C */
	// 	service_data[IDX_TEMPH] = (temp * 100) >> 8;
	// 	service_data[IDX_TEMPL] = (temp * 100) & 0xff;
	// 	if (temp++ == 25) {
	// 		temp = 0;
	// 	}
	// 	err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
	// 	if (err) {
	// 		printk("Failed to update advertising data (err %d)\n", err);
	// 	}
	// 	k_sleep(K_MSEC(BT_GAP_ADV_SLOW_INT_MIN));
		
	
	// }
	// return 0;
}