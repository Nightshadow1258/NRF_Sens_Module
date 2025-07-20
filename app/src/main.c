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
// #include "sensor.h"
#include <math.h>

// ADC Stuff
#include <zephyr/drivers/adc.h>



#include <hal/nrf_gpio.h>

#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>


const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));
const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));


#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
						 DT_SPEC_AND_COMMA)};

// LOG_MODULE_DECLARE(Sensor_Modul);
LOG_MODULE_REGISTER(Sensor_Modul, LOG_LEVEL_DBG);

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

// Magnetic Door Sensor
#define DOOR_NODE DT_ALIAS(door0)
#define DEBOUNCE_TIMEOUT_MS 50 // 50 ms debounce timeout

uint64_t last_time = 0;
static const struct gpio_dt_spec door = GPIO_DT_SPEC_GET(DOOR_NODE, gpios);

static struct gpio_callback door_cb_data;
static struct k_work_delayable debounce_work;

void debounce_handler(struct k_work *work)
{
	int val = gpio_pin_get_dt(&door);
	LOG_INF("Debounced door state: %s\n", val ? "OPEN" : "CLOSED");
}

void door_callback(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
	k_work_reschedule(&debounce_work, K_MSEC(30)); // 30 ms debounce window
}

void door_sensor_init(void)
{
	if (!device_is_ready(door.port))
	{
		LOG_ERR("Door GPIO device not ready");
		return;
	}

	if (gpio_pin_configure_dt(&door, GPIO_INPUT | GPIO_PULL_UP))
	{
		LOG_ERR("Failed to configure door GPIO pin");
		return;
	}

	// Enable interrupt before adding callback
	if (gpio_pin_interrupt_configure_dt(&door, GPIO_INT_EDGE_BOTH))
	{
		LOG_ERR("Failed to configure door GPIO interrupt");
		return;
	}

	gpio_init_callback(&door_cb_data, door_callback, BIT(door.pin));

	if (gpio_add_callback(door.port, &door_cb_data))
	{
		LOG_ERR("Failed to add door callback");
		return;
	}

	// Debounce work item setup
	k_work_init_delayable(&debounce_work, debounce_handler);

	// Wakeup source for power management
	pm_device_wakeup_enable(door.port, true);
}

#define SERVICE_DATA_LEN 13 // 13
#define SERVICE_UUID 0xfcd2 /* BTHome service UUID */
#define IDX_BAT 4			/* Index of byte of Bat in service data*/
#define IDX_TEMPL 6			/* Index of lo byte of temp in service data*/
#define IDX_TEMPH 7			/* Index of hi byte of temp in service data*/
#define IDX_HUML 9			/* Index of lo byte of temp in service data*/
#define IDX_HUMH 10			/* Index of hi byte of temp in service data*/
#define IDX_STATE 12		/* Index of byte of count in service data*/

#define ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
								  BT_GAP_ADV_SLOW_INT_MIN,    \
								  BT_GAP_ADV_SLOW_INT_MAX, NULL)

