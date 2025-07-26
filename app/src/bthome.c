#include "bthome.h"

static int bthome_init(void)
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

static int bthome_send_data(service_data)
{
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

}

int bthome_encode(uint8_t *buf, size_t len, const service_data_t *data) {
    if (len < SERVICE_DATA_LEN) return -ENOMEM;

    buf[0] = SERVICE_UUID & 0xFF;
    buf[1] = (SERVICE_UUID >> 8) & 0xFF;
    buf[2] = 0x40;         // BTHome header

    buf[3] = 0x01;         // Battery
    buf[4] = data->battery_mv / 10;  // Simplified

    buf[5] = 0x02;         // Temperature
    int16_t temp_encoded = (int16_t)(data->temperature * 100);
    buf[6] = temp_encoded & 0xFF;
    buf[7] = (temp_encoded >> 8) & 0xFF;

    buf[8] = 0x03;         // Humidity
    buf[9] = 0x00;         // (populate if available)
    buf[10] = 0x00;

    buf[11] = 0x09;        // State
    buf[12] = data->door_open ? 1 : 0;

    return SERVICE_DATA_LEN;
}
