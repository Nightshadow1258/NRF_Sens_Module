#pragma once

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

// BLE and BTHome Service Data
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME

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
	0x2D, /* Window State: 0 (False = Closed) 1 (True = Open)  / 
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

static int bthome_init(void);

static int bthome_send_data(service_data);

int bthome_encode(uint8_t *buf, size_t len, const service_data_t *data);


//Object ids by order
#if 0
#define ID_PACKET				0x00
#define ID_BATTERY				0x01
#define ID_TEMPERATURE_PRECISE 	0x02
#define ID_HUMIDITY_PRECISE 	0x03
#define ID_PRESSURE 			0x04
#define ID_ILLUMINANCE			0x05
#define ID_MASS					0x06
#define ID_MASSLB				0x07
#define ID_DEWPOINT				0x08
#define ID_COUNT				0x09
#define ID_ENERGY				0x0A
#define ID_POWER				0x0B
#define ID_VOLTAGE				0x0C
#define ID_PM25					0x0D
#define ID_PM10					0x0E
#define STATE_GENERIC_BOOLEAN	0x0F
#define STATE_POWER_ON			0x10
#define STATE_OPENING			0x11
#define ID_CO2					0x12
#define ID_TVOC					0x13
#define ID_MOISTURE_PRECISE		0x14
#define STATE_BATTERY_LOW		0x15
#define STATE_BATTERY_CHARHING	0x16
#define STATE_CO				0x17
#define STATE_COLD				0x18
#define STATE_CONNECTIVITY		0x19
#define STATE_DOOR				0x1A
#define STATE_GARAGE_DOOR		0x1B
#define STATE_GAS_DETECTED		0x1C
#define STATE_HEAT				0x1D
#define STATE_LIGHT				0x1E
#define STATE_LOCK				0x1F
#define STATE_MOISTURE			0x20
#define STATE_MOTION			0x21
#define STATE_MOVING			0x22
#define STATE_OCCUPANCY			0x23
#define STATE_PLUG				0x24
#define STATE_PRESENCE			0x25
#define STATE_PROBLEM			0x26
#define STATE_RUNNING			0x27
#define STATE_SAFETY			0x28
#define STATE_SMOKE				0x29
#define STATE_SOUND				0x2A
#define STATE_TAMPER			0x2B
#define STATE_VIBRATION			0x2C
#define STATE_WINDOW			0x2D
#define ID_HUMIDITY				0x2E
#define ID_MOISTURE				0x2F
#define EVENT_BUTTON			0x3A
#define EVENT_DIMMER			0x3C
#define ID_COUNT2				0x3D
#define ID_COUNT4				0x3E
#define ID_ROTATION				0x3F
#define ID_DISTANCE				0x40
#define ID_DISTANCEM			0x41
#define ID_DURATION				0x42
#define ID_CURRENT				0x43
#define ID_SPD					0x44
#define ID_TEMPERATURE			0x45
#define ID_UV					0x46
#define ID_VOLUME1				0x47
#define ID_VOLUME2				0x48
#define ID_VOLUMEFR				0x49
#define ID_VOLTAGE1				0x4A
#define ID_GAS					0x4B
#define ID_GAS4					0x4C
#define ID_ENERGY4				0x4D
#define ID_VOLUME				0x4E
#define ID_WATER				0x4F
#endif