/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// INCLUDES

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

#include "pmic.h"
#include "sensors.h"
#include <math.h>

// ADC Stuff

#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>

#include <hal/nrf_rtc.h>
#include <hal/nrf_ppi.h>
#include <hal/nrf_power.h>

// RTC and Wakeup
#include <zephyr/drivers/counter.h>

#include "error_flags.h"

const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));
const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));



LOG_MODULE_REGISTER(Sensor_Modul, LOG_LEVEL_DBG);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_M 1000*60*5  // 10 minutes

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);



// RTC

// const struct device *rtc = DEVICE_DT_GET(DT_NODELABEL(rtc1));

// counter_start(rtc);
// counter_set_channel_alarm(rtc, 0, &(struct counter_alarm_cfg){
//     .ticks = counter_us_to_ticks(rtc, 60000000), // 60s
//     .callback = rtc_handler,
//     .user_data = NULL,
//     .flags = 0
// });


// void setup_rtc_wakeup(void)
// {
// 	NRF_RTC1->PRESCALER = 0;		// 32.768 kHz
// 	NRF_RTC1->CC[0] = 32768 * 1000; // Wake after ~10 seconds
// 	NRF_RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
// 	NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Msk;
// 	NRF_RTC1->TASKS_START = 1;
// }

// void setup_rtc_wakeup_ppi(void) {
//     NRF_PPI->CH[0].EEP = (uint32_t)&NRF_RTC1->EVENTS_COMPARE[0];
//     NRF_PPI->CH[0].TEP = (uint32_t)&NRF_POWER->TASKS_CONSTLAT;  // or SYSTEMOFF wakeup task
//     NRF_PPI->CHENSET = PPI_CHENSET_CH0_Msk;
// }

// BLE and BTHome Service Data
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define SERVICE_DATA_LEN 17 	// 13
#define SERVICE_UUID 0xfcd2 	/* BTHome service UUID */
#define IDX_BAT 4				/* Index of byte of Bat in service data*/
#define IDX_TEMPL 6				/* Index of lo byte of temp in service data*/
#define IDX_TEMPH 7				/* Index of hi byte of temp in service data*/
#define IDX_HUML 9				/* Index of lo byte of hum in service data*/
#define IDX_HUMH 10				/* Index of hi byte of hum in service data*/
#define IDX_ERROR 14			/* Index of byte of count in service data*/
#define IDX_WINDOW_STATE 16		/* Index of byte of count in service data*/


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
	0x09, /* 8bit count used to communicate error flags*/
	0x00,
	0x2D, /* Default State is 0 -> FALSE and OFF/CLOSED and 1 -> TRUE and ON/OPEN*/
	0x00,
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

	//setup_rtc_wakeup(); // Configure RTC compare event

	int err;

	sensors_init();
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
	LOG_INF("Version_Tag: 2024-11-07_v0.5.0\n");

	// BLE INIT and Setup for BTHome

	int door_state = sensor_get_door_state();
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



	struct sensor_value temp, hum;
	temp.val1 = 110;
	temp.val2 = 110;
	hum.val1 = 110;
	hum.val2 = 110;

	LOG_INF("Initialization of all Interfaces DONE!\n");
	LOG_INF("Entering Main Loop\n");

	while (true)
	{

		// uint32_t reason = NRF_POWER->RESETREAS;
		// LOG_INF("Reset reason: 0x%08x", reason);

		// if (reason & POWER_RESETREAS_RESETPIN_Msk)
		// {
		// 	LOG_INF("Reset from pin");
		// }
		// if (reason & POWER_RESETREAS_LPCOMP_Msk)
		// {
		// 	LOG_INF("Woke up from LPCOMP");
		// }
		// if (reason & POWER_RESETREAS_OFF_Msk)
		// {
		// 	LOG_INF("Wakeup caused by GPIO");
		// }
		// NRF_POWER->RESETREAS = 0xFFFFFFFF; // Clear flags
		get_pmic_sensors();

		temp = sensor_sht4x_get_temperature();
		hum = sensor_sht4x_get_humidity();


		LOG_DBG("SHT4X: %.2f Temp. [C] ; %0.2f RH [%%]\n",
				sensor_value_to_double(&temp),
				sensor_value_to_double(&hum));

		//
		get_adc_data();
		uint16_t bat_percentage = get_pmic_battery_percent();
		// converting Measurements to BTHome protocol
		int16_t ble_temp = (floor(sensor_value_to_double(&temp) * 100));
		uint16_t ble_hum = (floor(sensor_value_to_double(&hum) * 100));

		// see https://bthome.io/format/ for details on the format
		// https://www.reddit.com/r/microcontrollers/comments/1342j4u/splitting_16bit_int_to_8bit/
		service_data[IDX_TEMPH] = (ble_temp & 0xff00) >> 8;
		service_data[IDX_TEMPL] = ble_temp & 0xff;
		service_data[IDX_HUMH] = (ble_hum & 0xff00) >> 8;
		service_data[IDX_HUML] = ble_hum & 0xff;
		service_data[IDX_BAT] = bat_percentage;			  // Needs to be changed -> ADC Measurement of Voltage from battery needed here
		service_data[IDX_WINDOW_STATE] = door_state; // read the door state and update the service data
		service_data[IDX_ERROR] = error_flags_get(); // read error flags and update service data
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
		k_sleep(K_MSEC(100));
		

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

		k_sleep(K_MSEC(100)); // needed to ensure that logging is outputted before entering power down mode
		// pm_device_action_run(uart_dev, PM_DEVICE_ACTION_SUSPEND);
		// pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_SUSPEND);

		k_sleep(K_MSEC(SLEEP_TIME_M)); 
		// suspend unused devices to save power


		// NRF_POWER->SYSTEMOFF = 1; // Enter System Off Mode via direct register access
		//  pm_state_force(0, &(struct pm_state_info){
		//  					  .state = PM_STATE_SOFT_OFF,
		//  					  .substate_id = 0,
		//  				  }); // Enter Soft Off Mode via PM API - recommended way

		//NRF_RTC1->EVENTS_COMPARE[0] = 0;
		// LOG_INF("Entering idle sleep...");
		// __WFI(); // Wait For Interrupt
	}
}

