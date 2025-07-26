

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
	0x00, /* Default State is 0 -> FALSE and OFF/CLOSED and 1 -> TRUE and ON/OPEN*/
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

static int bthome_init(void);

static int bthome_send_data(service_data);

int bthome_encode(uint8_t *buf, size_t len, const service_data_t *data);