/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// INCLUDES

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

#include "pmic.h"
//#include "sensor.h"
#include <math.h>


// ADC Stuff
#include <zephyr/drivers/adc.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};


//LOG_MODULE_DECLARE(SensorNode);
LOG_MODULE_REGISTER(SensorNode, LOG_LEVEL_DBG);

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


#define SERVICE_DATA_LEN        11 //13
#define SERVICE_UUID            0xfcd2      /* BTHome service UUID */
#define IDX_BAT					4			/* Index of byte of Bat in service data*/
#define IDX_TEMPL               6           /* Index of lo byte of temp in service data*/
#define IDX_TEMPH               7           /* Index of hi byte of temp in service data*/
#define IDX_HUML               	9           /* Index of lo byte of temp in service data*/
#define IDX_HUMH               	10         	/* Index of hi byte of temp in service data*/
#define IDX_STATE              	12         	/* Index of byte of count in service data*/


#define ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				  BT_GAP_ADV_SLOW_INT_MIN, \
				  BT_GAP_ADV_SLOW_INT_MAX, NULL)

static uint8_t service_data[SERVICE_DATA_LEN] = {
	BT_UUID_16_ENCODE(SERVICE_UUID),
	0x40,
	0x01,	/* Battery */
	0x61,	/* in percent */
	0x02,	/* Temperature */
	0x98,	/* Low byte */
	0x08,   /* High byte */
	0x03,	/* Humidity */
	0xbf,	/* 50.55%  low byte*/
	0x13,   /* 50.55%  high byte*/
	// 0x09,	/* 8bit count used to communicate a state ON/OFF or OPEN/CLOSE*/
	// 0x00,	/* Default State is 0 -> FALSE and OFF/CLOSED and 1 -> TRUE and ON/OPEN*/
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))
};

static void bt_ready(int err)
{
	if (err) {
		LOG_DBG("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_DBG("Advertising failed to start (err %d)\n", err);
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
	LOG_INF("Starting Initialization of all Interfaces\n");

	// BLE INIT and Setup for BTHome
	// const struct device *const sht = DEVICE_DT_GET_ANY(sensirion_sht4x);
	struct sensor_value temp, hum;
	temp.val1 = 110;
	temp.val2 = 110;
	hum.val1 = 110;
	hum.val2 = 110;
	// if (!device_is_ready(sht))
	// {
	// 	LOG_DBG("Device %s is not ready.\n", sht->name);
	// 	return 0;
	// }


	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		LOG_DBG("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	// ADC INIT and Setup
	uint16_t vbat_raw_lsb;
	uint32_t count = 0;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			LOG_DBG("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			LOG_DBG("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}

	LOG_INF("Initialization of all Interfaces DONE!\n");
	LOG_INF("Entering Main Loop\n");

	while (true)
	{

		
		// Reading SHT4 Sensor - Temp and Humidity
		// if (sensor_sample_fetch(sht))
		// {
		// 	printf("Failed to fetch sample from SHT4X device\n");
		// 	return 0;
		// }

		// sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		// sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);

		// printf("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]\n",
		// 	   sensor_value_to_double(&temp),
		// 	   sensor_value_to_double(&hum));

		
		//
		read_sensors();
		LOG_DBG("ADC reading[%u]:\n", count++);
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			int32_t val_mv;

			printk("- %s, channel %d: ",
			       adc_channels[i].dev->name,
			       adc_channels[i].channel_id);

			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read_dt(&adc_channels[i], &sequence);
			if (err < 0) {
				printk("Could not read (%d)\n", err);
				continue;
			}

			/*
			 * If using differential mode, the 16 bit value
			 * in the ADC sample buffer should be a signed 2's
			 * complement value.
			 */
			if (adc_channels[i].channel_cfg.differential) {
				val_mv = (int32_t)((int16_t)buf);
			} else {
				val_mv = (int32_t)buf;
			}
			vbat_raw_lsb = (uint16_t) val_mv/4095 * 3.3 * 4 / 3;
			printk("Battery Voltage %d\n", vbat_raw_lsb);

		}


		// converting Measurements to BTHome protocol
		int16_t ble_temp = (floor(sensor_value_to_double(&temp)*100));
		uint16_t ble_hum = (floor(sensor_value_to_double(&hum)*100));

		
		// see https://bthome.io/format/ for details on the format
		// https://www.reddit.com/r/microcontrollers/comments/1342j4u/splitting_16bit_int_to_8bit/
		service_data[IDX_TEMPH] = (ble_temp & 0xff00) >> 8;
		service_data[IDX_TEMPL] = ble_temp & 0xff;
		service_data[IDX_HUMH] = (ble_hum & 0xff00) >> 8;
		service_data[IDX_HUML] =  ble_hum & 0xff;	
		service_data[IDX_BAT] = 80; // Needs to be changed -> ADC Measurement of Voltage from battery needed here
		//service_data[IDX_STATE] = 1; // Needs to be changed -> to a GPIO reading from the Door Sensor
		
		printk("TEMP: transformed %x, Shifted: %x,%x\n", ble_temp, service_data[IDX_TEMPH], service_data[IDX_TEMPL]);
		printk("HUM: transformed %x, Shifted: %x,%x\n", ble_hum, service_data[IDX_HUMH], service_data[IDX_HUML]);

		// set data via ble and BTHome
		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			printk("Failed to update advertising data (err %d)\n", err);
		}
		printk("Updated advertising data \n");
		k_sleep(K_MSEC(1000*60*15));

	}
}