static uint8_t service_data[SERVICE_DATA_LEN] = {
	BT_UUID_16_ENCODE(SERVICE_UUID),
	0x40,
	0x01, /* Battery */
	0x61, /* in percent */
	0x02, /* Temperature */
	0x98, /* Low byte */
	0x08, /* High byte */
	0x03, /* Humidity */
	0xbf, /* 50.55%  low byte*/
	0x13, /* 50.55%  high byte*/
	0x09, /* 8bit count used to communicate a state ON/OFF or OPEN/CLOSE*/
	0x00, /* Default State is 0 -> FALSE and OFF/CLOSED and 1 -> TRUE and ON/OPEN*/
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

static void bt_ready(int err)
{
	if (err)
	{
		LOG_DBG("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_DBG("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err)
	{
		LOG_DBG("Advertising failed to start (err %d)\n", err);
		return;
	}
}

int main(void)
{

	pm_device_action_run(uart_dev, PM_DEVICE_ACTION_RESUME);
	pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_RESUME);
	int err;

	const struct device *const sht = DEVICE_DT_GET_ANY(sensirion_sht4x);

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

	// door sensor init
	door_sensor_init();
	// read door state
	int16_t door_state = gpio_pin_get_dt(&door); // 0 for closed, 1 for open

	// wake up timer
	// K_TIMER_DEFINE(wakeup_timer, wakeup_handler, NULL);
	// void wakeup_handler(struct k_timer * timer_id)
	// {
	// 	// Trigger wakeup logic or post an event
	// }

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err)
	{
		LOG_INF("Bluetooth init failed (err %d)\n", err);
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
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
	{
		if (!adc_is_ready_dt(&adc_channels[i]))
		{
			LOG_DBG("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0)
		{
			LOG_DBG("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}

	struct sensor_value temp, hum;
	temp.val1 = 110;
	temp.val2 = 110;
	hum.val1 = 110;
	hum.val2 = 110;
	if (!device_is_ready(sht))
	{
		LOG_DBG("Device %s is not ready.\n", sht->name);
		return 0;
	}

	LOG_INF("Initialization of all Interfaces DONE!\n");
	LOG_INF("Entering Main Loop\n");

	while (true)
	{

		// Reading SHT4 Sensor - Temp and Humidity
		if (sensor_sample_fetch(sht))
		{
			LOG_DBG("Failed to fetch sample from SHT4X device\n");
			return 0;
		}

		sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);

		LOG_DBG("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]\n",
				sensor_value_to_double(&temp),
				sensor_value_to_double(&hum));

		//
		read_sensors();
		LOG_DBG("ADC reading[%u]:\n", count++);
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
		{
			int32_t val_mv;

			LOG_DBG("- %s, channel %d: ",
					adc_channels[i].dev->name,
					adc_channels[i].channel_id);

			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read_dt(&adc_channels[i], &sequence);
			if (err < 0)
			{
				LOG_DBG("Could not read (%d)\n", err);
				continue;
			}

			/*
			 * If using differential mode, the 16 bit value
			 * in the ADC sample buffer should be a signed 2's
			 * complement value.
			 */
			if (adc_channels[i].channel_cfg.differential)
			{
				val_mv = (int32_t)((int16_t)buf);
			}
			else
			{
				val_mv = (int32_t)buf;
			}
			vbat_raw_lsb = (uint16_t)val_mv / 4095 * 3.3 * 4 / 3;
			LOG_DBG("Battery Voltage %d\n", vbat_raw_lsb);
		}

		// converting Measurements to BTHome protocol
		int16_t ble_temp = (floor(sensor_value_to_double(&temp) * 100));
		uint16_t ble_hum = (floor(sensor_value_to_double(&hum) * 100));

		// see https://bthome.io/format/ for details on the format
		// https://www.reddit.com/r/microcontrollers/comments/1342j4u/splitting_16bit_int_to_8bit/
		service_data[IDX_TEMPH] = (ble_temp & 0xff00) >> 8;
		service_data[IDX_TEMPL] = ble_temp & 0xff;
		service_data[IDX_HUMH] = (ble_hum & 0xff00) >> 8;
		service_data[IDX_HUML] = ble_hum & 0xff;
		service_data[IDX_BAT] = 80;						  // Needs to be changed -> ADC Measurement of Voltage from battery needed here
		service_data[IDX_STATE] = door_state; // read the door state and update the service data

		LOG_DBG("TEMP: transformed %x, Shifted: %x,%x\n", ble_temp, service_data[IDX_TEMPH], service_data[IDX_TEMPL]);
		LOG_DBG("HUM: transformed %x, Shifted: %x,%x\n", ble_hum, service_data[IDX_HUMH], service_data[IDX_HUML]);

		// set data via ble and BTHome
		LOG_HEXDUMP_INF(service_data, sizeof(service_data), "Service Data:");
		for (int i = 0; i < ARRAY_SIZE(ad); i++)
		{
			LOG_HEXDUMP_INF(ad[i].data, ad[i].data_len, "AD Payload:");
		}

		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err)
		{
			LOG_DBG("Failed to update advertising data (err %d)\n", err);
		}
		LOG_DBG("Updated advertising data \n");



		if (door_state == 0)
		{
			LOG_INF("Door is CLOSED - Entering Soft Off Mode\n");
			nrf_gpio_cfg_sense_input(door.pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_HIGH);
		}
		else
		{
			LOG_INF("Door is OPEN - Entering Soft Off Mode\n");
			    nrf_gpio_cfg_sense_input(door.pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
		}


		k_sleep(K_MSEC(1000)); // needed to ensure that logging is outputted before entering power down mode

		// suspend unused devices to save power
		pm_device_action_run(uart_dev, PM_DEVICE_ACTION_SUSPEND);
		pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_SUSPEND);



		NRF_POWER->SYSTEMOFF = 1; // Enter System Off Mode via direct register access
		// pm_state_force(0, &(struct pm_state_info){
		// 					  .state = PM_STATE_SOFT_OFF,
		// 					  .substate_id = 0,
		// 				  }); // Enter Soft Off Mode via PM API - recommended way

						
		k_sleep(K_MSEC(1000 * 60 * 5)); // 5 Minutes Sleep
	}
}




// #include <zephyr/drivers/counter.h>

// const struct device *rtc = DEVICE_DT_GET(DT_NODELABEL(rtc1));

// counter_start(rtc);
// counter_set_channel_alarm(rtc, 0, &(struct counter_alarm_cfg){
//     .ticks = counter_us_to_ticks(rtc, 60000000), // 60s
//     .callback = rtc_handler,
//     .user_data = NULL,
//     .flags = 0
// });
